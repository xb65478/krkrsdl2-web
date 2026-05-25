# FFmpeg Audio / Video Implementation Plan

文档标志位：`FFMPEG_AV_IMPLEMENTATION_PLAN`
文档类型：执行交接计划。
更新日期：2026-05-18

适用范围：

- Engine repo: `/Users/xiabin/my_work1/krkrsdl2`
- Web repo: `/Users/xiabin/my_work1/web-krkr`
- 当前主样本：`DRACU-RIOT!`

## 目标

本计划记录已经完成的 FFmpeg-first Audio / Video 实现路线，以及后续待做的媒体兼容性边界。2026-05-18 起，项目当前主线已切到 WebGL renderer；本文件不再承担下一步排序。

核心路线：

```text
KRKR Storage / XP3
  -> FFmpeg AVIOContext
  -> libavformat demux
  -> libavcodec decode
  -> audio PCM / video frame
  -> KRKR audio/video 状态机
  -> Web Audio / KRKR Layer 输出
```

FFmpeg 是默认媒体解码底座。浏览器原生 `canPlayType()` / `decodeAudioData()` / `HTMLVideoElement` 只能作为对照或降级信息，不作为是否接入 FFmpeg 的决策条件。

SDL3 作为语义参考，不直接整块复制实现。优先参考其状态、事件、wait、stop、complete、loop、position 等行为。

## 非目标

- 不把音视频继续维持为 stub/no-op 假成功。
- 不用浏览器原生 codec 能力替代 FFmpeg 主路线。
- 不整块搬 SDL3 的 SDL_mixer 或本地平台输出层。
- 不在第一阶段追求所有 AlphaMovie / 透明视频画面语义；先保证状态和 wait 可信。
- 不把 KAGParserEx 临时 bridge 继续扩大成长期 media 调度器。

## 参考入口

### krkrsdl2

| 领域 | 文件 |
|---|---|
| Audio core | `src/core/sound/sdl2/WaveImpl.cpp` |
| Video core | `src/core/visual/sdl2/VideoOvlImpl.cpp` |
| Plugin fallback / DLL routing | `src/core/base/sdl2/PluginImpl.cpp` |
| Internal plugin registry | `src/plugins/InternalPlugins.cpp` |
| KAGParserEx bridge risk | `external/KAGParserEx/KAGParserEx.cpp` |
| Build config | `CMakeLists.txt`、`src/config/src_list/kirikirisdl2/sources.txt` |

### web-krkr

| 领域 | 文件 |
|---|---|
| Browser host | `index.html` |
| Audio bridge | `audioutil.js` |
| Video bridge | `videoutil.js` |
| Audio diagnostic | `diag_audio_state.js` |
| Video diagnostic | `diag_video_wait.js` |
| Shared diagnostic helper | `diag_survey_common.js` |

### SDL3 semantic reference

优先查本地：

```text
/Users/xiabin/my_work1/krkrsdl3/cpp/core/media/sound
/Users/xiabin/my_work1/krkrsdl3/cpp/environ/WaveMixer.cpp
/Users/xiabin/my_work1/krkrsdl3/cpp/core/script/tjsNativeVideoOverlay.cpp
/Users/xiabin/my_work1/krkrsdl3/cpp/core/media/movie
/Users/xiabin/my_work1/krkrsdl3/cpp/plugins
```

参考目标是语义表，不是机械移植：

- audio: open / play / stop / pause / loop / position / samplePosition / end;
- video: open / ready / play / pause / stop / timeupdate / complete;
- KAG wait: `wb` / `wm` / `waitvideo` 等待和释放；
- click skip: natural ended 与 user skip 要能区分 reason。

## 阶段 0：基线与 inventory

执行内容：

1. 扫描 DRACU 包体中的 audio/video 文件扩展、magic、大小和路径。
2. 扫描 KAG/TJS 中 audio/video 调用点。
3. 记录当前运行中命中的 media stub/no-op 日志。
4. 建议在 `web-krkr` 新增 `tools/media_inventory.js`，输出 JSON/Markdown artifact。

产物：

```text
web-krkr/diag-results/<run-id>/media-inventory.json
web-krkr/diag-results/<run-id>/media-inventory.md
```

验收：

- inventory 覆盖真实包体 media 文件，不只看扩展名；
- 至少列出样本文件的 magic/header；
- 明确当前命中的 `WaveSoundBuffer` / `VideoOverlay` / `krmovie.dll` / `AlphaMovie.dll` / `wuvorbis.dll` 路径。

## 阶段 1：FFmpeg wasm build skeleton

执行内容：

1. 将 FFmpeg 以静态库形式接入 `krkrsdl2` WASM 构建。
2. 第一版默认 LGPL-only 配置；如果真实 codec 必须 GPL，单独记录并等待确认。
3. `--disable-everything` 后按 inventory 打开所需 demuxer / decoder / parser / protocol。
4. 至少包含：

```text
libavformat
libavcodec
libavutil
libswresample
libswscale
```

验收：

- `cmake --build /Users/xiabin/my_work1/krkrsdl2/build_web -j 8` 通过；
- wasm size 变化可解释；
- 启动基础 gate 不回归；
- 文档中记录 FFmpeg 配置、许可证边界和启用 codec/demuxer。

## 阶段 0/1 修复补充：2026-05-18

Mimo 已提交的阶段 0/1 经过修补后，已作为阶段 2 的提交基座。

阶段 0 当前状态：

- `web-krkr/tools/media_inventory.js` 已改为递归扫描每个 XP3 的解包目录。
- 已覆盖 `.ogg`、`.sli`、`.wmv`、`.amv` 和 video 目录下的 `.png` companion。
- KAG/TJS 调用扫描已改为递归覆盖 `.ks`、`.tjs`、`.ks.scn`，并包含 `sysmovie`、`playvoice`、`openvideo`、`preparevideo`、`insertTag` / `tagHandlers` / `addKagHandler` 等路径。
- 本轮 artifact：`/Users/xiabin/my_work1/web-krkr/diag-results/phase0-inventory-2026-05-18/`。
- 新 inventory 统计：32730 个 media/sidecar/companion 文件，其中 audio 19636、video 14、sidecar 13078、video companion 2、KAG/TJS media call sites 35。

阶段 1 当前状态：

- `external/ffmpeg/install/lib` 下已有 FFmpeg 静态库，`cmake --build /Users/xiabin/my_work1/krkrsdl2/build_web -j 8` 可以通过；这证明静态链接骨架可用。
- `build_ffmpeg_wasm.sh` 已改成 LGPL-first 默认；本轮为真实 DRACU-RIOT! `.wmv` probe 显式执行了 `KRKRSDL2_FFMPEG_ENABLE_GPL_CODECS=1 ./build_ffmpeg_wasm.sh`。
- 当前 FFmpeg 配置已启用 `asf` demuxer、`wmav1/wmav2/wmapro/wmalossless/wmavoice`、`wmv1/wmv2/wmv3/wmv3image`、`vc1`、`msmpeg4v1/v2/v3`；许可证模式为 GPL v2+，提交说明必须写明该边界。
- 当前 `FFmpegAudioDecoder` / `FFmpegVideoDecoder` 已通过 `FFmpegAVIOBridge` 走 KRKR Storage / XP3；普通 `openPath()` 仅保留为测试入口。
- 阶段 1 当时的 `FFmpegAudioDecoder` / `FFmpegVideoDecoder` 只是独立 probe/decode 雏形；后续阶段 3B 已新增 `FFmpegWaveDecoder` 并接入 direct `WaveSoundBuffer` audio；阶段 4 已新增 `FFmpegVideoPlayer` 并接入 `VideoOverlay` / KRKR Layer movie A/V 最小闭环。
- C/C++ 侧 `KRKRSDL2_FFMPEG_ENABLED` 判断已改为 `#if`，避免 `KRKRSDL2_FFMPEG_ENABLED=0` 仍进入 FFmpeg 分支。

阶段 4 完成后需要注意：

1. 继续保留 LGPL-first 默认构建脚本，但当前提交使用了 GPL codec mode 来覆盖真实 `.wmv` 包体，且 `OP_low.wmv` / `mono_loop.wmv` 音轨依赖 WMAPRO；发布/分发前必须重新确认许可证策略。
2. 阶段 2 只证明真实 KRKR Storage / XP3 -> FFmpeg probe 闭环；不要把它写成 runtime audio/video 已播放完成。
3. 阶段 3B 只证明直接 TJS `WaveSoundBuffer` 使用真实 BGM/voice 资源的 audio 契约闭环；游戏主流程原脚本是否自动命中 BGM/SE/voice 仍是待做 / 待优化项。
4. 阶段 4 证明 `VideoOverlay` direct movie A/V 可播放、可截图、可输出 movie audio queue；用户手动验收确认 EXTRA -> movie 第一个解锁 movie 可见、有声、点击可停止返回。不要把它写成 wait reason、AlphaMovie、长流程 A/V drift 已完成。
5. 2026-05-18 起，后续主线切到 WebGL renderer；FFmpeg 剩余项转为待做 / 待优化。

## 后续阶段实现边界：可复用壳与契约，不照搬 native 后端

“照搬”在本项目里要拆成两层看：

- 可复用的是 KRKR 的壳与契约：TJS 可见类、属性、状态字符串、事件、wait 释放时机、loop/position/stop/end 行为。
- 不可照搬的是 native 平台后端：FAudio、OpenAL、DirectShow/krmovie、SDL_mixer、SDL3 平台输出层、Windows message/event queue。

Audio 的可复用壳：

- `WaveSoundBuffer` 是 KRKR audio 契约入口，后续实现必须保留它对脚本暴露的行为。
- 需要对齐的契约包括 `open`、`play`、`pause/resume`、`stop`、`loop`、`position`、`samplePosition`、`status/running/stopped`、end event、label event、volume、pan、frequency。
- FFmpeg 只负责从 KRKR Storage / XP3 中 demux/decode 出 PCM；Web Audio 只负责浏览器最终出声。两者都不应绕过 `WaveSoundBuffer` 的状态与事件语义。
- 不能把现有 FAudio/OpenAL/NullAudioDevice 当成 Web 后端直接搬；Web 侧需要新的 Web Audio 输出设备或桥接层，并由 `WaveSoundBuffer` 驱动。

Video 的可复用壳：

- `VideoOverlay` / `krmovie.dll` 的重点不是某个播放器对象，而是 KRKR 看到的状态和事件。
- 需要对齐的契约包括 `unload/ready/play/pause/stop`、`onStatusChanged`、`onPeriod`、`onFrameUpdate`、frame/time 前进、natural complete、segment loop、layer/mixer 模式下的可见更新。
- FFmpeg 负责 demux/decode video frame，`libswscale` 输出 RGBA 或后续需要的像素格式；KRKR Layer/video 路径负责呈现。
- 不能照搬 Windows `krmovie` / DirectShow / `EC_UPDATE` / `EC_COMPLETE` 后端代码，但要在 WASM 后端产生等价语义：有新帧时触发 frame update，自然结束时触发 complete，用户 stop/skip 时走不同 reason。

SDL3 参考的使用方式：

- SDL3 只作为语义参考：状态转换、wait/end、click-skip、loop、position、错误可见性。
- 不直接复制 SDL3 的 SDL_mixer、平台音频输出、OpenGL/窗口层或本地线程模型。
- 如果 SDL2 当前界面看起来正确，也只能证明图像/脚本主路径接近可用；不能证明 audio/video wait、end、skip、loop 的契约已经成立。

## 阶段 2：KRKR Storage -> FFmpeg AVIO

状态：已完成。

执行内容：

1. 实现自定义 `AVIOContext`：

```text
read  -> KRKR storage / XP3 read
seek  -> KRKR storage seek
close -> release storage handle
```

2. 用 `avformat_open_input` 打开真实 KRKR storage 路径，不把媒体文件复制到普通临时文件。
3. 暴露轻量 probe 日志：format、stream、codec、duration、sample rate、channels、video size、pixel format。

验收：

- 真实包体 audio 文件可以 probe 出 audio stream；
- 真实包体 video 文件如存在，可以 probe 出 video stream；
- read/seek 错误有结构化日志；
- 不破坏 XP3 / Range / savedata 既有路径。

本轮验收 artifact：

- `/tmp/krkr-ffmpeg-avio-probe-summary.json`
- `/Users/xiabin/my_work1/web-krkr/diag-results/ffmpeg-avio-phase2-2026-05-17T17-22-21-683Z/summary.json`

通过证据：

- audio：`http://127.0.0.1:8085/DRACU-RIOT!/bgm.xp3>BGM01.ogg`，format=`ogg`，codec=`vorbis`，duration=`110.652s`，sampleRate=`44100`，channels=`2`，readCalls=`6`，seekCalls=`3`，readErrors=`0`，seekErrors=`0`。
- video：`http://127.0.0.1:8085/DRACU-RIOT!/video.xp3>mono_loop.wmv`，format=`asf`，video codec=`wmv3`，size=`1280x720`，fps=`30`，pixFmt=`yuv420p`，duration=`8.466s`，readCalls=`46`，readErrors=`0`，seekErrors=`0`。

边界：

- 这是 probe / demux 入口完成，不是主流程声音或视频播放完成。
- 直接 `WaveSoundBuffer` audio 输出已在阶段 3B 闭合；`VideoOverlay` direct movie A/V 输出已在阶段 4 闭合。
- 主流程原脚本自动触发 BGM/SE/voice 仍未单独证明。

## 阶段 3：Audio 最小闭环

状态：已完成 direct `WaveSoundBuffer` 3A/3B。

执行内容：

1. 用 FFmpeg wave decoder 通过 KRKR Storage / XP3 AVIO decode audio 到 PCM。
2. 用 `libswresample` 转成 KRKR/OpenAL/WebAudio queue 可消费的 s16 PCM。
3. 接到 WASM/browser audio 输出：

```text
WaveSoundBuffer -> FFmpeg AVIO/decode -> OpenAL/WebAudio streaming queue
```

4. `WaveSoundBuffer` 状态机至少覆盖：

```text
open
play
pause / resume
stop
loop
position
samplePosition
running / stopped
end event
```

已拆分完成：

- 3A：`WaveSoundBuffer.open/play/stop` 不再在 Emscripten 下直接 stub 成功；`FFmpegWaveDecoder` 注册到 `TVPCreateWaveDecoder`，真实 `bgm.xp3>BGM01.ogg` 可进入 FFmpeg decode，PCM 进入 OpenAL/WebAudio queue；wasm 单线程下由 `WaveSoundBuffer::Trigger()` 驱动 audio pump。
- 3B：补 `samplePosition/position` 可前进、pause/resume 可观测、stop 后 stopped、短 voice natural end、短 voice loop 继续 play；`Module.getAudioDiagState()` 暴露 FFmpeg/OpenAL/wasm pump 计数。

验收：

- `diag_audio_state.js` 能看到 FFmpeg wave open/render 成功；
- play 后 `samplePosition` 前进；
- pause 后 `paused=true` 且 drift 有界，resume 后继续前进；
- stop 后 `status=stop`；
- 短 voice natural end 后 `status=stop`；
- loop 短 voice 超过自然时长后仍 `status=play` 且 `looping=true`；
- OpenAL/WebAudio queue 与 wasm pump 均有计数增长；
- 不再通过 stub 假成功判断直接 `WaveSoundBuffer` 音频完成。

本轮验收 artifact：

- `/tmp/krkr-audio-state-summary.json`
- `/Users/xiabin/my_work1/web-krkr/diag-results/audio-phase3-2026-05-18T07-15-30-145Z/summary.json`

通过证据：

- long BGM：`http://127.0.0.1:8085/DRACU-RIOT!/bgm.xp3>BGM01.ogg`，`afterPlay=0`，`afterObserve=143380`，`afterPause=143615`，`afterPauseHold=143615`，`afterResumeObserve=189111`，`pauseDriftLimit=44100`。
- short voice natural end：`http://127.0.0.1:8085/DRACU-RIOT!/voice.xp3>azu/azu205_021.ogg`，约 `0.326s`，观察后 `status=stop`。
- short voice loop：同一 voice，`looping=1`，超过自然时长后仍 `status=play`，且 render/append 计数继续增长。
- diag deltas：`ffmpegWaveOpenCount=1`、`ffmpegWaveRenderCallCount=28`、`ffmpegWaveRenderedSamples=154336`、`openALCreateStreamCount=1`、`openALAppendBufferCount=27`、`openALPlayCount=2`、`wasmPumpTriggerCount=152`、`wasmPumpFillBufferCount=38`。

边界：

- 这是直接 TJS 创建 `WaveSoundBuffer` 的真实资源契约验收，不等于游戏主流程原 KAG/TJS 已自动命中全部 BGM/SE/voice。
- `wuvorbis.dll` 插件 facade、音量/声像/频率长流程、`SetPosition()` 专项 seek、主流程 wait/label 语义仍是后续待做。

## 阶段 4：Video / Movie Audio 最小闭环

状态：已完成 direct `VideoOverlay` movie A/V phase4。

执行内容：

1. 用 FFmpeg demux/decode video frame。
2. 用 `libswscale` 转 RGBA。
3. 将 frame 写回 KRKR video/layer 输出路径。
4. `VideoOverlay` / `krmovie.dll` 状态机至少覆盖：

```text
open
ready
play
time advances
frame update
pause / stop
complete
```

已拆分完成：

- 4A：`VideoOverlay.open/play/stop/pause/rewind` 在 Emscripten + FFmpeg enabled 路径下不再快速 no-op；`FFmpegVideoPlayer` 通过 KRKR Storage / XP3 AVIO 解码真实 `.wmv`，RGBA/BGRA frame 写回 KRKR layer。
- 4B：movie audio 通过独立 audio AVIO / `AVFormatContext` 解码，不与 video demux 互相抢包；PCM 经 `libswresample` 转 s16 stereo 后进入现有 OpenAL/WebAudio `iTVPSoundBuffer` streaming queue。
- 4C：`VideoOverlay` 音频属性和音轨接口转发到 `FFmpegVideoPlayer`：`audioVolume`、`audioBalance`、`numberOfAudioStream`、`enabledAudioStream`、`selectAudioStream`、`disableAudioStream`。
- 4D：`diag_video_wait.js` 升级为 phase4 gate，记录 `movieAudioStreamObserved`、`movieAudioQueued`、`movieAudioOutputStarted`，并保存截图 artifact 供人工或多模态视觉确认。

验收：

- `diag_video_wait.js` 能看到 FFmpeg probe/decode 成功；
- 首帧或中帧可截图；
- play 后 time/frame 前进；
- natural ended 触发 complete；
- KAG `waitvideo` / `wm` 不提前也不卡死。

本轮验收 artifact：

- `/tmp/krkr-video-wait-summary.json`
- `/Users/xiabin/my_work1/web-krkr/diag-results/video-phase4-2026-05-18T07-26-41-001Z/summary.json`
- `/tmp/krkr-oplow-video-audio-summary.json`
- `/Users/xiabin/my_work1/web-krkr/diag-results/video-phase4-oplow-2026-05-18T07-26-41-001Z/summary.json`

通过证据：

- `video.xp3>mono_loop.wmv`：`wmv3` 1280x720 30fps + `wmapro` 44100Hz stereo；`openedRealVideo=true`、`frameAdvanced=true`、`naturalCompleteObserved=true`、`movieAudioQueued=true`、`movieAudioOutputStarted=true`，OpenAL deltas `createStream=1`、`appendBuffer=54`、`appendBytes=442368`、`play=1`。
- `video.xp3>OP_low.wmv`：`wmv3` 640x360 24fps + `wmapro` 44100Hz stereo；`openedRealVideo=true`、`frameAdvanced=true`、`movieAudioQueued=true`、`movieAudioOutputStarted=true`，截图 `/tmp/krkr-oplow-video-audio-frame.png` 已写出。长 OP 本轮通过 `DIAG_VIDEO_EXPECT_NATURAL_COMPLETE=0` 跳过自然结束等待。

边界：

- 这是 direct TJS/diagnostic 创建 `VideoOverlay` 的真实资源契约验收，不等于 EXTRA -> movie 全 UI、click-skip 或所有 KAG wait reason 已闭合。
- `waitvideo` / `wm` 的 natural ended、click-skip、stop/error reason 需要阶段 5 单独验收。
- `AlphaMovie.dll` / AMV、透明视频、长流程 A/V drift 和用户 skip 后清理仍是后续待做。

## 阶段 5：Wait / click skip 语义（待做 / 待优化）

执行内容：

1. 对齐 SDL3 / 游戏脚本中的 wait 语义。
2. 用户点击 skip 时 stop 当前 audio/video wait，并释放 KAG wait。EXTRA -> movie 手动点击停止返回已通过一次实机路径验收；这里要补的是 reason / wait / longrun 结构化验收。
3. 日志区分：

```text
complete reason=ended
complete reason=click-skip
complete reason=stop
complete reason=error
```

4. 不把该逻辑写成具体 logo/video 文件名特化。

验收：

- natural ended 正常释放 wait；
- click-skip 立即释放 wait；
- stop 后状态不漂；
- 后续脚本能继续推进；
- 不破坏 SAVE/LOAD/SYSTEM 菜单点击路径。

## 阶段 6：Stub / fallback 退场

执行内容：

1. 将 media 相关 dangerous no-op 改为真实路径或 fail-visible：

```text
WaveSoundBuffer
wuvorbis.dll
VideoOverlay
krmovie.dll
AlphaMovie.dll 状态面
```

2. 更新 `ENGINE_COMPAT_MATRIX.md` 和 `STUB_HACK_AUDIT.md`。

验收：

- 命中真实 media 调用时不再只注册 stub 然后静默成功；
- 未支持的格式或 API 输出明确 unsupported；
- gate/survey 中能区分 unsupported、decode error、storage error、状态机错误。

## 阶段 7：回归与交付

最低命令：

```bash
cmake --build /Users/xiabin/my_work1/krkrsdl2/build_web -j 8
cp /Users/xiabin/my_work1/krkrsdl2/build_web/krkrsdl2.js /Users/xiabin/my_work1/web-krkr/krkrsdl2.js
cp /Users/xiabin/my_work1/krkrsdl2/build_web/krkrsdl2.wasm /Users/xiabin/my_work1/web-krkr/krkrsdl2.wasm
git -C /Users/xiabin/my_work1/krkrsdl2 diff --check
git -C /Users/xiabin/my_work1/web-krkr diff --check
node /Users/xiabin/my_work1/web-krkr/diag_title_click.js
node /Users/xiabin/my_work1/web-krkr/diag_progress_first_line.js
node /Users/xiabin/my_work1/web-krkr/diag_audio_state.js
node /Users/xiabin/my_work1/web-krkr/diag_video_wait.js
```

涉及菜单或 click skip 时再跑：

```bash
node /Users/xiabin/my_work1/web-krkr/diag_system_menu.js --scenario=save,load,system
node /Users/xiabin/my_work1/web-krkr/diag_system_click.js
```

交付时必须说明：

- FFmpeg 配置和许可证边界；
- inventory 发现的真实格式；
- 已支持的 demuxer / decoder；
- audio 状态机完成度；
- video 状态机完成度；
- click-skip / wait 语义完成度；
- 仍 unsupported 的格式或 API；
- 所有 artifact 路径。

## Codex 验收顺序

DeepSeek 执行后，Codex 按下面顺序验收：

1. 文档与 diff 审核：确认没有引入无记录的 stub/no-op 或样本特化。
2. 构建审核：确认 FFmpeg 静态链接、许可证和 wasm 输出合理。
3. AVIO probe 验收：真实 KRKR storage 文件可被 FFmpeg probe。
4. Audio 验收：`diag_audio_state.js` 的 decode/play/loop/stop/end 状态。
5. Video 验收：`diag_video_wait.js` 的 ready/play/frame/complete 状态。
6. Movie audio 验收：`diag_video_wait.js` 的 `movieAudioStreamObserved`、`movieAudioQueued`、`movieAudioOutputStarted` 状态。
7. Wait/skip 验收：natural ended 和 click-skip 都能释放 wait。
8. 主路径回归：title、first line、system menu 不回退。
9. 文档同步：矩阵、审计、strategy、snapshot 均反映真实完成度。
