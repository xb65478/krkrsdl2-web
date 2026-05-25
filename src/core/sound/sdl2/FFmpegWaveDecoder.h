/*
 * FFmpeg Wave Decoder Creator for KRKRSDL2
 * Copyright (c) Kirikiri SDL2 Developers
 * SPDX-License-Identifier: MIT
 *
 * Phase 3: Implements tTVPWaveDecoderCreator / tTVPWaveDecoder using FFmpeg
 * via the AVIO bridge. This allows WaveSoundBuffer to decode .ogg, .mp3,
 * .flac, .aac, .wmv/.wma (and more) audio through the existing KRKR audio
 * pipeline and OpenAL output.
 *
 * The decoder outputs s16 stereo 44100 Hz PCM to match the standard
 * KRKR audio format expected by the loop manager and sound buffer.
 */

#pragma once

#if KRKRSDL2_FFMPEG_ENABLED

#include "WaveIntf.h"   // tTVPWaveDecoder, tTVPWaveDecoderCreator, tTVPWaveFormat
#include "FFmpegAVIO.h"

struct AVFormatContext;
struct AVCodecContext;
struct SwrContext;
struct AVPacket;
struct AVFrame;

/**
 * FFmpeg-based tTVPWaveDecoder.
 *
 * Wraps an FFmpeg decode pipeline (AVIO bridge -> demux -> decode -> resample)
 * behind the tTVPWaveDecoder interface that KRKR's WaveSoundBuffer expects.
 *
 * Output format: s16 interleaved, stereo (or mono), at the source sample rate.
 * If the source is not s16/stereo, libswresample converts it.
 */
class tTVPWD_FFmpeg : public tTVPWaveDecoder
{
    krkr::avio::FFmpegAVIOBridge avioBridge_;
    AVFormatContext* fmtCtx_;
    AVCodecContext*  codecCtx_;
    SwrContext*      swrCtx_;
    AVPacket*        packet_;
    AVFrame*         frame_;
    int              audioStreamIdx_;
    bool             isOpen_;
    bool             eof_;

    tTVPWaveFormat   format_;

    // Residual resampled samples buffer
    uint8_t*         residualBuf_;
    int              residualSamples_;   // remaining samples in residual buf
    int              residualOffset_;    // byte offset into residual buf

    // Position tracking
    tjs_uint64       decodedSamples_;    // total samples decoded so far

public:
    tTVPWD_FFmpeg();
    ~tTVPWD_FFmpeg();

    /**
     * Open audio from a KRKR storage name.
     * @return true if open + probe + codec init succeeded.
     */
    bool Open(const ttstr& storagename);

    // tTVPWaveDecoder interface
    void GetFormat(tTVPWaveFormat& format) override;
    bool Render(void* buf, tjs_uint bufsamplelen, tjs_uint& rendered) override;
    bool SetPosition(tjs_uint64 samplepos) override;
};

/**
 * FFmpeg-based tTVPWaveDecoderCreator.
 *
 * Registered into the KRKR wave decoder chain so that
 * TVPCreateWaveDecoder picks it up for supported extensions.
 */
class tTVPWDC_FFmpeg : public tTVPWaveDecoderCreator
{
public:
    tTVPWaveDecoder* Create(const ttstr& storagename,
                            const ttstr& extension) override;
};

/**
 * Register the FFmpeg wave decoder creator.
 * Call once during initialization.
 */
void TVPRegisterFFmpegWaveDecoderCreator();

#endif // KRKRSDL2_FFMPEG_ENABLED
