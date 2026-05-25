/*
 * FFmpeg Wave Decoder Creator for KRKRSDL2
 * Copyright (c) Kirikiri SDL2 Developers
 * SPDX-License-Identifier: MIT
 *
 * Phase 3: FFmpeg-based audio decoder integrated into KRKR's
 * WaveSoundBuffer pipeline via the tTVPWaveDecoder interface.
 */

#include "FFmpegWaveDecoder.h"

#if KRKRSDL2_FFMPEG_ENABLED

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}

#include "tjsCommHead.h"
#include "StorageIntf.h"
#include "DebugIntf.h"
#include "LogFilter.h"

#include <cstring>
#include <algorithm>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

static tjs_uint64 TVPAudioDiagFFmpegWaveOpenCount = 0;
static tjs_uint64 TVPAudioDiagFFmpegWaveRenderCallCount = 0;
static tjs_uint64 TVPAudioDiagFFmpegWaveRenderedSamples = 0;
static tjs_uint64 TVPAudioDiagFFmpegWaveRenderedBytes = 0;
static double TVPAudioDiagFFmpegWaveLastRenderLogMs = 0.0;

extern "C" tjs_uint64 KRKRSDL2AudioDiagGetFFmpegWaveOpenCount()
{
    return TVPAudioDiagFFmpegWaveOpenCount;
}

extern "C" tjs_uint64 KRKRSDL2AudioDiagGetFFmpegWaveRenderCallCount()
{
    return TVPAudioDiagFFmpegWaveRenderCallCount;
}

extern "C" tjs_uint64 KRKRSDL2AudioDiagGetFFmpegWaveRenderedSamples()
{
    return TVPAudioDiagFFmpegWaveRenderedSamples;
}

extern "C" tjs_uint64 KRKRSDL2AudioDiagGetFFmpegWaveRenderedBytes()
{
    return TVPAudioDiagFFmpegWaveRenderedBytes;
}

// ---------------------------------------------------------------------------
// tTVPWD_FFmpeg
// ---------------------------------------------------------------------------

tTVPWD_FFmpeg::tTVPWD_FFmpeg()
    : fmtCtx_(nullptr)
    , codecCtx_(nullptr)
    , swrCtx_(nullptr)
    , packet_(nullptr)
    , frame_(nullptr)
    , audioStreamIdx_(-1)
    , isOpen_(false)
    , eof_(false)
    , residualBuf_(nullptr)
    , residualSamples_(0)
    , residualOffset_(0)
    , decodedSamples_(0)
{
    memset(&format_, 0, sizeof(format_));
}

tTVPWD_FFmpeg::~tTVPWD_FFmpeg()
{
    if (packet_) { av_packet_free(&packet_); packet_ = nullptr; }
    if (frame_)  { av_frame_free(&frame_);   frame_  = nullptr; }
    if (swrCtx_) { swr_free(&swrCtx_);       swrCtx_ = nullptr; }
    if (codecCtx_) { avcodec_free_context(&codecCtx_); codecCtx_ = nullptr; }
    // fmtCtx_ is owned by avioBridge_
    avioBridge_.close();
    fmtCtx_ = nullptr;

    if (residualBuf_) {
        av_free(residualBuf_);
        residualBuf_ = nullptr;
    }
    isOpen_ = false;
}

bool tTVPWD_FFmpeg::Open(const ttstr& storagename)
{
    std::string name = storagename.AsNarrowStdString();
#ifdef __EMSCRIPTEN__
    double perfStartMs = TVPPerfEnabled() ? emscripten_get_now() : 0.0;
    double afterAvioMs = perfStartMs;
    double afterStreamMs = perfStartMs;
    double afterCodecFindMs = perfStartMs;
    double afterCodecAllocMs = perfStartMs;
    double afterCodecParamsMs = perfStartMs;
    double afterCodecOpenMs = perfStartMs;
    double afterSwrMs = perfStartMs;
    double afterAllocMs = perfStartMs;
#endif

    fprintf(stderr, "[FFmpeg-WaveDec] Opening: '%s'\n", name.c_str());

    // Open via AVIO bridge
    if (!avioBridge_.openFromKRKRStorage(name, &fmtCtx_)) {
        fprintf(stderr, "[FFmpeg-WaveDec] AVIO open failed for '%s'\n", name.c_str());
        return false;
    }
#ifdef __EMSCRIPTEN__
    if(TVPPerfEnabled()) afterAvioMs = emscripten_get_now();
#endif

    // Find audio stream
    audioStreamIdx_ = av_find_best_stream(fmtCtx_, AVMEDIA_TYPE_AUDIO,
                                           -1, -1, nullptr, 0);
    if (audioStreamIdx_ < 0) {
        fprintf(stderr, "[FFmpeg-WaveDec] No audio stream found in '%s'\n", name.c_str());
        return false;
    }

    AVStream* stream = fmtCtx_->streams[audioStreamIdx_];
#ifdef __EMSCRIPTEN__
    if(TVPPerfEnabled()) afterStreamMs = emscripten_get_now();
#endif

    // Find and open codec
    const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec) {
        fprintf(stderr, "[FFmpeg-WaveDec] Unsupported codec for '%s'\n", name.c_str());
        return false;
    }
#ifdef __EMSCRIPTEN__
    if(TVPPerfEnabled()) afterCodecFindMs = emscripten_get_now();
#endif

    codecCtx_ = avcodec_alloc_context3(codec);
    if (!codecCtx_) return false;
#ifdef __EMSCRIPTEN__
    if(TVPPerfEnabled()) afterCodecAllocMs = emscripten_get_now();
#endif

    int ret = avcodec_parameters_to_context(codecCtx_, stream->codecpar);
    if (ret < 0) return false;
#ifdef __EMSCRIPTEN__
    if(TVPPerfEnabled()) afterCodecParamsMs = emscripten_get_now();
#endif

    ret = avcodec_open2(codecCtx_, codec, nullptr);
    if (ret < 0) {
        char errBuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errBuf, sizeof(errBuf));
        fprintf(stderr, "[FFmpeg-WaveDec] Codec open failed: %s\n", errBuf);
        return false;
    }
#ifdef __EMSCRIPTEN__
    if(TVPPerfEnabled()) afterCodecOpenMs = emscripten_get_now();
#endif

    // Determine output format
    // KRKR audio pipeline expects s16 interleaved PCM
    int srcChannels = codecCtx_->ch_layout.nb_channels;
    int srcRate = codecCtx_->sample_rate;

    // Output: keep source channel count (1 or 2), keep source sample rate, s16
    int outChannels = (srcChannels >= 2) ? 2 : 1;
    int outRate = srcRate;

    // Fill KRKR format struct
    format_.SamplesPerSec = outRate;
    format_.Channels = outChannels;
    format_.BitsPerSample = 16;
    format_.BytesPerSample = 2;
    format_.IsFloat = false;
    format_.SpeakerConfig = 0;
    format_.Seekable = true;

    // Calculate total samples
    if (stream->duration != AV_NOPTS_VALUE) {
        double durationSec = stream->duration * av_q2d(stream->time_base);
        format_.TotalSamples = (tjs_uint64)(durationSec * outRate);
        format_.TotalTime = (tjs_uint64)(durationSec * 1000.0);
    } else if (fmtCtx_->duration != AV_NOPTS_VALUE) {
        double durationSec = fmtCtx_->duration / (double)AV_TIME_BASE;
        format_.TotalSamples = (tjs_uint64)(durationSec * outRate);
        format_.TotalTime = (tjs_uint64)(durationSec * 1000.0);
    } else {
        format_.TotalSamples = 0;
        format_.TotalTime = 0;
    }

    // Initialize resampler
    swrCtx_ = swr_alloc();
    if (!swrCtx_) return false;

    av_opt_set_chlayout(swrCtx_, "in_chlayout", &codecCtx_->ch_layout, 0);
    av_opt_set_int(swrCtx_, "in_sample_rate", srcRate, 0);
    av_opt_set_sample_fmt(swrCtx_, "in_sample_fmt", codecCtx_->sample_fmt, 0);

    AVChannelLayout outLayout;
    if (outChannels == 1) {
        outLayout = AV_CHANNEL_LAYOUT_MONO;
    } else {
        outLayout = AV_CHANNEL_LAYOUT_STEREO;
    }
    av_opt_set_chlayout(swrCtx_, "out_chlayout", &outLayout, 0);
    av_opt_set_int(swrCtx_, "out_sample_rate", outRate, 0);
    av_opt_set_sample_fmt(swrCtx_, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

    ret = swr_init(swrCtx_);
    if (ret < 0) {
        char errBuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errBuf, sizeof(errBuf));
        fprintf(stderr, "[FFmpeg-WaveDec] SWR init failed: %s\n", errBuf);
        return false;
    }
#ifdef __EMSCRIPTEN__
    if(TVPPerfEnabled()) afterSwrMs = emscripten_get_now();
#endif

    // Allocate packet/frame
    packet_ = av_packet_alloc();
    frame_ = av_frame_alloc();
    if (!packet_ || !frame_) return false;

    // Allocate residual buffer (generous: 1 second worth of output)
    int residualBufSize = outRate * outChannels * 2; // s16
    residualBuf_ = (uint8_t*)av_malloc(residualBufSize);
    if (!residualBuf_) return false;
#ifdef __EMSCRIPTEN__
    if(TVPPerfEnabled()) afterAllocMs = emscripten_get_now();
#endif

    isOpen_ = true;
    eof_ = false;
    decodedSamples_ = 0;
    residualSamples_ = 0;
    residualOffset_ = 0;
    TVPAudioDiagFFmpegWaveOpenCount++;

    fprintf(stderr, "[FFmpeg-WaveDec] Opened OK: '%s' rate=%d ch=%d total=%llu samples\n",
            name.c_str(), outRate, outChannels,
            (unsigned long long)format_.TotalSamples);
#ifdef __EMSCRIPTEN__
    if(TVPPerfEnabled()) {
        double perfEndMs = emscripten_get_now();
        fprintf(stderr,
            "[PERF] ffmpeg.wave.open storage=%s total_ms=%.3f avio_ms=%.3f stream_ms=%.3f codec_find_ms=%.3f codec_alloc_ms=%.3f codec_params_ms=%.3f codec_open_ms=%.3f swr_ms=%.3f alloc_ms=%.3f rate=%d ch=%d total_samples=%llu avio_reads=%d avio_seeks=%d avio_bytes=%lld\n",
            name.c_str(),
            perfEndMs - perfStartMs,
            afterAvioMs - perfStartMs,
            afterStreamMs - afterAvioMs,
            afterCodecFindMs - afterStreamMs,
            afterCodecAllocMs - afterCodecFindMs,
            afterCodecParamsMs - afterCodecAllocMs,
            afterCodecOpenMs - afterCodecParamsMs,
            afterSwrMs - afterCodecOpenMs,
            afterAllocMs - afterSwrMs,
            outRate,
            outChannels,
            (unsigned long long)format_.TotalSamples,
            avioBridge_.getReadCallCount(),
            avioBridge_.getSeekCallCount(),
            (long long)avioBridge_.getBytesRead());
    }
#endif

    return true;
}

void tTVPWD_FFmpeg::GetFormat(tTVPWaveFormat& format)
{
    format = format_;
}

bool tTVPWD_FFmpeg::Render(void* buf, tjs_uint bufsamplelen, tjs_uint& rendered)
{
    TVPAudioDiagFFmpegWaveRenderCallCount++;
#ifdef __EMSCRIPTEN__
    double perfStartMs = TVPPerfEnabled() ? emscripten_get_now() : 0.0;
    int readFrames = 0;
    int receiveFrames = 0;
    int convertFrames = 0;
#endif
    if (!isOpen_) {
        rendered = 0;
        return false;
    }

    int bytesPerSample = format_.BytesPerSample * format_.Channels; // frame size
    uint8_t* outBuf = (uint8_t*)buf;
    tjs_uint totalWritten = 0;

    while (totalWritten < bufsamplelen) {
        // First drain residual buffer
        if (residualSamples_ > 0) {
            int toWrite = std::min((int)(bufsamplelen - totalWritten), residualSamples_);
            memcpy(outBuf + totalWritten * bytesPerSample,
                   residualBuf_ + residualOffset_,
                   toWrite * bytesPerSample);
            totalWritten += toWrite;
            residualSamples_ -= toWrite;
            residualOffset_ += toWrite * bytesPerSample;
            if (residualSamples_ == 0) {
                residualOffset_ = 0;
            }
            continue;
        }

        if (eof_) break;

        // Try to receive a decoded frame
        int ret = avcodec_receive_frame(codecCtx_, frame_);
        if (ret == AVERROR(EAGAIN)) {
            // Need more data
            ret = av_read_frame(fmtCtx_, packet_);
#ifdef __EMSCRIPTEN__
            readFrames++;
#endif
            if (ret < 0) {
                if (ret == AVERROR_EOF) {
                    // Flush decoder
                    avcodec_send_packet(codecCtx_, nullptr);
                    eof_ = true;
                    // Try one more receive
                    continue;
                }
                eof_ = true;
                break;
            }
            if (packet_->stream_index == audioStreamIdx_) {
                ret = avcodec_send_packet(codecCtx_, packet_);
                // Ignore EAGAIN/EOF errors from send_packet
            }
            av_packet_unref(packet_);
            continue;
        } else if (ret == AVERROR_EOF) {
            eof_ = true;
            break;
        } else if (ret < 0) {
            eof_ = true;
            break;
        }

        // Resample frame to s16 output
        int outSamples = swr_get_out_samples(swrCtx_, frame_->nb_samples);
        if (outSamples <= 0) {
            av_frame_unref(frame_);
            continue;
        }

        // How many can we write directly?
        int directWrite = std::min(outSamples, (int)(bufsamplelen - totalWritten));

        // Convert directly into output buffer
        uint8_t* directPtr = outBuf + totalWritten * bytesPerSample;
        int converted = swr_convert(swrCtx_, &directPtr, directWrite,
                                     (const uint8_t**)frame_->extended_data,
                                     frame_->nb_samples);

        if (converted > 0) {
            totalWritten += converted;
            decodedSamples_ += converted;
#ifdef __EMSCRIPTEN__
            convertFrames++;
#endif
        }

        // If there are remaining resampled samples, drain into residual buf
        // (swr_convert may have buffered internally)
        if (converted < outSamples) {
            // Flush remaining from swr
            int maxResidual = format_.SamplesPerSec; // 1 sec max
            uint8_t* residPtr = residualBuf_;
            int residual = swr_convert(swrCtx_, &residPtr, maxResidual,
                                        nullptr, 0);
            if (residual > 0) {
                residualSamples_ = residual;
                residualOffset_ = 0;
            }
        }

        av_frame_unref(frame_);
#ifdef __EMSCRIPTEN__
        receiveFrames++;
#endif
    }

    rendered = totalWritten;
    TVPAudioDiagFFmpegWaveRenderedSamples += totalWritten;
    TVPAudioDiagFFmpegWaveRenderedBytes +=
        (tjs_uint64)totalWritten * format_.BytesPerSample * format_.Channels;

#ifdef __EMSCRIPTEN__
    if(TVPPerfEnabled()) {
        double perfEndMs = emscripten_get_now();
        bool printSample = perfEndMs - perfStartMs >= 4.0 ||
            TVPAudioDiagFFmpegWaveRenderCallCount <= 8 ||
            (TVPAudioDiagFFmpegWaveLastRenderLogMs > 0.0 &&
             perfEndMs - TVPAudioDiagFFmpegWaveLastRenderLogMs >= 1000.0);
        if(printSample) {
            TVPAudioDiagFFmpegWaveLastRenderLogMs = perfEndMs;
            fprintf(stderr,
                "[PERF] ffmpeg.wave.render call=%llu request_samples=%u rendered=%u ms=%.3f read_frames=%d receive_frames=%d convert_frames=%d eof=%d avio_reads=%d avio_seeks=%d avio_bytes=%lld\n",
                (unsigned long long)TVPAudioDiagFFmpegWaveRenderCallCount,
                (unsigned)bufsamplelen,
                (unsigned)rendered,
                perfEndMs - perfStartMs,
                readFrames,
                receiveFrames,
                convertFrames,
                eof_ ? 1 : 0,
                avioBridge_.getReadCallCount(),
                avioBridge_.getSeekCallCount(),
                (long long)avioBridge_.getBytesRead());
        }
    }
#endif
    // Return false if we couldn't fill the buffer (EOF)
    return totalWritten >= bufsamplelen;
}

bool tTVPWD_FFmpeg::SetPosition(tjs_uint64 samplepos)
{
    if (!isOpen_ || !fmtCtx_) return false;

    // Convert sample position to timestamp
    AVStream* stream = fmtCtx_->streams[audioStreamIdx_];
    double timeSec = (double)samplepos / format_.SamplesPerSec;
    int64_t timestamp = (int64_t)(timeSec / av_q2d(stream->time_base));

    int ret = av_seek_frame(fmtCtx_, audioStreamIdx_, timestamp,
                             AVSEEK_FLAG_BACKWARD);
    if (ret < 0) {
        fprintf(stderr, "[FFmpeg-WaveDec] Seek failed to sample %llu\n",
                (unsigned long long)samplepos);
        return false;
    }

    avcodec_flush_buffers(codecCtx_);
    // Also flush the resampler
    if (swrCtx_) {
        // Drain any buffered samples
        uint8_t* dummy = residualBuf_;
        swr_convert(swrCtx_, &dummy, format_.SamplesPerSec, nullptr, 0);
    }

    eof_ = false;
    decodedSamples_ = samplepos;
    residualSamples_ = 0;
    residualOffset_ = 0;

    return true;
}

// ---------------------------------------------------------------------------
// tTVPWDC_FFmpeg – Creator
// ---------------------------------------------------------------------------

tTVPWaveDecoder* tTVPWDC_FFmpeg::Create(const ttstr& storagename,
                                         const ttstr& extension)
{
    // Accept audio formats that FFmpeg can handle
    // Priority: let KRKR's built-in WAV/Opus decoders handle .wav/.opus first,
    // and FFmpeg handles everything else.
    if (extension == TJS_W(".ogg") ||
        extension == TJS_W(".mp3") ||
        extension == TJS_W(".flac") ||
        extension == TJS_W(".aac") ||
        extension == TJS_W(".m4a") ||
        extension == TJS_W(".wma") ||
        extension == TJS_W(".wmv"))    // wmv often used for audio-only in KRKR
    {
        tTVPWD_FFmpeg* decoder = new tTVPWD_FFmpeg();
        if (decoder->Open(storagename)) {
            return decoder;
        }
        delete decoder;
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------

static tTVPWDC_FFmpeg TVPFFmpegWaveDecoderCreator;
static bool TVPFFmpegWaveDecoderCreatorRegistered = false;

void TVPRegisterFFmpegWaveDecoderCreator()
{
    if (!TVPFFmpegWaveDecoderCreatorRegistered) {
        TVPRegisterWaveDecoderCreator(&TVPFFmpegWaveDecoderCreator);
        TVPFFmpegWaveDecoderCreatorRegistered = true;
        fprintf(stderr, "[FFmpeg-WaveDec] Registered FFmpeg wave decoder creator\n");
    }
}

#endif // KRKRSDL2_FFMPEG_ENABLED
