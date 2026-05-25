/*
 * FFmpeg AVIO Bridge for KRKR Storage / XP3
 * Copyright (c) Kirikiri SDL2 Developers
 * SPDX-License-Identifier: MIT
 *
 * Implementation of the custom AVIOContext that routes FFmpeg I/O
 * through KRKR's tTJSBinaryStream.
 */

#include "FFmpegAVIO.h"

#if KRKRSDL2_FFMPEG_ENABLED

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/log.h>
#include <libavutil/pixdesc.h>
}

#include <cstring>
#include <cstdio>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// KRKR headers for storage access
// tjsCommHead.h sets up 'using namespace TJS;' which resolves tTJSBinaryStream.
#include "tjsCommHead.h"
#include "StorageIntf.h"
#include "LogFilter.h"

namespace krkr {
namespace avio {

// ---------------------------------------------------------------------------
// AVIO Callbacks
// ---------------------------------------------------------------------------

/**
 * Read callback for FFmpeg AVIOContext.
 * Delegates to tTJSBinaryStream::Read.
 *
 * @param opaque  Pointer to the FFmpegAVIOBridge instance.
 * @param buf     Buffer to read into.
 * @param buf_size Maximum bytes to read.
 * @return Number of bytes read, or AVERROR_EOF on end of stream,
 *         or AVERROR(EIO) on error.
 */
int FFmpegAVIOBridge::avioReadPacket(void* opaque, uint8_t* buf, int buf_size)
	{
	    FFmpegAVIOBridge* self = static_cast<FFmpegAVIOBridge*>(opaque);
	    if (!self || !self->stream_ || !buf || buf_size <= 0) {
	        return AVERROR(EIO);
	    }
#ifdef __EMSCRIPTEN__
	    double perfStartMs = TVPPerfEnabled() ? emscripten_get_now() : 0.0;
#endif

	    try {
	        self->readCallCount_++;
	        tjs_uint bytesRead = self->stream_->Read(buf, static_cast<tjs_uint>(buf_size));
	        if (bytesRead == 0) {
	            self->eofCount_++;
	            return AVERROR_EOF;
	        }
	        self->bytesRead_ += bytesRead;
#ifdef __EMSCRIPTEN__
	        if(TVPPerfEnabled()) {
	            double perfEndMs = emscripten_get_now();
	            if(perfEndMs - perfStartMs >= 4.0 || self->readCallCount_ <= 4) {
	                fprintf(stderr,
	                    "[PERF] ffmpeg.avio.read storage=%s call=%d request=%d read=%u total_bytes=%lld ms=%.3f\n",
	                    self->storageName_.c_str(),
	                    self->readCallCount_,
	                    buf_size,
	                    (unsigned)bytesRead,
	                    (long long)self->bytesRead_,
	                    perfEndMs - perfStartMs);
	            }
	        }
#endif
	        return static_cast<int>(bytesRead);
	    } catch (...) {
	        self->readErrorCount_++;
	        fprintf(stderr, "[FFmpeg-AVIO] Read error on '%s'\n",
	                self->storageName_.c_str());
	        return AVERROR(EIO);
	    }
	}

/**
 * Seek callback for FFmpeg AVIOContext.
 * Delegates to tTJSBinaryStream::Seek / GetSize.
 *
 * @param opaque  Pointer to the FFmpegAVIOBridge instance.
 * @param offset  Seek offset.
 * @param whence  SEEK_SET, SEEK_CUR, SEEK_END, or AVSEEK_SIZE.
 * @return New position after seek, or stream size for AVSEEK_SIZE,
 *         or AVERROR(EIO) on error.
 */
int64_t FFmpegAVIOBridge::avioSeek(void* opaque, int64_t offset, int whence)
{
    FFmpegAVIOBridge* self = static_cast<FFmpegAVIOBridge*>(opaque);
    if (!self || !self->stream_) {
        return AVERROR(EIO);
    }

	    try {
	        // Handle AVSEEK_SIZE: return the total stream size
	        if (whence == AVSEEK_SIZE) {
	            return self->streamSize_;
	        }
	        self->seekCallCount_++;

	        // Map FFmpeg whence to TJS whence
	        tjs_int tjsWhence;
	        switch (whence & ~AVSEEK_FORCE) {
            case SEEK_SET:
                tjsWhence = TJS_BS_SEEK_SET;
                break;
            case SEEK_CUR:
                tjsWhence = TJS_BS_SEEK_CUR;
                break;
            case SEEK_END:
                tjsWhence = TJS_BS_SEEK_END;
                break;
	            default:
	                self->seekErrorCount_++;
	                fprintf(stderr, "[FFmpeg-AVIO] Unknown whence %d on '%s'\n",
	                        whence, self->storageName_.c_str());
	                return AVERROR(EINVAL);
	        }

        tjs_uint64 newPos = self->stream_->Seek(
            static_cast<tjs_int64>(offset), tjsWhence);
	        return static_cast<int64_t>(newPos);
	    } catch (...) {
	        self->seekErrorCount_++;
	        fprintf(stderr, "[FFmpeg-AVIO] Seek error on '%s' (offset=%lld, whence=%d)\n",
	                self->storageName_.c_str(), (long long)offset, whence);
	        return AVERROR(EIO);
	    }
	}

// ---------------------------------------------------------------------------
// FFmpegAVIOBridge Implementation
// ---------------------------------------------------------------------------

FFmpegAVIOBridge::FFmpegAVIOBridge()
    : stream_(nullptr)
    , avioBuffer_(nullptr)
	    , avioCtx_(nullptr)
	    , fmtCtx_(nullptr)
	    , streamSize_(-1)
	    , readCallCount_(0)
	    , seekCallCount_(0)
	    , readErrorCount_(0)
	    , seekErrorCount_(0)
	    , eofCount_(0)
	    , bytesRead_(0)
	{
	}

FFmpegAVIOBridge::~FFmpegAVIOBridge()
{
    close();
}

bool FFmpegAVIOBridge::openFromKRKRStorage(const std::string& krkrStorageName,
                                           AVFormatContext** outFmtCtx)
{
#ifdef __EMSCRIPTEN__
    double perfStartMs = TVPPerfEnabled() ? emscripten_get_now() : 0.0;
    double afterCreateStreamMs = perfStartMs;
    double afterGetSizeMs = perfStartMs;
    double afterAvioAllocMs = perfStartMs;
    double afterFmtAllocMs = perfStartMs;
    double afterOpenInputMs = perfStartMs;
    double afterFindInfoMs = perfStartMs;
#endif
    if (stream_) {
        close();
    }

	    storageName_ = krkrStorageName;
	    readCallCount_ = 0;
	    seekCallCount_ = 0;
	    readErrorCount_ = 0;
	    seekErrorCount_ = 0;
	    eofCount_ = 0;
	    bytesRead_ = 0;

	    if (outFmtCtx) {
	        *outFmtCtx = nullptr;
    }

    // ---- Step 1: Open KRKR storage stream ----
    fprintf(stderr, "[FFmpeg-AVIO] Opening KRKR storage: '%s'\n",
            krkrStorageName.c_str());

    try {
        // Convert std::string to ttstr for KRKR API
        // TVPCreateStream handles auto-path search, XP3 archive lookup, etc.
        ttstr krkrName(krkrStorageName.c_str());
        stream_ = TVPCreateStream(krkrName, TJS_BS_READ);
#ifdef __EMSCRIPTEN__
        if(TVPPerfEnabled()) afterCreateStreamMs = emscripten_get_now();
#endif
    } catch (...) {
        fprintf(stderr, "[FFmpeg-AVIO] Failed to open KRKR storage: '%s' (exception)\n",
                krkrStorageName.c_str());
        stream_ = nullptr;
        return false;
    }

    if (!stream_) {
        fprintf(stderr, "[FFmpeg-AVIO] Failed to open KRKR storage: '%s' (null stream)\n",
                krkrStorageName.c_str());
        return false;
    }

    // Cache stream size
    try {
        streamSize_ = static_cast<int64_t>(stream_->GetSize());
    } catch (...) {
        streamSize_ = -1;
    }
#ifdef __EMSCRIPTEN__
    if(TVPPerfEnabled()) afterGetSizeMs = emscripten_get_now();
#endif

    fprintf(stderr, "[FFmpeg-AVIO] KRKR stream opened: '%s', size=%lld bytes\n",
            krkrStorageName.c_str(), (long long)streamSize_);

    // ---- Step 2: Allocate AVIO buffer ----
    avioBuffer_ = static_cast<uint8_t*>(av_malloc(kAVIOBufferSize));
    if (!avioBuffer_) {
        fprintf(stderr, "[FFmpeg-AVIO] Failed to allocate AVIO buffer\n");
        close();
        return false;
    }

    // ---- Step 3: Create custom AVIOContext ----
    avioCtx_ = avio_alloc_context(
        avioBuffer_,          // buffer
        kAVIOBufferSize,      // buffer size
        0,                    // write_flag = 0 (read-only)
        this,                 // opaque = this bridge
        &avioReadPacket,      // read_packet callback
        nullptr,              // write_packet = nullptr (read-only)
        &avioSeek             // seek callback
    );

    if (!avioCtx_) {
        fprintf(stderr, "[FFmpeg-AVIO] Failed to allocate AVIOContext\n");
        // Note: avioBuffer_ is freed by avio_alloc_context on failure
        // but we need to handle it carefully
        av_free(avioBuffer_);
        avioBuffer_ = nullptr;
        close();
        return false;
    }
#ifdef __EMSCRIPTEN__
    if(TVPPerfEnabled()) afterAvioAllocMs = emscripten_get_now();
#endif

    // ---- Step 4: Create AVFormatContext with custom AVIO ----
    fmtCtx_ = avformat_alloc_context();
    if (!fmtCtx_) {
        fprintf(stderr, "[FFmpeg-AVIO] Failed to allocate AVFormatContext\n");
        close();
        return false;
    }
#ifdef __EMSCRIPTEN__
    if(TVPPerfEnabled()) afterFmtAllocMs = emscripten_get_now();
#endif

	    // Attach our custom AVIO to the format context
	    fmtCtx_->pb = avioCtx_;
	    fmtCtx_->flags |= AVFMT_FLAG_CUSTOM_IO;

	    // ---- Step 5: Open input using the custom AVIO ----
	    // Data comes from our AVIO. The storage name is kept as a URL hint
	    // so extension-based demuxer probing still sees the original suffix.
	    int ret = avformat_open_input(&fmtCtx_, krkrStorageName.c_str(),
	                                  nullptr, nullptr);
    if (ret < 0) {
        char errBuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errBuf, sizeof(errBuf));
        fprintf(stderr, "[FFmpeg-AVIO] avformat_open_input failed for '%s': %s\n",
                krkrStorageName.c_str(), errBuf);
        // avformat_open_input frees fmtCtx_ on failure, so null it out
        fmtCtx_ = nullptr;
        close();
        return false;
    }
#ifdef __EMSCRIPTEN__
    if(TVPPerfEnabled()) afterOpenInputMs = emscripten_get_now();
#endif

    // ---- Step 6: Find stream info (probing) ----
    ret = avformat_find_stream_info(fmtCtx_, nullptr);
    if (ret < 0) {
        char errBuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errBuf, sizeof(errBuf));
        fprintf(stderr, "[FFmpeg-AVIO] avformat_find_stream_info failed for '%s': %s\n",
                krkrStorageName.c_str(), errBuf);
        // Don't close fmtCtx_ here; we still want to try to use it
        // Some formats may work with partial info
        // But for safety, we'll report this as a failure
        close();
        return false;
    }
#ifdef __EMSCRIPTEN__
    if(TVPPerfEnabled()) afterFindInfoMs = emscripten_get_now();
#endif

    // ---- Success ----
    if (outFmtCtx) {
        *outFmtCtx = fmtCtx_;
    }

    fprintf(stderr, "[FFmpeg-AVIO] Successfully opened and probed: '%s'\n",
            krkrStorageName.c_str());
    logProbeInfo();
#ifdef __EMSCRIPTEN__
    if(TVPPerfEnabled()) {
        double perfEndMs = emscripten_get_now();
        fprintf(stderr,
            "[PERF] ffmpeg.avio.open storage=%s total_ms=%.3f create_stream_ms=%.3f size_ms=%.3f avio_alloc_ms=%.3f fmt_alloc_ms=%.3f open_input_ms=%.3f find_info_ms=%.3f finish_ms=%.3f stream_size=%lld read_calls=%d seek_calls=%d bytes_read=%lld\n",
            krkrStorageName.c_str(),
            perfEndMs - perfStartMs,
            afterCreateStreamMs - perfStartMs,
            afterGetSizeMs - afterCreateStreamMs,
            afterAvioAllocMs - afterGetSizeMs,
            afterFmtAllocMs - afterAvioAllocMs,
            afterOpenInputMs - afterFmtAllocMs,
            afterFindInfoMs - afterOpenInputMs,
            perfEndMs - afterFindInfoMs,
            (long long)streamSize_,
            readCallCount_,
            seekCallCount_,
            (long long)bytesRead_);
    }
#endif

    return true;
}

void FFmpegAVIOBridge::close()
{
    // Order matters: format context references AVIO context

    if (fmtCtx_) {
        // Detach our AVIO before closing format context
        // avformat_close_input will free the format context
        // but we need to handle AVIO cleanup ourselves
        avformat_close_input(&fmtCtx_);
        fmtCtx_ = nullptr;
    }

    if (avioCtx_) {
        // The buffer inside AVIOContext is managed by FFmpeg after
        // avio_alloc_context succeeds. We need to free the context.
        // Note: the internal buffer may have been reallocated by FFmpeg,
        // so we should NOT free avioBuffer_ separately.
        av_freep(&avioCtx_->buffer);
        avio_context_free(&avioCtx_);
        avioCtx_ = nullptr;
        avioBuffer_ = nullptr;  // was freed with the context
    } else if (avioBuffer_) {
        av_free(avioBuffer_);
        avioBuffer_ = nullptr;
    }

    if (stream_) {
        try {
            delete stream_;
        } catch (...) {
            fprintf(stderr, "[FFmpeg-AVIO] Exception during stream cleanup\n");
        }
        stream_ = nullptr;
    }

	    streamSize_ = -1;
	    storageName_.clear();
	    readCallCount_ = 0;
	    seekCallCount_ = 0;
	    readErrorCount_ = 0;
	    seekErrorCount_ = 0;
	    eofCount_ = 0;
	    bytesRead_ = 0;
	}

int64_t FFmpegAVIOBridge::getStreamSize() const
{
    return streamSize_;
}

void FFmpegAVIOBridge::logProbeInfo() const
{
    if (!fmtCtx_) {
        fprintf(stderr, "[FFmpeg-AVIO] No format context available for probe info\n");
        return;
    }

    fprintf(stderr, "[FFmpeg-AVIO] === Probe Results for '%s' ===\n",
            storageName_.c_str());

    // Format info
    fprintf(stderr, "[FFmpeg-AVIO]   Format: %s (%s)\n",
            fmtCtx_->iformat ? fmtCtx_->iformat->name : "unknown",
            fmtCtx_->iformat ? fmtCtx_->iformat->long_name : "unknown");

    // Duration
    if (fmtCtx_->duration != AV_NOPTS_VALUE) {
        double durationSec = fmtCtx_->duration / (double)AV_TIME_BASE;
        fprintf(stderr, "[FFmpeg-AVIO]   Duration: %.3f seconds\n", durationSec);
    } else {
        fprintf(stderr, "[FFmpeg-AVIO]   Duration: unknown\n");
    }

    // Bitrate
    if (fmtCtx_->bit_rate > 0) {
        fprintf(stderr, "[FFmpeg-AVIO]   Bitrate: %lld bps\n",
                (long long)fmtCtx_->bit_rate);
    }

    // Streams
    fprintf(stderr, "[FFmpeg-AVIO]   Streams: %u\n", fmtCtx_->nb_streams);

    for (unsigned int i = 0; i < fmtCtx_->nb_streams; i++) {
        AVStream* stream = fmtCtx_->streams[i];
        AVCodecParameters* par = stream->codecpar;

        const char* typeName = av_get_media_type_string(par->codec_type);
        const AVCodec* codec = avcodec_find_decoder(par->codec_id);
        const char* codecName = codec ? codec->name : "unknown";

        fprintf(stderr, "[FFmpeg-AVIO]   Stream #%u: type=%s codec=%s",
                i, typeName ? typeName : "unknown", codecName);

        if (par->codec_type == AVMEDIA_TYPE_AUDIO) {
            fprintf(stderr, " sample_rate=%d channels=%d",
                    par->sample_rate, par->ch_layout.nb_channels);
            if (par->bit_rate > 0) {
                fprintf(stderr, " bitrate=%lld", (long long)par->bit_rate);
            }
        } else if (par->codec_type == AVMEDIA_TYPE_VIDEO) {
            fprintf(stderr, " %dx%d", par->width, par->height);
            if (stream->avg_frame_rate.den != 0) {
                fprintf(stderr, " fps=%.2f",
                        av_q2d(stream->avg_frame_rate));
            }
            const char* pixFmtName = av_get_pix_fmt_name(
                static_cast<AVPixelFormat>(par->format));
            if (pixFmtName) {
                fprintf(stderr, " pix_fmt=%s", pixFmtName);
            }
        }

        fprintf(stderr, "\n");
    }

    fprintf(stderr, "[FFmpeg-AVIO] === End Probe Results ===\n");
}

} // namespace avio
} // namespace krkr

#endif // KRKRSDL2_FFMPEG_ENABLED
