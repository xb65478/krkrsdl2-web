# FFmpeg WASM Integration

本文记录 `krkrsdl2` WASM 构建中的 FFmpeg 静态库接入状态。当前文件覆盖阶段 1 build skeleton、阶段 2 KRKR Storage / XP3 AVIO probe、阶段 3 direct `WaveSoundBuffer` audio、阶段 4 `VideoOverlay` movie A/V 最小闭环，以及后续格式兼容性模型。当前项目主线已切到 WebGL renderer；FFmpeg 后续扩展转为待做 / 待优化项。

## 当前结论

- FFmpeg version: `6.1`
- Target: Emscripten/WASM static libraries
- Default license mode: LGPL-first
- Current checked-in library mode: GPL codec experiment enabled for DRACU-RIOT! WMV probe
- Current runtime state: libraries、KRKR Storage AVIO probe、direct `WaveSoundBuffer` audio phase3B 已完成；`VideoOverlay` movie A/V phase4 已能通过 FFmpeg decode、KRKR Layer 输出和 OpenAL/WebAudio movie audio queue 播放真实 `.wmv`；用户手动验收确认 EXTRA -> movie 第一个解锁 movie 可见、有声、点击可停止返回。
- Current roadmap state: FFmpeg 不再是当前下一步主线；主流程原脚本 BGM/SE/voice 自动命中、click-skip/wait reason、AlphaMovie、longrun 和跨包格式 profile 扩展均列为待做 / 待优化项。下一步主线见 `WEBGL_RENDERER_PLAN_2026-05-18.md`。

## Build Modes

### LGPL-First Default

默认命令：

```bash
cd /Users/xiabin/my_work1/krkrsdl2
./build_ffmpeg_wasm.sh
```

默认配置不启用 `--enable-gpl`、`--enable-version3` 或 `--enable-nonfree`。它主要覆盖 Ogg/Vorbis/Opus、MP3、FLAC、AAC、WAV/PCM 以及常见容器 probe。

### GPL Codec Experiment

DRACU-RIOT! 的 `video.xp3` 包含 `.wmv`，这可能需要 WMV/WMA/VC-1 相关组件。该路径只能显式打开：

```bash
cd /Users/xiabin/my_work1/krkrsdl2
KRKRSDL2_FFMPEG_ENABLE_GPL_CODECS=1 ./build_ffmpeg_wasm.sh
```

打开后会额外启用 ASF demuxer、WMV/WMA/VC-1/MSMPEG4 decoder 和相关 parser。提交或发布前必须在文档和提交说明中明确许可证边界。

2026-05-18 当前构建已按该模式重建，用于验证 DRACU-RIOT! `video.xp3` 内 `.wmv`。`configure` 摘要显示 license 为 GPL v2+。真实 OP / loop movie 的音轨为 WMAPRO，因此本轮显式补启用 `wmapro`；同时补开 `wmalossless`、`wmavoice` 作为 WMA 家族边界，后续发布/分发策略必须重新确认。

## Compatibility Model

本项目不按“每个格式写一个函数”的方式扩展媒体支持。实际兼容性由三层共同决定：

```text
KRKR/TJS media contract
  -> FFmpeg probe / demux / codec id
  -> wasm FFmpeg enabled components
```

### KRKR / Script Contract

脚本层主要决定“资源应交给哪个媒体对象”，不是负责解码：

| 入口 | 当前观察 | 含义 |
|---|---|---|
| `WaveSoundBuffer` / BGM / SE / Voice | 当前包体主要查 `.ogg` / `.wav` | 音频契约入口；解码由 wave decoder / FFmpeg 完成 |
| `VideoOverlay` / `sysmovie` / movie | DRACU 脚本查 `mpg` / `mpv` / `mpeg` / `wmv` | 普通 movie 契约入口；FFmpeg probe 容器和 codec |
| `AlphaMovie.dll` / `AlphaMovie` | DRACU 脚本查 `.amv` | 透明/特殊 movie 契约；不是普通 FFmpeg movie |
| `.sli` | 音频 sidecar | loop / label 等 KRKR 信息，不是音频 codec |

### FFmpeg Probe Semantics

SDL3 和 Kirikiroid2 / Android 侧的语义都不是为 WMV、MPG、OGG 各写一个业务函数，而是：

1. 用自定义 IO 接到 KRKR Storage / XP3；
2. 让 FFmpeg probe 容器；
3. 枚举 stream；
4. 按 `codec_id` 调 `avcodec_find_decoder`；
5. 解码 PCM / video frame；
6. 回填 KRKR 的 `WaveSoundBuffer` / `VideoOverlay` 状态机。

本地参考入口：

```text
/Users/xiabin/my_work1/krkrsdl3/cpp/core/media/movie/CodecDemuxFFmpeg.cpp
/Users/xiabin/my_work1/krkrsdl3/cpp/core/media/movie/CodecVideoFFmpeg.cpp
/Users/xiabin/my_work1/krkrsdl3/cpp/core/media/movie/CodecAudioFFmpeg.cpp
/Users/xiabin/my_work1/Kirikiroid2/src/core/movie/ffmpeg/DemuxFFmpeg.cpp
/Users/xiabin/my_work1/Kirikiroid2/src/core/movie/ffmpeg/VideoCodecFFmpeg.cpp
/Users/xiabin/my_work1/Kirikiroid2/src/core/movie/ffmpeg/AudioCodecFFmpeg.cpp
```

### Wasm Build Profile

WASM 版为了体积、启动时间、许可证和可控性，`build_ffmpeg_wasm.sh` 使用 `--disable-everything` 后按需打开组件。因此“FFmpeg 支持某格式”和“当前 wasm 构建支持某格式”不是一回事。

后续新增格式时优先做 profile，而不是直接打开全量 FFmpeg：

| Profile | 用途 | 方向 |
|---|---|---|
| LGPL default | 常见音频和较安全默认分发 | `ogg/vorbis/opus`、`wav/pcm`、`mp3`、`flac`、`aac`、`mov/matroska/avi` 等已启用或按需扩展 |
| VN/full experiment | 商业 VN 老格式覆盖 | `asf/wmv/wma`、`mpeg/mpg`、`msmpeg4`、更多 parser；需要许可证、体积、性能说明 |
| Package-specific probe | 新游戏包验收 | 用 inventory / ffprobe / `diag_ffmpeg_avio_probe.js` 找出真实 container + codec，再决定是否扩 profile |

### Current DRACU-RIOT! Inventory

当前主样本的媒体清单来自：

```text
/Users/xiabin/my_work1/web-krkr/diag-results/phase0-inventory-2026-05-18/media-inventory.md
```

已观察到的真实格式：

| 类型 | 文件扩展 | container / codec | 当前处理 |
|---|---|---|---|
| BGM / SE / Voice | `.ogg` | Ogg / Vorbis | direct `WaveSoundBuffer` phase3B 已完成 |
| Movie | `.wmv` | ASF / WMV3 video / WMAV2 或 WMAPRO audio | direct `VideoOverlay` movie A/V phase4 已完成 |
| AlphaMovie | `.amv` | FFmpeg 普通 probe 失败；AlphaMovie candidate | 待做，不归入普通 movie |
| Loop sidecar | `.sli` | KRKR sidecar | 待做 / longrun audio 语义 |
| Companion image | `.png` | PNG | 视具体 movie/AlphaMovie 路径处理 |

本机 ffprobe 抽样结论：

- `OP_low.wmv`：ASF，`wmv3` 640x360 24fps + `wmapro` 44100Hz stereo；
- `mono_loop.wmv`：ASF，`wmv3` 1280x720 30fps + `wmapro` 44100Hz stereo；
- ED low 资源：ASF，`wmv3` + `wmav2`；
- BGM / SE / Voice：Ogg / Vorbis；
- `sea_loop.amv`：FFmpeg 普通 probe 报 invalid data，需走 AlphaMovie 语义。

### Compatibility Risks

- 扩 FFmpeg profile 会增加 wasm 体积、下载时间、初始化时间和内存压力。
- GPL / version3 / nonfree 边界必须单独记录；当前 `.wmv` 验证使用 GPL codec experiment。
- 浏览器移动端性能可能比桌面差；需要 longrun 和实际设备观察。
- 对普通 container/codec，通常只需扩 FFmpeg build profile 和诊断矩阵；对 `.amv` / AlphaMovie、`.sli` loop sidecar 这类 KRKR 特殊契约，需要实现对象语义，不是打开 decoder 就够。
- 新包体接入时必须先跑 inventory / probe，不能按扩展名直接假设 codec。

## Default Enabled Components

### Libraries

```text
libavformat
libavcodec
libavutil
libswresample
libswscale
```

### Demuxers

| Demuxer | Format |
|---|---|
| ogg | `.ogg` |
| avi | `.avi` |
| mov | `.mp4`, `.m4v`, `.m4a`, `.mov` |
| matroska | `.mkv`, `.webm` |
| wav | `.wav` |
| mp3 | `.mp3` |
| flac | `.flac` |
| aac | `.aac` |

### Decoders

| Decoder | Type |
|---|---|
| vorbis | audio |
| opus | audio |
| mp3 | audio |
| flac | audio |
| aac | audio |
| pcm_s16le / pcm_s16be | audio |
| pcm_f32le / pcm_f32be | audio |

### GPL Experiment Components

| Component | Purpose |
|---|---|
| asf demuxer | `.wmv` / `.wma` container |
| wmav1 / wmav2 | WMA audio |
| wmapro / wmalossless / wmavoice | WMA Pro / extended WMA audio |
| wmv1 / wmv2 / wmv3 / wmv3image | WMV video |
| vc1 | VC-1 video |
| msmpeg4v1/v2/v3 | legacy Microsoft MPEG-4 video |

## File Structure

```text
krkrsdl2/
├── external/ffmpeg/
│   ├── ffmpeg-6.1/
│   └── install/
│       ├── include/
│       └── lib/
├── src/core/sound/sdl2/FFmpegAVIO.*
├── src/core/sound/sdl2/FFmpegAudioDecoder.*
├── src/core/sound/sdl2/FFmpegWaveDecoder.*
├── src/core/visual/sdl2/FFmpegVideoDecoder.*
├── src/core/visual/sdl2/FFmpegVideoPlayer.*
├── src/plugins/kremscripten.cpp  (Module.probeFFmpegAVIO / Module.getAudioDiagState)
├── build_ffmpeg_wasm.sh
└── CMakeLists.txt
```

## Build Instructions

```bash
cd /Users/xiabin/my_work1/krkrsdl2
./build_ffmpeg_wasm.sh
cmake --build /Users/xiabin/my_work1/krkrsdl2/build_web -j 8
```

## C++ Probe Shells

The current decoder classes are standalone shells:

```cpp
#if KRKRSDL2_FFMPEG_ENABLED
#include "FFmpegAudioDecoder.h"

krkr::audio::FFmpegAudioDecoder decoder;
if (decoder.open("audio.ogg")) {
    uint8_t buffer[4096];
    int bytes = decoder.decode(buffer, sizeof(buffer));
    decoder.close();
}
#endif
```

```cpp
#if KRKRSDL2_FFMPEG_ENABLED
#include "FFmpegVideoDecoder.h"

krkr::video::FFmpegVideoDecoder decoder;
if (decoder.open("video.wmv")) {
    uint8_t* data = nullptr;
    int linesize = 0;
    if (decoder.decodeNextFrame(&data, &linesize)) {
        // RGBA frame data is available here.
    }
    decoder.close();
}
#endif
```

`open()` 当前走 `FFmpegAVIOBridge`，即 KRKR Storage / XP3；`openPath()` 仅保留为普通文件路径测试入口。

## KRKR Storage AVIO Probe

阶段 2 已完成。浏览器侧入口：

```bash
cd /Users/xiabin/my_work1/web-krkr
node diag_ffmpeg_avio_probe.js
```

通过 artifact：

```text
/tmp/krkr-ffmpeg-avio-probe-summary.json
/Users/xiabin/my_work1/web-krkr/diag-results/ffmpeg-avio-phase2-2026-05-17T17-22-21-683Z/summary.json
```

当前通过样本：

| Kind | Storage | Format / codec | Evidence |
|---|---|---|---|
| audio | `http://127.0.0.1:8085/DRACU-RIOT!/bgm.xp3>BGM01.ogg` | `ogg` / `vorbis` | `audioStreamCount=1`, `readCalls=6`, `seekCalls=3`, `readErrors=0`, `seekErrors=0` |
| video | `http://127.0.0.1:8085/DRACU-RIOT!/video.xp3>mono_loop.wmv` | `asf` / `wmv3` | `videoStreamCount=1`, `1280x720`, `fps=30`, `readCalls=46`, `readErrors=0`, `seekErrors=0` |

## WaveSoundBuffer Audio Phase 3B

Direct `WaveSoundBuffer` audio runtime 已完成 phase3B。浏览器侧入口：

```bash
cd /Users/xiabin/my_work1/web-krkr
node diag_audio_state.js
```

通过 artifact：

```text
/tmp/krkr-audio-state-summary.json
/Users/xiabin/my_work1/web-krkr/diag-results/audio-phase3-2026-05-18T07-15-30-145Z/summary.json
```

通过样本：

| Kind | Storage | Evidence |
|---|---|---|
| long BGM | `http://127.0.0.1:8085/DRACU-RIOT!/bgm.xp3>BGM01.ogg` | `samplePosition` 从 `0` 前进到 `143380`；pause drift 有界；resume 后到 `189111`；stop 后 `status=stop` |
| short voice | `http://127.0.0.1:8085/DRACU-RIOT!/voice.xp3>azu/azu205_021.ogg` | natural end 后 `status=stop` |
| short voice loop | 同上 | 超过自然时长后仍 `status=play` 且 `looping=true` |

计数证据包括 FFmpeg wave open/render、OpenAL/WebAudio append/play、wasm pump trigger/fill 均增长。

边界：

- 这是 direct TJS `WaveSoundBuffer` 真实资源契约，不证明游戏主流程原 KAG/TJS 已自动命中 BGM/SE/voice。
- `wuvorbis.dll` facade、音量/声像/频率长流程、seek、wait/label 仍待后续专项覆盖。

## VideoOverlay Movie A/V Phase 4

`VideoOverlay` movie A/V 最小闭环已完成。浏览器侧入口：

```bash
cd /Users/xiabin/my_work1/web-krkr
node diag_video_wait.js
```

关键实现：

- `src/core/visual/sdl2/FFmpegVideoPlayer.*` 通过 KRKR Storage / XP3 AVIO 打开 movie；
- video 侧 FFmpeg demux/decode，`libswscale` 转 BGRA，写回 KRKR layer frame buffer；
- audio 侧用独立 AVIO / `AVFormatContext` 解码 movie 音轨，`libswresample` 转 s16 stereo PCM，进入现有 OpenAL/WebAudio `iTVPSoundBuffer` streaming queue；
- `VideoOvlImpl.cpp` 在 WASM main loop tick 中推进 FFmpeg player，并把 `audioVolume`、`audioBalance`、`numberOfAudioStream`、`enabledAudioStream`、`selectAudioStream`、`disableAudioStream` 转发到 FFmpeg player；
- `diag_video_wait.js` 已记录 movie audio stream / append / play deltas，并产出截图 artifact 供人工或多模态视觉验收。

通过 artifact：

```text
/tmp/krkr-video-wait-summary.json
/Users/xiabin/my_work1/web-krkr/diag-results/video-phase4-2026-05-18T07-26-41-001Z/summary.json
/tmp/krkr-oplow-video-audio-summary.json
/Users/xiabin/my_work1/web-krkr/diag-results/video-phase4-oplow-2026-05-18T07-26-41-001Z/summary.json
```

通过样本：

| Storage | Video | Audio | Evidence |
|---|---|---|---|
| `video.xp3>mono_loop.wmv` | `wmv3` 1280x720 30fps | `wmapro` 44100Hz stereo | `openedRealVideo=true`、`frameAdvanced=true`、`naturalCompleteObserved=true`、`movieAudioQueued=true`、`movieAudioOutputStarted=true`、OpenAL `appendBytes=442368` |
| `video.xp3>OP_low.wmv` | `wmv3` 640x360 24fps | `wmapro` 44100Hz stereo | `openedRealVideo=true`、`frameAdvanced=true`、`movieAudioQueued=true`、`movieAudioOutputStarted=true`、截图 `/tmp/krkr-oplow-video-audio-frame.png`；长 OP 本轮跳过自然结束等待 |

用户手动验收：

- 路径：进入首页后点击 EXTRA -> movie -> 第一个解锁 movie；
- 结果：视频有画面、有声音；
- 点击 during movie 后会停止并返回游戏界面。

边界：

- 这是 `VideoOverlay` direct movie A/V 最小闭环，不等于所有游戏脚本 wait/skip 语义完成。
- `waitvideo` / `wm` 的 natural ended、stop/error reason 仍需后续单独收敛；手动点击停止路径已通过一次实际 UI 验收，但 reason / longrun 仍需诊断固化。
- `AlphaMovie.dll` / AMV、透明视频、长流程 A/V drift、用户点击跳过后资源清理仍待后续专项覆盖。

## Verification

```bash
ls -la /Users/xiabin/my_work1/krkrsdl2/external/ffmpeg/install/lib/
cmake --build /Users/xiabin/my_work1/krkrsdl2/build_web -j 8
node /Users/xiabin/my_work1/web-krkr/diag_ffmpeg_avio_probe.js
node /Users/xiabin/my_work1/web-krkr/diag_audio_state.js
node /Users/xiabin/my_work1/web-krkr/diag_video_wait.js
git -C /Users/xiabin/my_work1/krkrsdl2 diff --check
```

## Known Limitations

1. Direct `WaveSoundBuffer` audio 已接入，但主流程原脚本 BGM/SE/voice 自动命中仍未单独证明。
2. `wuvorbis.dll` facade、wait/label/seek/volume/pan/frequency longrun 仍待覆盖。
3. `VideoOverlay` movie A/V 最小闭环和 EXTRA -> movie 手动播放有声路径已接入，但 wait reason、长流程 A/V drift、AlphaMovie/AMV 仍待覆盖。
4. 当前 `.wmv` playback 使用 GPL codec experiment，且真实 movie 音轨依赖 WMAPRO；发布/分发前需要单独确认许可证策略。
5. FFmpeg 兼容性后续按 profile 扩展，不作为当前主线；当前主线见 `WEBGL_RENDERER_PLAN_2026-05-18.md`。
