/*
 * FFmpeg Video Player for KRKRSDL2 (Emscripten)
 * Copyright (c) Kirikiri SDL2 Developers
 * SPDX-License-Identifier: MIT
 *
 * Phase 4: Self-contained video player that integrates with KRKR's
 * VideoOverlay contract. Uses the AVIO bridge to read from XP3,
 * FFmpeg to demux/decode video, libswscale to convert to KRKR 32bpp
 * BGRA/ARGB memory layout,
 * and fires KRKR events (onStatusChanged, onPeriod, onFrameUpdate)
 * at the right times.
 */

#pragma once

#if KRKRSDL2_FFMPEG_ENABLED

#include "FFmpegAVIO.h"
#include "OpenALWaveMixer.h"
#include <string>
#include <cstdint>
#include <vector>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <chrono>

struct AVFormatContext;
struct AVCodecContext;
struct SwsContext;
struct SwrContext;
struct AVPacket;
struct AVFrame;

namespace krkr {
namespace video {

/**
 * FFmpeg-based video player for Emscripten VideoOverlay.
 *
 * Manages the decode state machine:
 *   Idle -> Opened (Stop) -> Playing -> Complete (Stop)
 *                                    -> Paused
 *
 * The owner (tTJSNI_VideoOverlay) calls open/play/stop/pause/close
 * and queries position/frame/fps/duration. This class manages the
 * FFmpeg pipeline and fires events via callbacks.
 */
class FFmpegVideoPlayer {
public:
    FFmpegVideoPlayer();
    ~FFmpegVideoPlayer();

    // Non-copyable
    FFmpegVideoPlayer(const FFmpegVideoPlayer&) = delete;
    FFmpegVideoPlayer& operator=(const FFmpegVideoPlayer&) = delete;

    /**
     * Open a video file from KRKR storage.
     * @return true if probe + codec init succeeded
     */
    bool open(const std::string& storageName);

    /**
     * Close and release all resources.
     */
    void close();

    /**
     * Start/resume playback.
     * After calling play(), isPlaying() returns true and
     * advanceFrame() should be called periodically.
     */
    void play();

    /**
     * Stop playback. Resets to beginning.
     */
    void stop();

    /**
     * Pause playback. Position is preserved.
     */
    void pause();

    /**
     * Rewind to the beginning.
     */
    void rewind();

    /**
     * Advance one frame. Call this from a timer/main loop callback.
     * @param elapsedMs milliseconds since last call
     * @return true if a frame was decoded, false if EOF or error
     */
    bool advanceFrame(double elapsedMs);

    // ---- Queries ----
    bool isOpen() const { return isOpen_; }
    bool isPlaying() const { return isPlaying_; }
    bool isPaused() const { return isPaused_; }
    bool isComplete() const { return isComplete_; }

    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
    double getFps() const { return fps_; }
    int getFrame() const { return currentFrame_; }
    int getNumberOfFrames() const { return totalFrames_; }
    int64_t getTotalTimeMs() const { return totalTimeMs_; }
    int64_t getPositionMs() const { return positionMs_; }
    const uint8_t* getFrameBGRA() const;
    int getFrameStride() const;
    bool hasAudio() const { return audioStreamIdx_ >= 0 && audioCodecCtx_ != nullptr; }
    int getNumberOfAudioStreams() const { return audioStreamCount_; }
    int getEnabledAudioStream() const { return audioEnabled_ && hasAudio() ? requestedAudioStreamOrdinal_ : -1; }

    void setFrame(int f);
    void setPositionMs(int64_t ms);
    void setAudioVolume(int volume);
    int getAudioVolume() const { return audioVolume_; }
    void setAudioBalance(int balance);
    int getAudioBalance() const { return audioBalance_; }
    void selectAudioStream(unsigned int n);
    void disableAudioStream();

private:
    enum class PlayerState {
        Closed,
        Opening,
        Ready,
        Playing,
        Paused,
        Seeking,
        Flushing,
        Ended,
        Error
    };
    enum class ControlCommand {
        Open,
        Close,
        Play,
        Pause,
        Stop,
        Seek,
        Flush,
        SetLoop,
        SetVolume,
        SetRect,
        Eof,
        Error
    };
    struct DecodedVideoFrame {
        std::vector<uint8_t> bgra;
        int stride;
        int frame;
        int64_t positionMs;
        uint64_t generation;
    };
    struct DecodedAudioChunk {
        std::vector<uint8_t> pcm;
        int samples;
        uint64_t generation;
    };
    class DecodeThread;

    bool decodeOneFrame(); // internal: decode next frame from stream
    bool decodeOneFrameForWorker(uint64_t generation);
    bool openAudioStream();
    void closeAudio();
    void resetAudio();
    void resetAudioOutputOnly();
    void seekAudioMs(int64_t ms);
    void pumpAudio();
    void pumpAudioFromWorker();
    bool decodeOneAudioChunk();
    bool decodeOneAudioChunkForWorker(uint64_t generation);
    void applyAudioSettings();
    void flushAudioDecoder();
    void startDecodeWorker();
    void stopDecodeWorker();
    void decodeThreadMain();
    void clearDecodeQueuesLocked();
    void commandDecodeSeek(int64_t ms);
    void notifyDecodeWorker();
    bool consumeWorkerVideoFrame();
    int queuedWorkerAudioSamplesLocked() const;
    void logDecodeWorkerPerf(const char* phase);
    const char* stateName() const;
    const char* commandName(ControlCommand cmd) const;
    void setState(PlayerState state, const char* reason);
    void logControl(ControlCommand cmd, const char* phase, int64_t arg = 0) const;
    void logSync(const char* phase);

    krkr::avio::FFmpegAVIOBridge avioBridge_;
    krkr::avio::FFmpegAVIOBridge audioAvioBridge_;
    AVFormatContext* fmtCtx_;
    AVFormatContext* audioFmtCtx_;
    AVCodecContext*  codecCtx_;
    AVCodecContext*  audioCodecCtx_;
    SwsContext*      swsCtx_;
    SwrContext*      swrCtx_;
    AVPacket*        packet_;
    AVPacket*        audioPacket_;
    AVFrame*         frame_;
    AVFrame*         audioFrame_;
    AVFrame*         frameBGRA_;
    uint8_t*         bgraBuffer_;
    int              videoStreamIdx_;
    int              audioStreamIdx_;
    int              audioStreamCount_;
    int              requestedAudioStreamOrdinal_;
    iTVPSoundBuffer* audioBuffer_;
    bool             audioEnabled_;
    bool             audioDecoderDraining_;
    bool             audioEof_;
    int              audioSampleRate_;
    int              audioChannels_;
    int              audioBytesPerFrame_;
    int64_t          audioQueuedSamples_;
    int64_t          audioPlayedSamples_;
    int              audioVolume_;
    int              audioBalance_;
    std::vector<uint8_t> audioPcmBuffer_;
    std::vector<uint8_t> presentedBGRA_;
    int              presentedStride_;

    DecodeThread*    decodeThread_;
    bool             decodeWorkerEnabled_;
    bool             decodeTerminate_;
    bool             decodePlayRequested_;
    bool             decodeSeekPending_;
    int64_t          decodeSeekMs_;
    uint64_t         decodeGeneration_;
    bool             decodeVideoEof_;
    bool             decodeAudioEofWorker_;
    int              decodeFallbackFrame_;
    std::deque<DecodedVideoFrame> decodedVideoFrames_;
    std::deque<DecodedAudioChunk> decodedAudioChunks_;
    mutable std::mutex decodeMutex_;
    std::condition_variable decodeCv_;
    double           decodeLastPerfLogMs_;
    double           syncLastPerfLogMs_;

    // State
    PlayerState      state_;
    bool isOpen_;
    bool isPlaying_;
    bool isPaused_;
    bool isComplete_;

    // Video info
    int    width_;
    int    height_;
    double fps_;
    int    totalFrames_;
    int64_t totalTimeMs_;

    // Position tracking
    int    currentFrame_;
    int64_t positionMs_;
    double  frameAccumMs_;  // accumulated ms for frame stepping
    double  msPerFrame_;    // target ms per frame (1000/fps)

    std::string storageName_;
};

} // namespace video
} // namespace krkr

#endif // KRKRSDL2_FFMPEG_ENABLED
