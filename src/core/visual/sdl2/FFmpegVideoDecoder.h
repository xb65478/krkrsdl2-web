/*
 * FFmpeg Video Decoder for KRKRSDL2
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
struct SwsContext;
struct AVPacket;
struct AVFrame;

namespace krkr {
namespace video {

/**
 * FFmpeg-based video decoder for KRKRSDL2
 * Supports WMV, AVI, MP4, MKV, etc.
 *
 * Phase 2: Uses FFmpegAVIOBridge to read from KRKR Storage / XP3
 * instead of plain filesystem paths.
 */
class FFmpegVideoDecoder {
public:
    FFmpegVideoDecoder();
    ~FFmpegVideoDecoder();

    /**
     * Open a video file for decoding via KRKR Storage / XP3.
     * Uses FFmpegAVIOBridge for I/O through KRKR's virtual filesystem.
     * @param storageName KRKR storage name (e.g. "op.wmv")
     * @return true if successful
     */
    bool open(const std::string& storageName);

    /**
     * Open a video file for decoding via plain filesystem path.
     * Kept for backward compatibility and testing.
     * @param path Path to the video file
     * @return true if successful
     */
    bool openPath(const std::string& path);

    /**
     * Close the decoder and release resources
     */
    void close();

    /**
     * Decode the next video frame
     * @param outputData Pointer to store the RGBA frame data
     * @param linesize Pointer to store the linesize
     * @return true if a frame was decoded
     */
    bool decodeNextFrame(uint8_t** outputData, int* linesize);

    /**
     * Seek to a position in the video
     * @param positionSeconds Position in seconds
     * @return true if successful
     */
    bool seek(double positionSeconds);

    /**
     * Get the total duration of the video
     * @return Duration in seconds
     */
    double getDuration() const;

    /**
     * Get the current playback position
     * @return Position in seconds
     */
    double getPosition() const;

    /**
     * Get the video width
     * @return Width in pixels
     */
    int getWidth() const;

    /**
     * Get the video height
     * @return Height in pixels
     */
    int getHeight() const;

    /**
     * Get the video FPS
     * @return Frames per second
     */
    double getFps() const;

    /**
     * Check if the decoder has reached the end of file
     * @return true if EOF
     */
    bool isEOF() const;

    /**
     * Check if the decoder also has an audio stream
     * @return audio stream index, or -1 if no audio
     */
    int getAudioStreamIndex() const;

private:
    /**
     * Internal: set up codec, scaler, etc. after format context is ready.
     * @return true if successful
     */
    bool setupDecoder();

    krkr::avio::FFmpegAVIOBridge avioBridge_;  // KRKR Storage AVIO bridge
    AVFormatContext* formatContext;
    AVCodecContext* codecContext;
    SwsContext* swsContext;
    int videoStreamIndex;
    int audioStreamIdx;  // index of audio stream if present
    AVPacket* packet;
    AVFrame* frame;
    AVFrame* frameRGB;
    uint8_t* buffer;
    bool isInitialized;
    bool usingAVIO;  // true if opened via AVIO bridge
    
    int currentWidth;
    int currentHeight;
    double currentFps;
    double totalDuration;
    double currentPosition;
};

} // namespace video
} // namespace krkr

#endif // KRKRSDL2_FFMPEG_ENABLED
