/*
 * FFmpeg Video Player for KRKRSDL2 (Emscripten)
 * Copyright (c) Kirikiri SDL2 Developers
 * SPDX-License-Identifier: MIT
 */

#include "FFmpegVideoPlayer.h"

#if KRKRSDL2_FFMPEG_ENABLED

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}

#include <cstring>
#include <cstdio>
#include <algorithm>
#include "ThreadIntf.h"
#include "LogFilter.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace krkr {
namespace video {

class FFmpegVideoPlayer::DecodeThread : public tTVPThread {
    FFmpegVideoPlayer* owner_;
public:
    explicit DecodeThread(FFmpegVideoPlayer* owner) : owner_(owner)
    {
        SetPriority(ttpHigher);
        StartTread();
    }
protected:
    void Execute() override
    {
        if (owner_) owner_->decodeThreadMain();
    }
};

static bool krkr_ffmpeg_worker_enabled()
{
#if defined(__EMSCRIPTEN__) && defined(__EMSCRIPTEN_PTHREADS__)
    static int enabled = -1;
    if (enabled < 0) {
        enabled = MAIN_THREAD_EM_ASM_INT({
            try {
                var params = new URLSearchParams(location.search || "");
                var v = params.get("krkr_ffmpeg_worker");
                if (v === "0" || v === "false" || v === "off" || v === "no") return 0;
                if (typeof Module !== "undefined") {
                    var mv = Module["krkr_ffmpeg_worker"] || Module["krkrFFmpegWorker"];
                    if (mv === 0 || mv === false || mv === "0" || mv === "false" || mv === "off" || mv === "no") return 0;
                }
            } catch (e) {}
            return 1;
        });
    }
    return enabled != 0;
#else
    return false;
#endif
}

FFmpegVideoPlayer::FFmpegVideoPlayer()
    : fmtCtx_(nullptr)
    , audioFmtCtx_(nullptr)
    , codecCtx_(nullptr)
    , audioCodecCtx_(nullptr)
    , swsCtx_(nullptr)
    , swrCtx_(nullptr)
    , packet_(nullptr)
    , audioPacket_(nullptr)
    , frame_(nullptr)
    , audioFrame_(nullptr)
    , frameBGRA_(nullptr)
    , bgraBuffer_(nullptr)
    , videoStreamIdx_(-1)
    , audioStreamIdx_(-1)
    , audioStreamCount_(0)
    , requestedAudioStreamOrdinal_(0)
    , audioBuffer_(nullptr)
    , audioEnabled_(true)
    , audioDecoderDraining_(false)
    , audioEof_(false)
    , audioSampleRate_(44100)
    , audioChannels_(2)
    , audioBytesPerFrame_(4)
    , audioQueuedSamples_(0)
    , audioPlayedSamples_(0)
    , audioVolume_(100000)
    , audioBalance_(0)
    , presentedStride_(0)
    , decodeThread_(nullptr)
    , decodeWorkerEnabled_(false)
    , decodeTerminate_(false)
    , decodePlayRequested_(false)
    , decodeSeekPending_(false)
    , decodeSeekMs_(0)
    , decodeGeneration_(1)
    , decodeVideoEof_(false)
    , decodeAudioEofWorker_(false)
    , decodeFallbackFrame_(-1)
    , decodeLastPerfLogMs_(0.0)
    , syncLastPerfLogMs_(0.0)
    , state_(PlayerState::Closed)
    , isOpen_(false)
    , isPlaying_(false)
    , isPaused_(false)
    , isComplete_(false)
    , width_(0)
    , height_(0)
    , fps_(30.0)
    , totalFrames_(0)
    , totalTimeMs_(0)
    , currentFrame_(0)
    , positionMs_(0)
    , frameAccumMs_(0)
    , msPerFrame_(33.33)
{
}

FFmpegVideoPlayer::~FFmpegVideoPlayer()
{
    close();
}

const char* FFmpegVideoPlayer::stateName() const
{
    switch (state_) {
    case PlayerState::Closed: return "Closed";
    case PlayerState::Opening: return "Opening";
    case PlayerState::Ready: return "Ready";
    case PlayerState::Playing: return "Playing";
    case PlayerState::Paused: return "Paused";
    case PlayerState::Seeking: return "Seeking";
    case PlayerState::Flushing: return "Flushing";
    case PlayerState::Ended: return "Ended";
    case PlayerState::Error: return "Error";
    default: return "unknown";
    }
}

const char* FFmpegVideoPlayer::commandName(ControlCommand cmd) const
{
    switch (cmd) {
    case ControlCommand::Open: return "Open";
    case ControlCommand::Close: return "Close";
    case ControlCommand::Play: return "Play";
    case ControlCommand::Pause: return "Pause";
    case ControlCommand::Stop: return "Stop";
    case ControlCommand::Seek: return "Seek";
    case ControlCommand::Flush: return "Flush";
    case ControlCommand::SetLoop: return "SetLoop";
    case ControlCommand::SetVolume: return "SetVolume";
    case ControlCommand::SetRect: return "SetRect";
    case ControlCommand::Eof: return "EOF";
    case ControlCommand::Error: return "Error";
    default: return "unknown";
    }
}

void FFmpegVideoPlayer::setState(PlayerState state, const char* reason)
{
    if (state_ == state && !TVPPerfEnabled()) return;
    state_ = state;
    if (!TVPPerfEnabled()) return;
    size_t videoQueued = 0;
    int audioQueued = 0;
    uint64_t generation = decodeGeneration_;
    {
        std::lock_guard<std::mutex> lock(decodeMutex_);
        videoQueued = decodedVideoFrames_.size();
        audioQueued = queuedWorkerAudioSamplesLocked();
        generation = decodeGeneration_;
    }
    fprintf(stderr,
        "[PERF] av.state component=movie phase=%s storage=%s state=%s generation=%llu open=%d playing=%d paused=%d complete=%d video_q=%zu audio_q=%d frame=%d position_ms=%lld\n",
        reason ? reason : "state",
        storageName_.c_str(),
        stateName(),
        (unsigned long long)generation,
        isOpen_ ? 1 : 0,
        isPlaying_ ? 1 : 0,
        isPaused_ ? 1 : 0,
        isComplete_ ? 1 : 0,
        videoQueued,
        audioQueued,
        currentFrame_,
        (long long)positionMs_);
}

void FFmpegVideoPlayer::logControl(ControlCommand cmd, const char* phase, int64_t arg) const
{
    if (!TVPPerfEnabled()) return;
    size_t videoQueued = 0;
    int audioQueued = 0;
    uint64_t generation = decodeGeneration_;
    bool playRequested = false;
    bool seekPending = false;
    bool videoEof = false;
    bool audioEof = false;
    {
        std::lock_guard<std::mutex> lock(decodeMutex_);
        videoQueued = decodedVideoFrames_.size();
        audioQueued = queuedWorkerAudioSamplesLocked();
        generation = decodeGeneration_;
        playRequested = decodePlayRequested_;
        seekPending = decodeSeekPending_;
        videoEof = decodeVideoEof_;
        audioEof = decodeAudioEofWorker_;
    }
    fprintf(stderr,
        "[PERF] movie.queue cmd=%s phase=%s storage=%s state=%s generation=%llu arg=%lld play=%d seek=%d video_q=%zu audio_q=%d video_eof=%d audio_eof=%d frame=%d position_ms=%lld\n",
        commandName(cmd),
        phase ? phase : "event",
        storageName_.c_str(),
        stateName(),
        (unsigned long long)generation,
        (long long)arg,
        playRequested ? 1 : 0,
        seekPending ? 1 : 0,
        videoQueued,
        audioQueued,
        videoEof ? 1 : 0,
        audioEof ? 1 : 0,
        currentFrame_,
        (long long)positionMs_);
}

void FFmpegVideoPlayer::logSync(const char* phase)
{
    if (!TVPPerfEnabled()) return;
#ifdef __EMSCRIPTEN__
    double now = emscripten_get_now();
#else
    double now = 0.0;
#endif
    bool important = phase &&
        (std::strcmp(phase, "play") == 0 ||
         std::strcmp(phase, "pause") == 0 ||
         std::strcmp(phase, "eof") == 0 ||
         std::strcmp(phase, "audio-open") == 0 ||
         std::strcmp(phase, "audio-stream-switch") == 0 ||
         std::strcmp(phase, "audio-disable") == 0);
    if (!important && syncLastPerfLogMs_ > 0.0 && now - syncLastPerfLogMs_ < 500.0)
        return;
    syncLastPerfLogMs_ = now;
    size_t videoQueued = 0;
    int workerAudioQueued = 0;
    uint64_t generation = decodeGeneration_;
    {
        std::lock_guard<std::mutex> lock(decodeMutex_);
        videoQueued = decodedVideoFrames_.size();
        workerAudioQueued = queuedWorkerAudioSamplesLocked();
        generation = decodeGeneration_;
    }
    int64_t aheadSamples = audioQueuedSamples_ - audioPlayedSamples_;
    fprintf(stderr,
        "[PERF] movie.sync phase=%s storage=%s state=%s generation=%llu frame=%d position_ms=%lld video_q=%zu worker_audio_q=%d audio_queued_samples=%lld audio_played_samples=%lld audio_ahead_samples=%lld sink_id=%d backend=%s sink_state=%s sink_generation=%llu ring_fill_bytes=%d underrun=%d\n",
        phase ? phase : "sync",
        storageName_.c_str(),
        stateName(),
        (unsigned long long)generation,
        currentFrame_,
        (long long)positionMs_,
        videoQueued,
        workerAudioQueued,
        (long long)audioQueuedSamples_,
        (long long)audioPlayedSamples_,
        (long long)aheadSamples,
        audioBuffer_ ? audioBuffer_->GetDebugSinkId() : 0,
        audioBuffer_ ? audioBuffer_->GetDebugBackendName() : "none",
        audioBuffer_ ? audioBuffer_->GetDebugStateName() : "none",
        audioBuffer_ ? (unsigned long long)audioBuffer_->GetDebugGeneration() : 0,
        audioBuffer_ ? audioBuffer_->GetDebugRingFillBytes() : -1,
        audioBuffer_ ? audioBuffer_->GetDebugUnderrunCount() : -1);
}

bool FFmpegVideoPlayer::open(const std::string& storageName)
{
    if (isOpen_) close();

    storageName_ = storageName;
    audioStreamCount_ = 0;
    requestedAudioStreamOrdinal_ = 0;
    setState(PlayerState::Opening, "open-begin");
    logControl(ControlCommand::Open, "begin");
    fprintf(stderr, "[FFmpeg-VideoPlayer] Opening: '%s'\n", storageName.c_str());

    // Open via AVIO bridge
    if (!avioBridge_.openFromKRKRStorage(storageName, &fmtCtx_)) {
        fprintf(stderr, "[FFmpeg-VideoPlayer] AVIO open failed for '%s'\n",
                storageName.c_str());
        logControl(ControlCommand::Error, "avio-open-failed");
        setState(PlayerState::Error, "open-error");
        return false;
    }

    // Find video stream
    videoStreamIdx_ = av_find_best_stream(fmtCtx_, AVMEDIA_TYPE_VIDEO,
                                           -1, -1, nullptr, 0);
    if (videoStreamIdx_ < 0) {
        fprintf(stderr, "[FFmpeg-VideoPlayer] No video stream in '%s'\n",
                storageName.c_str());
        logControl(ControlCommand::Error, "no-video-stream");
        setState(PlayerState::Error, "open-error");
        close();
        return false;
    }

    AVStream* stream = fmtCtx_->streams[videoStreamIdx_];

    // Find and open codec
    const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec) {
        fprintf(stderr, "[FFmpeg-VideoPlayer] Unsupported codec for '%s'\n",
                storageName.c_str());
        logControl(ControlCommand::Error, "unsupported-codec");
        setState(PlayerState::Error, "open-error");
        close();
        return false;
    }

    codecCtx_ = avcodec_alloc_context3(codec);
    if (!codecCtx_) {
        logControl(ControlCommand::Error, "codec-alloc-failed");
        setState(PlayerState::Error, "open-error");
        close();
        return false;
    }

    int ret = avcodec_parameters_to_context(codecCtx_, stream->codecpar);
    if (ret < 0) {
        logControl(ControlCommand::Error, "codec-params-failed");
        setState(PlayerState::Error, "open-error");
        close();
        return false;
    }

    ret = avcodec_open2(codecCtx_, codec, nullptr);
    if (ret < 0) {
        char errBuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errBuf, sizeof(errBuf));
        fprintf(stderr, "[FFmpeg-VideoPlayer] Codec open failed: %s\n", errBuf);
        logControl(ControlCommand::Error, "codec-open-failed");
        setState(PlayerState::Error, "open-error");
        close();
        return false;
    }

    // Store video info
    width_ = codecCtx_->width;
    height_ = codecCtx_->height;

    // FPS
    if (stream->avg_frame_rate.den != 0 && stream->avg_frame_rate.num != 0) {
        fps_ = av_q2d(stream->avg_frame_rate);
    } else if (stream->r_frame_rate.den != 0 && stream->r_frame_rate.num != 0) {
        fps_ = av_q2d(stream->r_frame_rate);
    } else {
        fps_ = 30.0;
    }
    msPerFrame_ = 1000.0 / fps_;

    // Duration
    if (stream->duration != AV_NOPTS_VALUE) {
        double durationSec = stream->duration * av_q2d(stream->time_base);
        totalTimeMs_ = (int64_t)(durationSec * 1000.0);
        totalFrames_ = (int)(durationSec * fps_);
    } else if (fmtCtx_->duration != AV_NOPTS_VALUE) {
        double durationSec = fmtCtx_->duration / (double)AV_TIME_BASE;
        totalTimeMs_ = (int64_t)(durationSec * 1000.0);
        totalFrames_ = (int)(durationSec * fps_);
    } else {
        totalTimeMs_ = 0;
        totalFrames_ = 0;
    }

    // KRKR 32bpp bitmaps use ARGB integer semantics; on little-endian
    // memory this is byte-order BGRA.
    swsCtx_ = sws_getContext(
        width_, height_, codecCtx_->pix_fmt,
        width_, height_, AV_PIX_FMT_BGRA,
        SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!swsCtx_) {
        fprintf(stderr, "[FFmpeg-VideoPlayer] Failed to create scaler\n");
        logControl(ControlCommand::Error, "scaler-create-failed");
        setState(PlayerState::Error, "open-error");
        close();
        return false;
    }

    // Allocate frames
    frame_ = av_frame_alloc();
    frameBGRA_ = av_frame_alloc();
    if (!frame_ || !frameBGRA_) {
        logControl(ControlCommand::Error, "frame-alloc-failed");
        setState(PlayerState::Error, "open-error");
        close();
        return false;
    }

    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_BGRA, width_, height_, 1);
    bgraBuffer_ = (uint8_t*)av_malloc(numBytes);
    if (!bgraBuffer_) {
        logControl(ControlCommand::Error, "bgra-alloc-failed");
        setState(PlayerState::Error, "open-error");
        close();
        return false;
    }
    av_image_fill_arrays(frameBGRA_->data, frameBGRA_->linesize, bgraBuffer_,
                          AV_PIX_FMT_BGRA, width_, height_, 1);

    for (unsigned int i = 0; i < fmtCtx_->nb_streams; ++i) {
        if (fmtCtx_->streams[i] &&
            fmtCtx_->streams[i]->codecpar &&
            fmtCtx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            ++audioStreamCount_;
        }
    }

    // Allocate packet
    packet_ = av_packet_alloc();
    if (!packet_) {
        logControl(ControlCommand::Error, "packet-alloc-failed");
        setState(PlayerState::Error, "open-error");
        close();
        return false;
    }

    openAudioStream();

    isOpen_ = true;
    isPlaying_ = false;
    isPaused_ = false;
    isComplete_ = false;
    currentFrame_ = 0;
    positionMs_ = 0;
    frameAccumMs_ = 0;
    startDecodeWorker();
    setState(PlayerState::Ready, "open-ready");
    logControl(ControlCommand::Open, "end");

    fprintf(stderr, "[FFmpeg-VideoPlayer] Opened OK: '%s' %dx%d fps=%.2f "
            "totalFrames=%d totalTimeMs=%lld worker=%d\n",
            storageName.c_str(), width_, height_, fps_,
            totalFrames_, (long long)totalTimeMs_,
            decodeWorkerEnabled_ ? 1 : 0);

    return true;
}

bool FFmpegVideoPlayer::openAudioStream()
{
    logControl(ControlCommand::Open, "audio-begin");
    audioStreamIdx_ = -1;
    audioEnabled_ = true;
    audioDecoderDraining_ = false;
    audioEof_ = false;
    audioQueuedSamples_ = 0;
    audioPlayedSamples_ = 0;

    audioFmtCtx_ = nullptr;
    if (!audioAvioBridge_.openFromKRKRStorage(storageName_, &audioFmtCtx_)) {
        fprintf(stderr, "[FFmpeg-VideoPlayer] Audio AVIO open failed for '%s'\n",
                storageName_.c_str());
        logControl(ControlCommand::Error, "audio-avio-open-failed");
        return false;
    }

    if (audioStreamCount_ <= 0) {
        for (unsigned int i = 0; i < audioFmtCtx_->nb_streams; ++i) {
            if (audioFmtCtx_->streams[i] &&
                audioFmtCtx_->streams[i]->codecpar &&
                audioFmtCtx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                ++audioStreamCount_;
            }
        }
    }

    int ordinal = 0;
    for (unsigned int i = 0; i < audioFmtCtx_->nb_streams; ++i) {
        if (audioFmtCtx_->streams[i] &&
            audioFmtCtx_->streams[i]->codecpar &&
            audioFmtCtx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (ordinal == requestedAudioStreamOrdinal_) {
                audioStreamIdx_ = (int)i;
                break;
            }
            ++ordinal;
        }
    }
    if (audioStreamIdx_ < 0) {
        audioStreamIdx_ = av_find_best_stream(audioFmtCtx_, AVMEDIA_TYPE_AUDIO,
                                              -1, -1, nullptr, 0);
    }
    if (audioStreamIdx_ < 0) {
        fprintf(stderr, "[FFmpeg-VideoPlayer] No audio stream in '%s'\n",
                storageName_.c_str());
        audioAvioBridge_.close();
        audioFmtCtx_ = nullptr;
        logControl(ControlCommand::Open, "audio-missing");
        return false;
    }

    AVStream* stream = audioFmtCtx_->streams[audioStreamIdx_];
    const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec) {
        fprintf(stderr,
                "[FFmpeg-VideoPlayer] Unsupported audio codec id=%d for '%s'\n",
                stream->codecpar->codec_id, storageName_.c_str());
        audioAvioBridge_.close();
        audioFmtCtx_ = nullptr;
        audioStreamIdx_ = -1;
        logControl(ControlCommand::Error, "audio-unsupported-codec");
        return false;
    }

    audioCodecCtx_ = avcodec_alloc_context3(codec);
    if (!audioCodecCtx_) {
        audioAvioBridge_.close();
        audioFmtCtx_ = nullptr;
        audioStreamIdx_ = -1;
        return false;
    }

    int ret = avcodec_parameters_to_context(audioCodecCtx_, stream->codecpar);
    if (ret < 0) {
        closeAudio();
        return false;
    }

    ret = avcodec_open2(audioCodecCtx_, codec, nullptr);
    if (ret < 0) {
        char errBuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errBuf, sizeof(errBuf));
        fprintf(stderr, "[FFmpeg-VideoPlayer] Audio codec open failed: %s\n",
                errBuf);
        closeAudio();
        return false;
    }

    audioChannels_ = 2;
    audioSampleRate_ = audioCodecCtx_->sample_rate > 0
        ? audioCodecCtx_->sample_rate
        : 44100;
    audioBytesPerFrame_ = audioChannels_ * 2;

    swrCtx_ = swr_alloc();
    if (!swrCtx_) {
        closeAudio();
        return false;
    }

    AVChannelLayout inLayout = audioCodecCtx_->ch_layout;
    if (inLayout.nb_channels <= 0) {
        av_channel_layout_default(&inLayout,
            audioCodecCtx_->ch_layout.nb_channels > 0
                ? audioCodecCtx_->ch_layout.nb_channels
                : 2);
    }
    AVChannelLayout outLayout = AV_CHANNEL_LAYOUT_STEREO;
    av_opt_set_chlayout(swrCtx_, "in_chlayout", &inLayout, 0);
    av_opt_set_int(swrCtx_, "in_sample_rate", audioCodecCtx_->sample_rate, 0);
    av_opt_set_sample_fmt(swrCtx_, "in_sample_fmt", audioCodecCtx_->sample_fmt, 0);
    av_opt_set_chlayout(swrCtx_, "out_chlayout", &outLayout, 0);
    av_opt_set_int(swrCtx_, "out_sample_rate", audioSampleRate_, 0);
    av_opt_set_sample_fmt(swrCtx_, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

    ret = swr_init(swrCtx_);
    if (ret < 0) {
        char errBuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errBuf, sizeof(errBuf));
        fprintf(stderr, "[FFmpeg-VideoPlayer] Audio resampler init failed: %s\n",
                errBuf);
        closeAudio();
        return false;
    }

    audioPacket_ = av_packet_alloc();
    audioFrame_ = av_frame_alloc();
    if (!audioPacket_ || !audioFrame_) {
        closeAudio();
        return false;
    }

    tTVPWaveFormat fmt;
    std::memset(&fmt, 0, sizeof(fmt));
    fmt.SamplesPerSec = audioSampleRate_;
    fmt.Channels = audioChannels_;
    fmt.BitsPerSample = 16;
    fmt.BytesPerSample = 2;
    fmt.TotalSamples = totalTimeMs_ > 0
        ? (tjs_uint64)((totalTimeMs_ * (int64_t)audioSampleRate_) / 1000)
        : 0;
    fmt.TotalTime = totalTimeMs_ > 0 ? (tjs_uint64)totalTimeMs_ : 0;
    fmt.IsFloat = false;
    fmt.Seekable = true;
    TVPInitDirectSound();
    audioBuffer_ = TVPCreateSoundBuffer(fmt, 16);
    if (!audioBuffer_) {
        fprintf(stderr, "[FFmpeg-VideoPlayer] Audio output buffer creation failed\n");
        closeAudio();
        logControl(ControlCommand::Error, "audio-buffer-create-failed");
        return false;
    }

    audioPcmBuffer_.resize(audioSampleRate_ * audioBytesPerFrame_ / 10);
    applyAudioSettings();

    fprintf(stderr,
            "[FFmpeg-VideoPlayer] Audio opened: stream=%d codec=%s rate=%d ch=%d\n",
            audioStreamIdx_, codec->name ? codec->name : "unknown",
            audioSampleRate_, audioChannels_);
    logSync("audio-open");
    logControl(ControlCommand::Open, "audio-end");
    return true;
}

void FFmpegVideoPlayer::closeAudio()
{
    logControl(ControlCommand::Close, "audio-begin");
    if (audioBuffer_) {
        audioBuffer_->Stop();
        audioBuffer_->Release();
        audioBuffer_ = nullptr;
    }
    if (audioPacket_) { av_packet_free(&audioPacket_); audioPacket_ = nullptr; }
    if (audioFrame_) { av_frame_free(&audioFrame_); audioFrame_ = nullptr; }
    if (swrCtx_) { swr_free(&swrCtx_); swrCtx_ = nullptr; }
    if (audioCodecCtx_) {
        avcodec_free_context(&audioCodecCtx_);
        audioCodecCtx_ = nullptr;
    }
    audioStreamIdx_ = -1;
    audioDecoderDraining_ = false;
    audioEof_ = false;
    audioQueuedSamples_ = 0;
    audioPlayedSamples_ = 0;
    audioPcmBuffer_.clear();
    audioAvioBridge_.close();
    audioFmtCtx_ = nullptr;
    logControl(ControlCommand::Close, "audio-end");
}

void FFmpegVideoPlayer::flushAudioDecoder()
{
    if (audioCodecCtx_) avcodec_flush_buffers(audioCodecCtx_);
    if (swrCtx_) {
        std::vector<uint8_t> drain(audioSampleRate_ * audioBytesPerFrame_ / 2);
        uint8_t* ptr = drain.data();
        while (swr_convert(swrCtx_, &ptr,
                           (int)(drain.size() / audioBytesPerFrame_),
                           nullptr, 0) > 0) {
        }
    }
    audioDecoderDraining_ = false;
    audioEof_ = false;
}

void FFmpegVideoPlayer::resetAudio()
{
    logControl(ControlCommand::Flush, "audio-reset");
    if (audioBuffer_) audioBuffer_->Stop();
    if (decodeWorkerEnabled_) {
        resetAudioOutputOnly();
        commandDecodeSeek(0);
        return;
    }
    flushAudioDecoder();
    seekAudioMs(0);
    audioQueuedSamples_ = 0;
    audioPlayedSamples_ = 0;
}

void FFmpegVideoPlayer::resetAudioOutputOnly()
{
    logControl(ControlCommand::Flush, "audio-output-reset");
    if (audioBuffer_) audioBuffer_->Stop();
    audioQueuedSamples_ = 0;
    audioPlayedSamples_ = 0;
}

void FFmpegVideoPlayer::seekAudioMs(int64_t ms)
{
    if (!audioFmtCtx_ || !audioCodecCtx_ || audioStreamIdx_ < 0) return;
    logControl(ControlCommand::Seek, "audio-seek", ms);
    AVStream* audioStream = audioFmtCtx_->streams[audioStreamIdx_];
    int64_t audioTimestamp =
        (int64_t)((ms / 1000.0) / av_q2d(audioStream->time_base));
    if (av_seek_frame(audioFmtCtx_, audioStreamIdx_, audioTimestamp,
                      AVSEEK_FLAG_BACKWARD) < 0) {
        int64_t globalTimestamp = (int64_t)((ms / 1000.0) * AV_TIME_BASE);
        av_seek_frame(audioFmtCtx_, -1, globalTimestamp, AVSEEK_FLAG_BACKWARD);
    }
    flushAudioDecoder();
}

void FFmpegVideoPlayer::applyAudioSettings()
{
    if (!audioBuffer_) return;
    int volume = std::max(0, std::min(100000, audioVolume_));
    int balance = std::max(-100000, std::min(100000, audioBalance_));
    audioBuffer_->SetVolume(volume / 100000.0f);
    audioBuffer_->SetPan(balance / 100000.0f);
}

void FFmpegVideoPlayer::startDecodeWorker()
{
    stopDecodeWorker();
    decodeWorkerEnabled_ = krkr_ffmpeg_worker_enabled();
    if (!decodeWorkerEnabled_) return;

    {
        std::lock_guard<std::mutex> lock(decodeMutex_);
        decodeTerminate_ = false;
        decodePlayRequested_ = false;
        decodeSeekPending_ = false;
        decodeSeekMs_ = 0;
        decodeGeneration_++;
        decodeVideoEof_ = false;
        decodeAudioEofWorker_ = false;
        decodeFallbackFrame_ = -1;
        clearDecodeQueuesLocked();
    }
    logControl(ControlCommand::Open, "worker-start");
    decodeThread_ = new DecodeThread(this);
    fprintf(stderr, "[FFmpeg-VideoPlayer] Decode worker started: '%s'\n",
            storageName_.c_str());
}

void FFmpegVideoPlayer::stopDecodeWorker()
{
    logControl(ControlCommand::Close, "worker-stop-begin");
    DecodeThread* thread = nullptr;
    {
        std::lock_guard<std::mutex> lock(decodeMutex_);
        decodeTerminate_ = true;
        decodePlayRequested_ = false;
        decodeGeneration_++;
        clearDecodeQueuesLocked();
        thread = decodeThread_;
        decodeThread_ = nullptr;
    }
    decodeCv_.notify_all();
    if (thread) {
        thread->Terminate();
        thread->WaitFor();
        delete thread;
    }
    presentedBGRA_.clear();
    presentedStride_ = 0;
    decodeWorkerEnabled_ = false;
    logControl(ControlCommand::Close, "worker-stop-end");
}

void FFmpegVideoPlayer::clearDecodeQueuesLocked()
{
    decodedVideoFrames_.clear();
    decodedAudioChunks_.clear();
}

void FFmpegVideoPlayer::notifyDecodeWorker()
{
    decodeCv_.notify_all();
}

void FFmpegVideoPlayer::logDecodeWorkerPerf(const char* phase)
{
    if (!TVPPerfEnabled()) return;
#ifdef __EMSCRIPTEN__
    double now = emscripten_get_now();
#else
    double now = 0.0;
#endif
    if (decodeLastPerfLogMs_ > 0.0 && now - decodeLastPerfLogMs_ < 500.0)
        return;
    size_t videoQueued = 0;
    int audioQueued = 0;
    uint64_t generation = 0;
    bool playRequested = false;
    bool seekPending = false;
    bool videoEof = false;
    bool audioEof = false;
    {
        std::lock_guard<std::mutex> lock(decodeMutex_);
        videoQueued = decodedVideoFrames_.size();
        audioQueued = queuedWorkerAudioSamplesLocked();
        generation = decodeGeneration_;
        playRequested = decodePlayRequested_;
        seekPending = decodeSeekPending_;
        videoEof = decodeVideoEof_;
        audioEof = decodeAudioEofWorker_;
    }
    decodeLastPerfLogMs_ = now;
    fprintf(stderr,
        "[PERF] ffmpeg.worker phase=%s enabled=%d play=%d seek=%d video_q=%zu audio_q=%d generation=%llu video_eof=%d audio_eof=%d current_frame=%d position_ms=%lld\n",
        phase ? phase : "tick",
        decodeWorkerEnabled_ ? 1 : 0,
        playRequested ? 1 : 0,
        seekPending ? 1 : 0,
        videoQueued,
        audioQueued,
        (unsigned long long)generation,
        videoEof ? 1 : 0,
        audioEof ? 1 : 0,
        currentFrame_,
        (long long)positionMs_);
}

int FFmpegVideoPlayer::queuedWorkerAudioSamplesLocked() const
{
    int samples = 0;
    for (std::deque<DecodedAudioChunk>::const_iterator it = decodedAudioChunks_.begin();
         it != decodedAudioChunks_.end(); ++it) {
        if (it->generation == decodeGeneration_) samples += it->samples;
    }
    return samples;
}

void FFmpegVideoPlayer::commandDecodeSeek(int64_t ms)
{
    logControl(ControlCommand::Seek, "command-begin", ms);
    {
        std::lock_guard<std::mutex> lock(decodeMutex_);
        decodeGeneration_++;
        decodeSeekPending_ = true;
        decodeSeekMs_ = ms;
        decodeVideoEof_ = false;
        decodeAudioEofWorker_ = false;
        decodeFallbackFrame_ = (int)((double)ms / msPerFrame_) - 1;
        clearDecodeQueuesLocked();
    }
    notifyDecodeWorker();
    logControl(ControlCommand::Seek, "command-enqueued", ms);
}

bool FFmpegVideoPlayer::consumeWorkerVideoFrame()
{
	DecodedVideoFrame frame;
	size_t droppedStale = 0;
	size_t queueAfter = 0;
	{
		std::lock_guard<std::mutex> lock(decodeMutex_);
		while (!decodedVideoFrames_.empty() &&
			   decodedVideoFrames_.front().generation != decodeGeneration_) {
			decodedVideoFrames_.pop_front();
			++droppedStale;
		}
		if (decodedVideoFrames_.empty()) return false;
		frame = std::move(decodedVideoFrames_.front());
		decodedVideoFrames_.pop_front();
		queueAfter = decodedVideoFrames_.size();
	}
	presentedBGRA_ = std::move(frame.bgra);
	presentedStride_ = frame.stride;
	currentFrame_ = frame.frame;
	positionMs_ = frame.positionMs;
	if (TVPPerfEnabled()) {
		fprintf(stderr,
			"[PERF] movie.sync phase=video-consume storage=%s state=%s generation=%llu frame=%d position_ms=%lld video_q=%zu dropped_stale=%zu bytes=%zu stride=%d\n",
			storageName_.c_str(),
			stateName(),
			(unsigned long long)frame.generation,
			currentFrame_,
			(long long)positionMs_,
			queueAfter,
			droppedStale,
			presentedBGRA_.size(),
			presentedStride_);
	}
	return true;
}

void FFmpegVideoPlayer::decodeThreadMain()
{
    while (true) {
        uint64_t generation = 0;
        bool doSeek = false;
        int64_t seekMs = 0;
        {
            std::unique_lock<std::mutex> lock(decodeMutex_);
            decodeCv_.wait(lock, [this] {
                return decodeTerminate_ || decodePlayRequested_ || decodeSeekPending_;
            });
            if (decodeTerminate_) break;
            if (decodeSeekPending_) {
                doSeek = true;
                seekMs = decodeSeekMs_;
                decodeSeekPending_ = false;
                clearDecodeQueuesLocked();
            }
            generation = decodeGeneration_;
        }

        if (doSeek) {
            logControl(ControlCommand::Seek, "worker-begin", seekMs);
            if (fmtCtx_ && videoStreamIdx_ >= 0) {
                AVStream* stream = fmtCtx_->streams[videoStreamIdx_];
                int64_t timestamp = (int64_t)((seekMs / 1000.0) / av_q2d(stream->time_base));
                av_seek_frame(fmtCtx_, videoStreamIdx_, timestamp, AVSEEK_FLAG_BACKWARD);
                if (codecCtx_) avcodec_flush_buffers(codecCtx_);
            }
            if (audioFmtCtx_ && audioCodecCtx_ && audioStreamIdx_ >= 0) {
                AVStream* audioStream = audioFmtCtx_->streams[audioStreamIdx_];
                int64_t audioTimestamp =
                    (int64_t)((seekMs / 1000.0) / av_q2d(audioStream->time_base));
                if (av_seek_frame(audioFmtCtx_, audioStreamIdx_, audioTimestamp,
                                  AVSEEK_FLAG_BACKWARD) < 0) {
                    int64_t globalTimestamp = (int64_t)((seekMs / 1000.0) * AV_TIME_BASE);
                    av_seek_frame(audioFmtCtx_, -1, globalTimestamp, AVSEEK_FLAG_BACKWARD);
                }
                flushAudioDecoder();
            }
            {
                std::lock_guard<std::mutex> lock(decodeMutex_);
                if (generation == decodeGeneration_) {
                    decodeVideoEof_ = false;
                    decodeAudioEofWorker_ = false;
                }
            }
            logControl(ControlCommand::Seek, "worker-end", seekMs);
        }

        while (true) {
            bool shouldRun = false;
            size_t videoQueued = 0;
            int audioQueued = 0;
            {
                std::lock_guard<std::mutex> lock(decodeMutex_);
                if (decodeTerminate_ || generation != decodeGeneration_) break;
                shouldRun = decodePlayRequested_;
                videoQueued = decodedVideoFrames_.size();
                audioQueued = queuedWorkerAudioSamplesLocked();
            }
            if (!shouldRun) break;

            bool didWork = false;
            if (videoQueued < 6) {
                bool ok = decodeOneFrameForWorker(generation);
                didWork = true;
                if (!ok) {
                    std::lock_guard<std::mutex> lock(decodeMutex_);
                    if (generation == decodeGeneration_) decodeVideoEof_ = true;
                }
            }

            {
                std::lock_guard<std::mutex> lock(decodeMutex_);
                if (decodeTerminate_ || generation != decodeGeneration_) break;
                audioQueued = queuedWorkerAudioSamplesLocked();
            }
            if (audioEnabled_ && !decodeAudioEofWorker_ &&
                audioQueued < audioSampleRate_ * 2) {
                bool ok = decodeOneAudioChunkForWorker(generation);
                didWork = true;
                if (!ok) {
                    std::lock_guard<std::mutex> lock(decodeMutex_);
                    if (generation == decodeGeneration_) decodeAudioEofWorker_ = true;
                }
            }

            bool fullOrDone = false;
            {
                std::lock_guard<std::mutex> lock(decodeMutex_);
                if (decodeTerminate_ || generation != decodeGeneration_) break;
                fullOrDone =
                    (decodedVideoFrames_.size() >= 6 || decodeVideoEof_) &&
                    (!audioEnabled_ || decodeAudioEofWorker_ ||
                     queuedWorkerAudioSamplesLocked() >= audioSampleRate_ * 2);
            }
            if (fullOrDone || !didWork) {
                std::unique_lock<std::mutex> lock(decodeMutex_);
                decodeCv_.wait_for(lock, std::chrono::milliseconds(4), [this, generation] {
                    return decodeTerminate_ || generation != decodeGeneration_ ||
                           decodeSeekPending_ || !decodePlayRequested_ ||
                           decodedVideoFrames_.size() < 4 ||
                           queuedWorkerAudioSamplesLocked() < audioSampleRate_;
                });
                if (decodeTerminate_ || generation != decodeGeneration_ || decodeSeekPending_) break;
            }
        }
    }
}

void FFmpegVideoPlayer::close()
{
    logControl(ControlCommand::Close, "begin");
    setState(PlayerState::Flushing, "close-begin");
    stopDecodeWorker();
    isPlaying_ = false;
    isPaused_ = false;
    isComplete_ = false;
    isOpen_ = false;

    closeAudio();
    if (bgraBuffer_) { av_free(bgraBuffer_); bgraBuffer_ = nullptr; }
    if (packet_) { av_packet_free(&packet_); packet_ = nullptr; }
    if (frame_) { av_frame_free(&frame_); frame_ = nullptr; }
    if (frameBGRA_) { av_frame_free(&frameBGRA_); frameBGRA_ = nullptr; }
    if (swsCtx_) { sws_freeContext(swsCtx_); swsCtx_ = nullptr; }
    if (codecCtx_) { avcodec_free_context(&codecCtx_); codecCtx_ = nullptr; }
    // fmtCtx_ owned by avioBridge_
    avioBridge_.close();
    fmtCtx_ = nullptr;
    videoStreamIdx_ = -1;
    width_ = height_ = 0;
    currentFrame_ = 0;
    positionMs_ = 0;
    presentedBGRA_.clear();
    presentedStride_ = 0;
    setState(PlayerState::Closed, "close");
    logControl(ControlCommand::Close, "end");
}

void FFmpegVideoPlayer::play()
{
    if (!isOpen_) return;
    logControl(ControlCommand::Play, "begin");
    isPlaying_ = true;
    isPaused_ = false;
    isComplete_ = false;
    frameAccumMs_ = 0;
    if (audioBuffer_ && audioEnabled_) {
        applyAudioSettings();
        audioBuffer_->Play();
    }
    if (decodeWorkerEnabled_) {
        {
            std::lock_guard<std::mutex> lock(decodeMutex_);
            decodePlayRequested_ = true;
        }
        notifyDecodeWorker();
    }
    setState(PlayerState::Playing, "play");
    logSync("play");
    fprintf(stderr, "[FFmpeg-VideoPlayer] Play: '%s'\n", storageName_.c_str());
}

void FFmpegVideoPlayer::stop()
{
    if (!isOpen_) return;
    logControl(ControlCommand::Stop, "begin");
    setState(PlayerState::Flushing, "stop-begin");
    isPlaying_ = false;
    isPaused_ = false;
    if (decodeWorkerEnabled_) {
        {
            std::lock_guard<std::mutex> lock(decodeMutex_);
            decodePlayRequested_ = false;
        }
        if (audioBuffer_) audioBuffer_->Stop();
        resetAudioOutputOnly();
        commandDecodeSeek(0);
        currentFrame_ = 0;
        positionMs_ = 0;
        frameAccumMs_ = 0;
        isComplete_ = false;
        setState(PlayerState::Ready, "stop");
        logControl(ControlCommand::Stop, "end-worker");
        fprintf(stderr, "[FFmpeg-VideoPlayer] Stop: '%s'\n", storageName_.c_str());
        return;
    }
    if (audioBuffer_) audioBuffer_->Stop();
    rewind();
    setState(PlayerState::Ready, "stop");
    logControl(ControlCommand::Stop, "end");
    fprintf(stderr, "[FFmpeg-VideoPlayer] Stop: '%s'\n", storageName_.c_str());
}

void FFmpegVideoPlayer::pause()
{
    if (!isOpen_) return;
    logControl(ControlCommand::Pause, "begin");
    isPaused_ = true;
    isPlaying_ = false;
    if (audioBuffer_) audioBuffer_->Pause();
    if (decodeWorkerEnabled_) {
        {
            std::lock_guard<std::mutex> lock(decodeMutex_);
            decodePlayRequested_ = false;
        }
        notifyDecodeWorker();
    }
    setState(PlayerState::Paused, "pause");
    logSync("pause");
    fprintf(stderr, "[FFmpeg-VideoPlayer] Pause: '%s'\n", storageName_.c_str());
}

void FFmpegVideoPlayer::rewind()
{
    if (!isOpen_ || !fmtCtx_) return;
    logControl(ControlCommand::Seek, "rewind-begin", 0);
    setState(PlayerState::Seeking, "rewind-begin");
    if (decodeWorkerEnabled_) {
        resetAudioOutputOnly();
        commandDecodeSeek(0);
        currentFrame_ = 0;
        positionMs_ = 0;
        frameAccumMs_ = 0;
        isComplete_ = false;
        setState(isPlaying_ ? PlayerState::Playing : PlayerState::Ready, "rewind");
        logControl(ControlCommand::Seek, "rewind-end", 0);
        return;
    }
    av_seek_frame(fmtCtx_, videoStreamIdx_, 0, AVSEEK_FLAG_BACKWARD);
    if (codecCtx_) avcodec_flush_buffers(codecCtx_);
    resetAudio();
    currentFrame_ = 0;
    positionMs_ = 0;
    frameAccumMs_ = 0;
    isComplete_ = false;
    setState(isPlaying_ ? PlayerState::Playing : PlayerState::Ready, "rewind");
    logControl(ControlCommand::Seek, "rewind-end", 0);
}

bool FFmpegVideoPlayer::advanceFrame(double elapsedMs)
{
    if (!isOpen_ || !isPlaying_ || isPaused_ || isComplete_) return false;

    frameAccumMs_ += elapsedMs;
	if (decodeWorkerEnabled_) {
		pumpAudioFromWorker();
		logDecodeWorkerPerf("advance");

		bool decodedAny = false;
		if (presentedBGRA_.empty()) {
			if (consumeWorkerVideoFrame()) {
				decodedAny = true;
				frameAccumMs_ = 0.0;
			}
		}
		while (frameAccumMs_ >= msPerFrame_) {
			if (!consumeWorkerVideoFrame()) {
				bool workerComplete = false;
                {
                    std::lock_guard<std::mutex> lock(decodeMutex_);
                    workerComplete = decodeVideoEof_ && decodedVideoFrames_.empty();
                }
                if (workerComplete) {
                    if (audioBuffer_) audioBuffer_->Stop();
                    isComplete_ = true;
                    isPlaying_ = false;
                    {
                        std::lock_guard<std::mutex> lock(decodeMutex_);
                        decodePlayRequested_ = false;
                    }
                    setState(PlayerState::Ended, "eof");
                    logControl(ControlCommand::Eof, "worker-complete");
                    logSync("eof");
                    fprintf(stderr, "[FFmpeg-VideoPlayer] Complete: '%s' frame=%d\n",
                            storageName_.c_str(), currentFrame_);
                    return false;
                }
                break;
            }
            frameAccumMs_ -= msPerFrame_;
            decodedAny = true;
        }
        notifyDecodeWorker();
        return decodedAny;
    }

    pumpAudio();

    // Step frames based on accumulated time
    bool decodedAny = false;
    while (frameAccumMs_ >= msPerFrame_) {
        frameAccumMs_ -= msPerFrame_;
        if (!decodeOneFrame()) {
            // EOF reached
            if (audioBuffer_) audioBuffer_->Stop();
            isComplete_ = true;
            isPlaying_ = false;
            setState(PlayerState::Ended, "eof");
            logControl(ControlCommand::Eof, "complete");
            logSync("eof");
            fprintf(stderr, "[FFmpeg-VideoPlayer] Complete: '%s' frame=%d\n",
                    storageName_.c_str(), currentFrame_);
            return false;
        }
        decodedAny = true;
    }

    return decodedAny;
}

bool FFmpegVideoPlayer::decodeOneFrame()
{
    if (!fmtCtx_ || !codecCtx_) return false;

    while (true) {
        int ret = avcodec_receive_frame(codecCtx_, frame_);
        if (ret == AVERROR(EAGAIN)) {
            // Need more data
            ret = av_read_frame(fmtCtx_, packet_);
            if (ret < 0) {
                if (ret == AVERROR_EOF) {
                    // Flush decoder
                    avcodec_send_packet(codecCtx_, nullptr);
                    continue;
                }
                return false; // Error or EOF
            }
            if (packet_->stream_index == videoStreamIdx_) {
                avcodec_send_packet(codecCtx_, packet_);
            }
            av_packet_unref(packet_);
            continue;
        } else if (ret == AVERROR_EOF) {
            return false; // Fully flushed
        } else if (ret < 0) {
            return false; // Error
        }

        // Got a frame! Convert to RGBA
        sws_scale(swsCtx_,
                   frame_->data, frame_->linesize, 0, height_,
                   frameBGRA_->data, frameBGRA_->linesize);

        // Update position
        if (frame_->pts != AV_NOPTS_VALUE) {
            AVStream* stream = fmtCtx_->streams[videoStreamIdx_];
            double timeSec = frame_->pts * av_q2d(stream->time_base);
            positionMs_ = (int64_t)(timeSec * 1000.0);
            currentFrame_ = (int)(timeSec * fps_);
        } else {
            currentFrame_++;
            positionMs_ = (int64_t)(currentFrame_ * msPerFrame_);
        }

        av_frame_unref(frame_);
        return true;
    }
}

bool FFmpegVideoPlayer::decodeOneFrameForWorker(uint64_t generation)
{
    if (!fmtCtx_ || !codecCtx_) return false;

    while (true) {
        {
            std::lock_guard<std::mutex> lock(decodeMutex_);
            if (decodeTerminate_ || generation != decodeGeneration_) return true;
        }

        int ret = avcodec_receive_frame(codecCtx_, frame_);
        if (ret == AVERROR(EAGAIN)) {
            ret = av_read_frame(fmtCtx_, packet_);
            if (ret < 0) {
                if (ret == AVERROR_EOF) {
                    avcodec_send_packet(codecCtx_, nullptr);
                    continue;
                }
                return false;
            }
            if (packet_->stream_index == videoStreamIdx_) {
                avcodec_send_packet(codecCtx_, packet_);
            }
            av_packet_unref(packet_);
            continue;
        } else if (ret == AVERROR_EOF) {
            return false;
        } else if (ret < 0) {
            return false;
        }

        sws_scale(swsCtx_,
                   frame_->data, frame_->linesize, 0, height_,
                   frameBGRA_->data, frameBGRA_->linesize);

        DecodedVideoFrame item;
        item.stride = frameBGRA_->linesize[0];
        item.generation = generation;
        if (frame_->pts != AV_NOPTS_VALUE) {
            AVStream* stream = fmtCtx_->streams[videoStreamIdx_];
            double timeSec = frame_->pts * av_q2d(stream->time_base);
            item.positionMs = (int64_t)(timeSec * 1000.0);
            item.frame = (int)(timeSec * fps_);
        } else {
            int fallbackFrame = 0;
            {
                std::lock_guard<std::mutex> lock(decodeMutex_);
                fallbackFrame = ++decodeFallbackFrame_;
            }
            item.frame = fallbackFrame;
            item.positionMs = (int64_t)(fallbackFrame * msPerFrame_);
        }

        const int bytes = height_ * item.stride;
        item.bgra.resize(bytes);
        std::memcpy(item.bgra.data(), frameBGRA_->data[0], bytes);
        av_frame_unref(frame_);

        {
            std::lock_guard<std::mutex> lock(decodeMutex_);
            if (generation == decodeGeneration_ && !decodeTerminate_) {
                decodedVideoFrames_.push_back(std::move(item));
            }
        }
        return true;
    }
}

void FFmpegVideoPlayer::pumpAudio()
{
    if (!audioEnabled_ || !audioBuffer_ || audioEof_) return;

    audioPlayedSamples_ = audioBuffer_->GetCurrentPlaySamples();
    const int targetAheadSamples = audioSampleRate_;
    int guard = 0;
    while (audioBuffer_->IsBufferValid() &&
           (audioQueuedSamples_ - audioPlayedSamples_) < targetAheadSamples &&
           guard < 8) {
        if (!decodeOneAudioChunk()) break;
        audioPlayedSamples_ = audioBuffer_->GetCurrentPlaySamples();
        ++guard;
    }
    logSync("audio-pump");

    // AppendBuffer() calls EnsurePlayState(), so a source that underruns will
    // resume when fresh PCM is queued without inflating play diagnostics.
}

void FFmpegVideoPlayer::pumpAudioFromWorker()
{
    if (!audioEnabled_ || !audioBuffer_) return;

    audioPlayedSamples_ = audioBuffer_->GetCurrentPlaySamples();
    const int targetAheadSamples = audioSampleRate_;
    int guard = 0;
    while (audioBuffer_->IsBufferValid() &&
           (audioQueuedSamples_ - audioPlayedSamples_) < targetAheadSamples &&
           guard < 8) {
        DecodedAudioChunk chunk;
        {
            std::lock_guard<std::mutex> lock(decodeMutex_);
            while (!decodedAudioChunks_.empty() &&
                   decodedAudioChunks_.front().generation != decodeGeneration_) {
                decodedAudioChunks_.pop_front();
            }
            if (decodedAudioChunks_.empty()) break;
            chunk = std::move(decodedAudioChunks_.front());
            decodedAudioChunks_.pop_front();
        }
        if (!chunk.pcm.empty() && chunk.samples > 0) {
            audioBuffer_->AppendBuffer(chunk.pcm.data(), (unsigned int)chunk.pcm.size());
            audioQueuedSamples_ += chunk.samples;
        }
        audioPlayedSamples_ = audioBuffer_->GetCurrentPlaySamples();
        ++guard;
    }
    logSync("audio-pump-worker");
}

bool FFmpegVideoPlayer::decodeOneAudioChunk()
{
    if (!audioCodecCtx_ || !audioFrame_ || !swrCtx_ || !audioBuffer_) {
        return false;
    }

    while (true) {
        int ret = avcodec_receive_frame(audioCodecCtx_, audioFrame_);
        if (ret == AVERROR(EAGAIN)) {
            if (audioDecoderDraining_) {
                audioEof_ = true;
                return false;
            }
            ret = av_read_frame(audioFmtCtx_, audioPacket_);
            if (ret < 0) {
                if (ret == AVERROR_EOF) {
                    avcodec_send_packet(audioCodecCtx_, nullptr);
                    audioDecoderDraining_ = true;
                    continue;
                }
                audioEof_ = true;
                return false;
            }
            if (audioPacket_->stream_index == audioStreamIdx_) {
                avcodec_send_packet(audioCodecCtx_, audioPacket_);
            }
            av_packet_unref(audioPacket_);
            continue;
        } else if (ret == AVERROR_EOF) {
            audioEof_ = true;
            return false;
        } else if (ret < 0) {
            audioEof_ = true;
            return false;
        }

        int outSamples = swr_get_out_samples(swrCtx_, audioFrame_->nb_samples);
        if (outSamples <= 0) {
            av_frame_unref(audioFrame_);
            continue;
        }

        size_t needed = (size_t)outSamples * audioBytesPerFrame_;
        if (audioPcmBuffer_.size() < needed) {
            audioPcmBuffer_.resize(needed);
        }

        uint8_t* outPtr = audioPcmBuffer_.data();
        int converted = swr_convert(swrCtx_, &outPtr, outSamples,
                                     (const uint8_t**)audioFrame_->extended_data,
                                     audioFrame_->nb_samples);
        av_frame_unref(audioFrame_);
        if (converted <= 0) {
            continue;
        }

        unsigned int bytes = (unsigned int)(converted * audioBytesPerFrame_);
        audioBuffer_->AppendBuffer(audioPcmBuffer_.data(), bytes);
        audioQueuedSamples_ += converted;
        return true;
    }
}

bool FFmpegVideoPlayer::decodeOneAudioChunkForWorker(uint64_t generation)
{
    if (!audioCodecCtx_ || !audioFrame_ || !swrCtx_ || audioStreamIdx_ < 0) {
        return false;
    }

    while (true) {
        {
            std::lock_guard<std::mutex> lock(decodeMutex_);
            if (decodeTerminate_ || generation != decodeGeneration_ || !audioEnabled_) return true;
        }

        int ret = avcodec_receive_frame(audioCodecCtx_, audioFrame_);
        if (ret == AVERROR(EAGAIN)) {
            if (audioDecoderDraining_) {
                return false;
            }
            ret = av_read_frame(audioFmtCtx_, audioPacket_);
            if (ret < 0) {
                if (ret == AVERROR_EOF) {
                    avcodec_send_packet(audioCodecCtx_, nullptr);
                    audioDecoderDraining_ = true;
                    continue;
                }
                return false;
            }
            if (audioPacket_->stream_index == audioStreamIdx_) {
                avcodec_send_packet(audioCodecCtx_, audioPacket_);
            }
            av_packet_unref(audioPacket_);
            continue;
        } else if (ret == AVERROR_EOF) {
            return false;
        } else if (ret < 0) {
            return false;
        }

        int outSamples = swr_get_out_samples(swrCtx_, audioFrame_->nb_samples);
        if (outSamples <= 0) {
            av_frame_unref(audioFrame_);
            continue;
        }

        size_t needed = (size_t)outSamples * audioBytesPerFrame_;
        if (audioPcmBuffer_.size() < needed) {
            audioPcmBuffer_.resize(needed);
        }

        uint8_t* outPtr = audioPcmBuffer_.data();
        int converted = swr_convert(swrCtx_, &outPtr, outSamples,
                                     (const uint8_t**)audioFrame_->extended_data,
                                     audioFrame_->nb_samples);
        av_frame_unref(audioFrame_);
        if (converted <= 0) {
            continue;
        }

        DecodedAudioChunk chunk;
        chunk.samples = converted;
        chunk.generation = generation;
        chunk.pcm.resize((size_t)converted * audioBytesPerFrame_);
        std::memcpy(chunk.pcm.data(), audioPcmBuffer_.data(), chunk.pcm.size());
        {
            std::lock_guard<std::mutex> lock(decodeMutex_);
            if (generation == decodeGeneration_ && !decodeTerminate_ && audioEnabled_) {
                decodedAudioChunks_.push_back(std::move(chunk));
            }
        }
        return true;
    }
}

const uint8_t* FFmpegVideoPlayer::getFrameBGRA() const
{
    if (decodeWorkerEnabled_ && !presentedBGRA_.empty()) {
        return presentedBGRA_.data();
    }
    if (!frameBGRA_) return nullptr;
    return frameBGRA_->data[0];
}

int FFmpegVideoPlayer::getFrameStride() const
{
    if (decodeWorkerEnabled_ && presentedStride_ > 0 && !presentedBGRA_.empty()) {
        return presentedStride_;
    }
    if (!frameBGRA_) return 0;
    return frameBGRA_->linesize[0];
}

void FFmpegVideoPlayer::setFrame(int f)
{
    if (!isOpen_ || !fmtCtx_) return;
    double timeSec = (double)f / fps_;
    setPositionMs((int64_t)(timeSec * 1000.0));
}

void FFmpegVideoPlayer::setPositionMs(int64_t ms)
{
    if (!isOpen_ || !fmtCtx_) return;
    logControl(ControlCommand::Seek, "begin", ms);
    PlayerState resumeState = isPlaying_ ? PlayerState::Playing : PlayerState::Ready;
    setState(PlayerState::Seeking, "seek-begin");
    if (decodeWorkerEnabled_) {
        commandDecodeSeek(ms);
        if (isPlaying_ && audioBuffer_ && audioEnabled_) audioBuffer_->Play();
        positionMs_ = ms;
        currentFrame_ = (int)((double)ms / msPerFrame_);
        frameAccumMs_ = 0;
        isComplete_ = false;
        setState(resumeState, "seek");
        logControl(ControlCommand::Seek, "end-worker", ms);
        return;
    }
    AVStream* stream = fmtCtx_->streams[videoStreamIdx_];
    int64_t timestamp = (int64_t)((ms / 1000.0) / av_q2d(stream->time_base));
    av_seek_frame(fmtCtx_, videoStreamIdx_, timestamp, AVSEEK_FLAG_BACKWARD);
    if (codecCtx_) avcodec_flush_buffers(codecCtx_);
    resetAudio();
    seekAudioMs(ms);
    if (isPlaying_ && audioBuffer_ && audioEnabled_) audioBuffer_->Play();
    positionMs_ = ms;
    currentFrame_ = (int)((double)ms / msPerFrame_);
    isComplete_ = false;
    setState(resumeState, "seek");
    logControl(ControlCommand::Seek, "end", ms);
}

void FFmpegVideoPlayer::setAudioVolume(int volume)
{
    audioVolume_ = std::max(0, std::min(100000, volume));
    applyAudioSettings();
    logControl(ControlCommand::SetVolume, "audio-volume", audioVolume_);
}

void FFmpegVideoPlayer::setAudioBalance(int balance)
{
    audioBalance_ = std::max(-100000, std::min(100000, balance));
    applyAudioSettings();
    logControl(ControlCommand::SetVolume, "audio-balance", audioBalance_);
}

void FFmpegVideoPlayer::selectAudioStream(unsigned int n)
{
    if (!fmtCtx_) return;

    if ((int)n < 0 || (audioStreamCount_ > 0 && n >= (unsigned int)audioStreamCount_)) {
        return;
    }

    requestedAudioStreamOrdinal_ = (int)n;
    logControl(ControlCommand::Flush, "audio-stream-switch", requestedAudioStreamOrdinal_);
    setState(PlayerState::Flushing, "audio-stream-switch");
    bool wasWorker = decodeWorkerEnabled_;
    if (wasWorker) stopDecodeWorker();
    closeAudio();
    openAudioStream();
    if (wasWorker) {
        startDecodeWorker();
        commandDecodeSeek(positionMs_);
    } else {
        seekAudioMs(positionMs_);
    }
    if (isPlaying_ && audioBuffer_ && audioEnabled_) {
        audioBuffer_->Play();
        if (decodeWorkerEnabled_) {
            {
                std::lock_guard<std::mutex> lock(decodeMutex_);
                decodePlayRequested_ = true;
            }
            notifyDecodeWorker();
        }
    }
    setState(isPlaying_ ? PlayerState::Playing : PlayerState::Ready, "audio-stream-ready");
    logSync("audio-stream-switch");
}

void FFmpegVideoPlayer::disableAudioStream()
{
    audioEnabled_ = false;
    if (audioBuffer_) audioBuffer_->Pause();
    {
        std::lock_guard<std::mutex> lock(decodeMutex_);
        decodedAudioChunks_.clear();
        decodeGeneration_++;
    }
    audioQueuedSamples_ = 0;
    audioPlayedSamples_ = 0;
    logControl(ControlCommand::Flush, "audio-disable");
    logSync("audio-disable");
}

} // namespace video
} // namespace krkr

#endif // KRKRSDL2_FFMPEG_ENABLED
