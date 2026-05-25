/*
 * FFmpeg Video Decoder for KRKRSDL2
 * Copyright (c) Kirikiri SDL2 Developers
 * SPDX-License-Identifier: MIT
 *
 * Phase 2: open() now uses FFmpegAVIOBridge for KRKR Storage / XP3 access.
 * openPath() retains the old plain-filesystem path for testing.
 */

#include "FFmpegVideoDecoder.h"

#if KRKRSDL2_FFMPEG_ENABLED

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
}

#include <cstring>
#include <algorithm>

namespace krkr {
namespace video {

FFmpegVideoDecoder::FFmpegVideoDecoder()
    : formatContext(nullptr)
    , codecContext(nullptr)
    , swsContext(nullptr)
    , videoStreamIndex(-1)
    , audioStreamIdx(-1)
    , packet(nullptr)
    , frame(nullptr)
    , frameRGB(nullptr)
    , buffer(nullptr)
    , isInitialized(false)
    , usingAVIO(false)
    , currentWidth(0)
    , currentHeight(0)
    , currentFps(0)
    , totalDuration(0)
    , currentPosition(0)
{
}

FFmpegVideoDecoder::~FFmpegVideoDecoder()
{
    close();
}

bool FFmpegVideoDecoder::open(const std::string& storageName)
{
    if (isInitialized) {
        close();
    }

    fprintf(stderr, "[FFmpeg-Video] Opening via KRKR Storage: '%s'\n",
            storageName.c_str());

    // Use AVIO bridge to open from KRKR Storage / XP3
    if (!avioBridge_.openFromKRKRStorage(storageName, &formatContext)) {
        fprintf(stderr, "[FFmpeg-Video] AVIO bridge failed for '%s'\n",
                storageName.c_str());
        return false;
    }

    usingAVIO = true;

    // Set up codec, scaler, etc.
    if (!setupDecoder()) {
        close();
        return false;
    }

    return true;
}

bool FFmpegVideoDecoder::openPath(const std::string& path)
{
    if (isInitialized) {
        close();
    }

    fprintf(stderr, "[FFmpeg-Video] Opening via path: '%s'\n", path.c_str());

    // Open input file using plain filesystem path (legacy mode)
    int ret = avformat_open_input(&formatContext, path.c_str(), nullptr, nullptr);
    if (ret < 0) {
        char errBuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errBuf, sizeof(errBuf));
        fprintf(stderr, "[FFmpeg-Video] Failed to open input: %s (%s)\n",
                path.c_str(), errBuf);
        return false;
    }

    // Retrieve stream information
    ret = avformat_find_stream_info(formatContext, nullptr);
    if (ret < 0) {
        fprintf(stderr, "[FFmpeg-Video] Failed to find stream info\n");
        close();
        return false;
    }

    usingAVIO = false;

    // Set up codec, scaler, etc.
    if (!setupDecoder()) {
        close();
        return false;
    }

    return true;
}

bool FFmpegVideoDecoder::setupDecoder()
{
    if (!formatContext) {
        return false;
    }

    // Find best video stream
    videoStreamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO,
                                           -1, -1, nullptr, 0);
    if (videoStreamIndex < 0) {
        fprintf(stderr, "[FFmpeg-Video] No video stream found\n");
        return false;
    }

    // Also find audio stream index (informational)
    audioStreamIdx = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO,
                                          -1, -1, nullptr, 0);

    AVStream* stream = formatContext->streams[videoStreamIndex];
    
    // Get codec parameters
    const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec) {
        fprintf(stderr, "[FFmpeg-Video] Unsupported codec (id=%d)\n",
                stream->codecpar->codec_id);
        return false;
    }

    // Allocate codec context
    codecContext = avcodec_alloc_context3(codec);
    if (!codecContext) {
        fprintf(stderr, "[FFmpeg-Video] Failed to allocate codec context\n");
        return false;
    }

    // Copy codec parameters
    int ret = avcodec_parameters_to_context(codecContext, stream->codecpar);
    if (ret < 0) {
        fprintf(stderr, "[FFmpeg-Video] Failed to copy codec parameters\n");
        return false;
    }

    // Open codec
    ret = avcodec_open2(codecContext, codec, nullptr);
    if (ret < 0) {
        char errBuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errBuf, sizeof(errBuf));
        fprintf(stderr, "[FFmpeg-Video] Failed to open codec: %s\n", errBuf);
        return false;
    }

    // Store video information
    currentWidth = codecContext->width;
    currentHeight = codecContext->height;
    
    // Calculate FPS
    if (stream->avg_frame_rate.den != 0) {
        currentFps = av_q2d(stream->avg_frame_rate);
    } else if (stream->r_frame_rate.den != 0) {
        currentFps = av_q2d(stream->r_frame_rate);
    } else {
        currentFps = 30.0; // Default
    }
    
    // Calculate total duration
    if (stream->duration != AV_NOPTS_VALUE) {
        totalDuration = stream->duration * av_q2d(stream->time_base);
    } else if (formatContext->duration != AV_NOPTS_VALUE) {
        totalDuration = formatContext->duration / (double)AV_TIME_BASE;
    }

    // Initialize scaler for RGBA output
    swsContext = sws_getContext(
        currentWidth, currentHeight, codecContext->pix_fmt,
        currentWidth, currentHeight, AV_PIX_FMT_RGBA,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );
    
    if (!swsContext) {
        fprintf(stderr, "[FFmpeg-Video] Failed to create scaler context\n");
        return false;
    }

    // Allocate frame and buffer
    frame = av_frame_alloc();
    frameRGB = av_frame_alloc();
    if (!frame || !frameRGB) {
        fprintf(stderr, "[FFmpeg-Video] Failed to allocate frames\n");
        return false;
    }

    // Allocate buffer for RGBA output
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA,
                                             currentWidth, currentHeight, 1);
    buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));
    if (!buffer) {
        fprintf(stderr, "[FFmpeg-Video] Failed to allocate buffer\n");
        return false;
    }

    // Setup frame
    av_image_fill_arrays(frameRGB->data, frameRGB->linesize, buffer,
                          AV_PIX_FMT_RGBA, currentWidth, currentHeight, 1);

    // Allocate packet
    packet = av_packet_alloc();
    if (!packet) {
        fprintf(stderr, "[FFmpeg-Video] Failed to allocate packet\n");
        return false;
    }

    isInitialized = true;
    fprintf(stderr, "[FFmpeg-Video] Decoder ready: %dx%d fps=%.2f duration=%.2fs "
            "audio_stream=%d\n",
            currentWidth, currentHeight, currentFps, totalDuration,
            audioStreamIdx);

    return true;
}

void FFmpegVideoDecoder::close()
{
    if (buffer) {
        av_free(buffer);
        buffer = nullptr;
    }
    if (packet) {
        av_packet_free(&packet);
        packet = nullptr;
    }
    if (frame) {
        av_frame_free(&frame);
        frame = nullptr;
    }
    if (frameRGB) {
        av_frame_free(&frameRGB);
        frameRGB = nullptr;
    }
    if (swsContext) {
        sws_freeContext(swsContext);
        swsContext = nullptr;
    }
    if (codecContext) {
        avcodec_free_context(&codecContext);
        codecContext = nullptr;
    }

    if (usingAVIO) {
        // AVIO bridge owns the format context
        avioBridge_.close();
        formatContext = nullptr;
    } else {
        if (formatContext) {
            avformat_close_input(&formatContext);
            formatContext = nullptr;
        }
    }
    
    videoStreamIndex = -1;
    audioStreamIdx = -1;
    isInitialized = false;
    usingAVIO = false;
    currentPosition = 0;
}

bool FFmpegVideoDecoder::decodeNextFrame(uint8_t** outputData, int* linesize)
{
    if (!isInitialized || !outputData || !linesize) {
        return false;
    }

    while (true) {
        // Try to receive decoded frames
        int ret = avcodec_receive_frame(codecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            // Need more data, read next packet
            ret = av_read_frame(formatContext, packet);
            if (ret < 0) {
                if (ret == AVERROR_EOF) {
                    // End of file
                    return false;
                }
                fprintf(stderr, "[FFmpeg-Video] Error reading frame\n");
                return false;
            }

            if (packet->stream_index == videoStreamIndex) {
                ret = avcodec_send_packet(codecContext, packet);
                if (ret < 0) {
                    fprintf(stderr, "[FFmpeg-Video] Error sending packet\n");
                    av_packet_unref(packet);
                    return false;
                }
            }
            av_packet_unref(packet);
            continue;
        } else if (ret < 0) {
            if (ret == AVERROR_EOF) {
                return false;
            }
            fprintf(stderr, "[FFmpeg-Video] Error receiving frame\n");
            return false;
        }

        // Convert to RGBA
        sws_scale(swsContext,
                   frame->data, frame->linesize, 0, currentHeight,
                   frameRGB->data, frameRGB->linesize);

        // Update position
        if (frame->pts != AV_NOPTS_VALUE) {
            AVStream* stream = formatContext->streams[videoStreamIndex];
            currentPosition = frame->pts * av_q2d(stream->time_base);
        }

        av_frame_unref(frame);

        // Output the frame
        *outputData = frameRGB->data[0];
        *linesize = frameRGB->linesize[0];
        
        return true;
    }
}

bool FFmpegVideoDecoder::seek(double positionSeconds)
{
    if (!isInitialized || !formatContext) {
        return false;
    }

    int64_t timestamp = (int64_t)(positionSeconds * AV_TIME_BASE);
    int ret = av_seek_frame(formatContext, -1, timestamp, AVSEEK_FLAG_BACKWARD);
    if (ret < 0) {
        fprintf(stderr, "[FFmpeg-Video] Seek failed\n");
        return false;
    }

    avcodec_flush_buffers(codecContext);
    currentPosition = positionSeconds;
    return true;
}

double FFmpegVideoDecoder::getDuration() const
{
    return totalDuration;
}

double FFmpegVideoDecoder::getPosition() const
{
    return currentPosition;
}

int FFmpegVideoDecoder::getWidth() const
{
    return currentWidth;
}

int FFmpegVideoDecoder::getHeight() const
{
    return currentHeight;
}

double FFmpegVideoDecoder::getFps() const
{
    return currentFps;
}

bool FFmpegVideoDecoder::isEOF() const
{
    return isInitialized && currentPosition >= totalDuration;
}

int FFmpegVideoDecoder::getAudioStreamIndex() const
{
    return audioStreamIdx;
}

} // namespace video
} // namespace krkr

#endif // KRKRSDL2_FFMPEG_ENABLED
