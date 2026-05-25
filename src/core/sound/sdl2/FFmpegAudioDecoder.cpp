/*
 * FFmpeg Audio Decoder for KRKRSDL2
 * Copyright (c) Kirikiri SDL2 Developers
 * SPDX-License-Identifier: MIT
 *
 * Phase 2: open() now uses FFmpegAVIOBridge for KRKR Storage / XP3 access.
 * openPath() retains the old plain-filesystem path for testing.
 */

#include "FFmpegAudioDecoder.h"

#if KRKRSDL2_FFMPEG_ENABLED

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}

#include <cstring>
#include <algorithm>

namespace krkr {
namespace audio {

FFmpegAudioDecoder::FFmpegAudioDecoder()
    : formatContext(nullptr)
    , codecContext(nullptr)
    , swrContext(nullptr)
    , audioStreamIndex(-1)
    , packet(nullptr)
    , frame(nullptr)
    , isInitialized(false)
    , usingAVIO(false)
    , currentSampleRate(0)
    , currentChannels(0)
    , currentBitrate(0)
    , totalDuration(0)
    , currentPosition(0)
{
}

FFmpegAudioDecoder::~FFmpegAudioDecoder()
{
    close();
}

bool FFmpegAudioDecoder::open(const std::string& storageName)
{
    if (isInitialized) {
        close();
    }

    fprintf(stderr, "[FFmpeg-Audio] Opening via KRKR Storage: '%s'\n",
            storageName.c_str());

    // Use AVIO bridge to open from KRKR Storage / XP3
    if (!avioBridge_.openFromKRKRStorage(storageName, &formatContext)) {
        fprintf(stderr, "[FFmpeg-Audio] AVIO bridge failed for '%s'\n",
                storageName.c_str());
        return false;
    }

    usingAVIO = true;

    // Set up codec, resampler, etc.
    if (!setupDecoder()) {
        close();
        return false;
    }

    return true;
}

bool FFmpegAudioDecoder::openPath(const std::string& path)
{
    if (isInitialized) {
        close();
    }

    fprintf(stderr, "[FFmpeg-Audio] Opening via path: '%s'\n", path.c_str());

    // Open input file using plain filesystem path (legacy mode)
    int ret = avformat_open_input(&formatContext, path.c_str(), nullptr, nullptr);
    if (ret < 0) {
        char errBuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errBuf, sizeof(errBuf));
        fprintf(stderr, "[FFmpeg-Audio] Failed to open input: %s (%s)\n",
                path.c_str(), errBuf);
        return false;
    }

    // Retrieve stream information
    ret = avformat_find_stream_info(formatContext, nullptr);
    if (ret < 0) {
        fprintf(stderr, "[FFmpeg-Audio] Failed to find stream info\n");
        close();
        return false;
    }

    usingAVIO = false;

    // Set up codec, resampler, etc.
    if (!setupDecoder()) {
        close();
        return false;
    }

    return true;
}

bool FFmpegAudioDecoder::setupDecoder()
{
    if (!formatContext) {
        return false;
    }

    // Find best audio stream
    audioStreamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO,
                                            -1, -1, nullptr, 0);
    if (audioStreamIndex < 0) {
        fprintf(stderr, "[FFmpeg-Audio] No audio stream found\n");
        return false;
    }

    AVStream* stream = formatContext->streams[audioStreamIndex];
    
    // Get codec parameters
    const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec) {
        fprintf(stderr, "[FFmpeg-Audio] Unsupported codec (id=%d)\n",
                stream->codecpar->codec_id);
        return false;
    }

    // Allocate codec context
    codecContext = avcodec_alloc_context3(codec);
    if (!codecContext) {
        fprintf(stderr, "[FFmpeg-Audio] Failed to allocate codec context\n");
        return false;
    }

    // Copy codec parameters
    int ret = avcodec_parameters_to_context(codecContext, stream->codecpar);
    if (ret < 0) {
        fprintf(stderr, "[FFmpeg-Audio] Failed to copy codec parameters\n");
        return false;
    }

    // Open codec
    ret = avcodec_open2(codecContext, codec, nullptr);
    if (ret < 0) {
        char errBuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errBuf, sizeof(errBuf));
        fprintf(stderr, "[FFmpeg-Audio] Failed to open codec: %s\n", errBuf);
        return false;
    }

    // Store audio information
    currentSampleRate = codecContext->sample_rate;
    currentChannels = codecContext->ch_layout.nb_channels;
    currentBitrate = codecContext->bit_rate;
    
    // Calculate total duration
    if (stream->duration != AV_NOPTS_VALUE) {
        totalDuration = stream->duration * av_q2d(stream->time_base);
    } else if (formatContext->duration != AV_NOPTS_VALUE) {
        totalDuration = formatContext->duration / (double)AV_TIME_BASE;
    }

    // Initialize resampler for output format conversion
    swrContext = swr_alloc();
    if (!swrContext) {
        fprintf(stderr, "[FFmpeg-Audio] Failed to allocate resampler\n");
        return false;
    }

    // Set resampler options
    av_opt_set_chlayout(swrContext, "in_chlayout", &codecContext->ch_layout, 0);
    av_opt_set_int(swrContext, "in_sample_rate", codecContext->sample_rate, 0);
    av_opt_set_sample_fmt(swrContext, "in_sample_fmt", codecContext->sample_fmt, 0);

    // Output: stereo, 44100Hz, s16
    AVChannelLayout out_chlayout = AV_CHANNEL_LAYOUT_STEREO;
    av_opt_set_chlayout(swrContext, "out_chlayout", &out_chlayout, 0);
    av_opt_set_int(swrContext, "out_sample_rate", 44100, 0);
    av_opt_set_sample_fmt(swrContext, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

    // Initialize resampler
    ret = swr_init(swrContext);
    if (ret < 0) {
        char errBuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errBuf, sizeof(errBuf));
        fprintf(stderr, "[FFmpeg-Audio] Failed to initialize resampler: %s\n",
                errBuf);
        return false;
    }

    // Allocate packet and frame
    packet = av_packet_alloc();
    frame = av_frame_alloc();
    if (!packet || !frame) {
        fprintf(stderr, "[FFmpeg-Audio] Failed to allocate packet/frame\n");
        return false;
    }

    isInitialized = true;
    fprintf(stderr, "[FFmpeg-Audio] Decoder ready: sample_rate=%d channels=%d "
            "bitrate=%lld duration=%.2fs\n",
            currentSampleRate, currentChannels,
            (long long)currentBitrate, totalDuration);

    return true;
}

void FFmpegAudioDecoder::close()
{
    if (packet) {
        av_packet_free(&packet);
        packet = nullptr;
    }
    if (frame) {
        av_frame_free(&frame);
        frame = nullptr;
    }
    if (swrContext) {
        swr_free(&swrContext);
        swrContext = nullptr;
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
    
    audioStreamIndex = -1;
    isInitialized = false;
    usingAVIO = false;
    currentPosition = 0;
}

int FFmpegAudioDecoder::decode(uint8_t* outputBuffer, int bufferSize)
{
    if (!isInitialized || !outputBuffer) {
        return -1;
    }

    int totalSamplesWritten = 0;
    int bytesPerSample = 2 * 2; // 16-bit stereo
    int maxSamples = bufferSize / bytesPerSample;

    while (totalSamplesWritten < maxSamples) {
        // Try to receive decoded frames
        int ret = avcodec_receive_frame(codecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            // Need more data, read next packet
            ret = av_read_frame(formatContext, packet);
            if (ret < 0) {
                if (ret == AVERROR_EOF) {
                    // End of file
                    break;
                }
                fprintf(stderr, "[FFmpeg-Audio] Error reading frame\n");
                return -1;
            }

            if (packet->stream_index == audioStreamIndex) {
                ret = avcodec_send_packet(codecContext, packet);
                if (ret < 0) {
                    fprintf(stderr, "[FFmpeg-Audio] Error sending packet\n");
                    av_packet_unref(packet);
                    return -1;
                }
            }
            av_packet_unref(packet);
            continue;
        } else if (ret < 0) {
            if (ret == AVERROR_EOF) {
                break;
            }
            fprintf(stderr, "[FFmpeg-Audio] Error receiving frame\n");
            return -1;
        }

        // Convert to output format
        int outSamples = swr_get_out_samples(swrContext, frame->nb_samples);
        if (outSamples < 0) {
            fprintf(stderr, "[FFmpeg-Audio] Error getting output samples\n");
            av_frame_unref(frame);
            return -1;
        }

        int samplesToWrite = std::min(outSamples, maxSamples - totalSamplesWritten);
        uint8_t* outPtr = outputBuffer + (totalSamplesWritten * bytesPerSample);

        int converted = swr_convert(swrContext, &outPtr, samplesToWrite,
                                     (const uint8_t**)frame->extended_data,
                                     frame->nb_samples);
        
        if (converted < 0) {
            fprintf(stderr, "[FFmpeg-Audio] Error converting samples\n");
            av_frame_unref(frame);
            return -1;
        }

        totalSamplesWritten += converted;
        currentPosition += (double)converted / 44100.0;
        av_frame_unref(frame);
    }

    return totalSamplesWritten * bytesPerSample;
}

bool FFmpegAudioDecoder::seek(double positionSeconds)
{
    if (!isInitialized || !formatContext) {
        return false;
    }

    int64_t timestamp = (int64_t)(positionSeconds * AV_TIME_BASE);
    int ret = av_seek_frame(formatContext, -1, timestamp, AVSEEK_FLAG_BACKWARD);
    if (ret < 0) {
        fprintf(stderr, "[FFmpeg-Audio] Seek failed\n");
        return false;
    }

    avcodec_flush_buffers(codecContext);
    currentPosition = positionSeconds;
    return true;
}

double FFmpegAudioDecoder::getDuration() const
{
    return totalDuration;
}

double FFmpegAudioDecoder::getPosition() const
{
    return currentPosition;
}

int FFmpegAudioDecoder::getSampleRate() const
{
    return currentSampleRate;
}

int FFmpegAudioDecoder::getChannels() const
{
    return currentChannels;
}

int64_t FFmpegAudioDecoder::getBitrate() const
{
    return currentBitrate;
}

bool FFmpegAudioDecoder::isEOF() const
{
    return isInitialized && currentPosition >= totalDuration;
}

} // namespace audio
} // namespace krkr

#endif // KRKRSDL2_FFMPEG_ENABLED
