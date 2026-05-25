/*
 * FFmpeg AVIO Bridge for KRKR Storage / XP3
 * Copyright (c) Kirikiri SDL2 Developers
 * SPDX-License-Identifier: MIT
 *
 * This module provides a custom AVIOContext that routes FFmpeg I/O
 * through KRKR's tTJSBinaryStream (which handles XP3 archives,
 * auto-path search, and all registered storage media).
 *
 * Usage:
 *   krkr::avio::FFmpegAVIOBridge bridge;
 *   AVFormatContext* ctx = nullptr;
 *   if (bridge.openFromKRKRStorage(storageName, &ctx)) {
 *       // ctx is ready for avformat_find_stream_info, etc.
 *       // bridge owns the AVIOContext lifetime
 *   }
 *   bridge.close(); // or destructor handles it
 */

#pragma once

#if KRKRSDL2_FFMPEG_ENABLED

#include <string>
#include <cstdint>

// Forward declarations – avoid pulling in heavy TJS/FFmpeg headers
namespace TJS { class tTJSBinaryStream; }
struct AVIOContext;
struct AVFormatContext;

namespace krkr {
namespace avio {

/**
 * FFmpeg AVIOContext bridge to KRKR Storage / XP3.
 *
 * Provides read_packet / seek callbacks that delegate to
 * tTJSBinaryStream::Read / Seek, allowing FFmpeg to demux media
 * files stored inside XP3 archives or any KRKR storage media.
 *
 * Thread safety: NOT thread-safe. One bridge per decoder instance.
 */
class FFmpegAVIOBridge {
public:
    FFmpegAVIOBridge();
    ~FFmpegAVIOBridge();

    // Non-copyable, non-movable (owns raw resources)
    FFmpegAVIOBridge(const FFmpegAVIOBridge&) = delete;
    FFmpegAVIOBridge& operator=(const FFmpegAVIOBridge&) = delete;

    /**
     * Open a KRKR storage path and set up AVIOContext + AVFormatContext.
     *
     * @param krkrStorageName  KRKR storage name (e.g. "bgm01.ogg",
     *                         "video/op.wmv"). Will be resolved through
     *                         TVPCreateStream with auto-path search.
     * @param outFmtCtx        Receives the allocated AVFormatContext with
     *                         the custom AVIO attached. Caller must NOT
     *                         call avformat_open_input on it again.
     * @return true if open + probe succeeded.
     *
     * On success, ownership of the AVFormatContext is shared:
     *   - The bridge owns the AVIOContext and underlying stream.
     *   - The caller should call close() or let the destructor handle cleanup.
     *   - The returned AVFormatContext is freed by close().
     */
    bool openFromKRKRStorage(const std::string& krkrStorageName,
                             AVFormatContext** outFmtCtx);

    /**
     * Close and release all resources (stream, AVIO buffer,
     * AVIOContext, AVFormatContext).
     *
     * Safe to call multiple times.
     */
    void close();

    /**
     * Check if this bridge currently has an open stream.
     */
    bool isOpen() const { return stream_ != nullptr; }

    /**
     * Get the total size of the underlying storage, or -1 if unknown.
     */
    int64_t getStreamSize() const;

    int getReadCallCount() const { return readCallCount_; }
    int getSeekCallCount() const { return seekCallCount_; }
    int getReadErrorCount() const { return readErrorCount_; }
    int getSeekErrorCount() const { return seekErrorCount_; }
    int getEofCount() const { return eofCount_; }
    int64_t getBytesRead() const { return bytesRead_; }

    /**
     * Log probe results: format name, streams, codec info, duration, etc.
     * Call after openFromKRKRStorage succeeds.
     */
    void logProbeInfo() const;

private:
    // ---- AVIO callback trampolines (C linkage) ----
    static int avioReadPacket(void* opaque, uint8_t* buf, int buf_size);
    static int64_t avioSeek(void* opaque, int64_t offset, int whence);

    // ---- Internal state ----
    TJS::tTJSBinaryStream* stream_;  // KRKR binary stream (owned)
    uint8_t*          avioBuffer_;   // AVIO internal buffer (owned by AVIOContext)
    AVIOContext*      avioCtx_;      // Custom AVIO context (owned)
    AVFormatContext*  fmtCtx_;       // Format context (owned)
    int64_t           streamSize_;   // Cached stream size
    std::string       storageName_;  // For diagnostics
    int               readCallCount_;
    int               seekCallCount_;
    int               readErrorCount_;
    int               seekErrorCount_;
    int               eofCount_;
    int64_t           bytesRead_;

    static constexpr int kAVIOBufferSize = 32768; // 32 KB AVIO buffer
};

} // namespace avio
} // namespace krkr

#endif // KRKRSDL2_FFMPEG_ENABLED
