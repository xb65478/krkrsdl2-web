/*
 * FFmpeg Audio Decoder for KRKRSDL2
 * Copyright (c) Kirikiri SDL2 Developers
 * SPDX-License-Identifier: MIT
 */

#pragma once

#if KRKRSDL2_FFMPEG_ENABLED

#include <string>
#include <cstdint>
#include "FFmpegAVIO.h"

struct AVFormatContext;
struct AVCodecContext;
struct SwrContext;
struct AVPacket;
struct AVFrame;

namespace krkr {
namespace audio {

/**
 * FFmpeg-based audio decoder for KRKRSDL2
 * Supports Ogg Vorbis, WMV/WMA, MP3, FLAC, AAC, etc.
 *
 * Phase 2: Uses FFmpegAVIOBridge to read from KRKR Storage / XP3
 * instead of plain filesystem paths.
 */
class FFmpegAudioDecoder {
public:
    FFmpegAudioDecoder();
    ~FFmpegAudioDecoder();

    /**
     * Open an audio file for decoding via KRKR Storage / XP3.
     * Uses FFmpegAVIOBridge for I/O through KRKR's virtual filesystem.
     * @param storageName KRKR storage name (e.g. "bgm01.ogg")
     * @return true if successful
     */
    bool open(const std::string& storageName);

    /**
     * Open an audio file for decoding via plain filesystem path.
     * Kept for backward compatibility and testing.
     * @param path Path to the audio file
     * @return true if successful
     */
    bool openPath(const std::string& path);

    /**
     * Close the decoder and release resources
     */
    void close();

    /**
     * Decode audio samples
     * @param outputBuffer Buffer to store decoded PCM samples (s16 stereo 44100Hz)
     * @param bufferSize Size of the output buffer in bytes
     * @return Number of bytes written, or -1 on error
     */
    int decode(uint8_t* outputBuffer, int bufferSize);

    /**
     * Seek to a position in the audio
     * @param positionSeconds Position in seconds
     * @return true if successful
     */
    bool seek(double positionSeconds);

    /**
     * Get the total duration of the audio
     * @return Duration in seconds
     */
    double getDuration() const;

    /**
     * Get the current playback position
     * @return Position in seconds
     */
    double getPosition() const;

    /**
     * Get the sample rate of the audio
     * @return Sample rate in Hz
     */
    int getSampleRate() const;

    /**
     * Get the number of channels
     * @return Number of channels
     */
    int getChannels() const;

    /**
     * Get the bitrate of the audio
     * @return Bitrate in bits per second
     */
    int64_t getBitrate() const;

    /**
     * Check if the decoder has reached the end of file
     * @return true if EOF
     */
    bool isEOF() const;

private:
    /**
     * Internal: set up codec, resampler, etc. after format context is ready.
     * @return true if successful
     */
    bool setupDecoder();

    krkr::avio::FFmpegAVIOBridge avioBridge_;  // KRKR Storage AVIO bridge
    AVFormatContext* formatContext;
    AVCodecContext* codecContext;
    SwrContext* swrContext;
    int audioStreamIndex;
    AVPacket* packet;
    AVFrame* frame;
    bool isInitialized;
    bool usingAVIO;  // true if opened via AVIO bridge
    
    int currentSampleRate;
    int currentChannels;
    int64_t currentBitrate;
    double totalDuration;
    double currentPosition;
};

} // namespace audio
} // namespace krkr

#endif // KRKRSDL2_FFMPEG_ENABLED
