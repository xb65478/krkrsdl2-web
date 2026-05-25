# krkrsdl2 引擎侧推进方案

文档标志位：`STRATEGY_ENGINE`
文档类型：旧主线冻结前执行路线，不记录流水账。
权威规则：本文只保留 krkrsdl2 冻结前路线和迁移参考；新主线执行路线以 `/Users/xiabin/my_work1/kirikiroid2-web/PORTING_STRATEGY_WEB.md` 为准；当前冻结事实以 `PROJECT_CONTEXT_ENGINE.md` 为准。
更新日期：2026-05-24

## 2026-05-24 路线冻结

`krkrsdl2 + web-krkr` 不再作为新的 renderer / compatibility feature 实现线。本文以下 WebGL renderer、audio/video、motion/menu/performance 队列均作为旧线参考资料保留，用于迁移经验、回归对照和诊断资产查找。

新主线：

```text
/Users/xiabin/my_work1/kirikiroid2-web
```

第一阶段目标：用 `/Users/xiabin/my_work1/web-krkr/DRACU-RIOT!/data.xp3` 通过 `coi-server.py --xp3` 完成 streamed XP3、sibling manifest、Range/LRU/read hint、存档空间和 logo/title/正文截图闭环。

## 当前阶段判断

DRACU-RIOT! 主样本已经越过“能否启动、能否显示 motion、能否进入正文、能否打开系统菜单、能否播放真实音视频”的早期阶段：

- 自然启动 logo/title motion、标题 Start、正文首句、正文 `mono_loop.psb` 动画、角色立绘均已进入可用里程碑；
- SAVE / LOAD / SYSTEM 菜单、存档读写、配置写回与 IDBFS overlay 已通过既有 survey；
- SYSTEM/config 即时 UI 写回、设置持久化、控件覆盖、持久化后刷新卡死均已闭合；
- motion CPU-IR / visible bridge 当前足以支撑主样本可见路径；
- Audio direct `WaveSoundBuffer` phase3B 已完成；
- VideoOverlay movie A/V phase4 已完成，用户手动验收确认 EXTRA -> movie 可见、有声、点击可停止返回。

冻结前项目主线曾从 motion/menu/performance 收敛切换为：

```text
冻结前旧线主线：WebGL renderer 接入；CPU-IR 保留为第二语义、回归对照和 fallback
```

FFmpeg 兼容性扩展、主流程 audio survey、click-skip/wait reason、AlphaMovie/AMV、QuickMenu 诊断固化、菜单性能、截图长流程、跨包通用化仍保留为迁移参考，不再在 krkrsdl2 旧线继续排期。

## 冻结前主线：WebGL Renderer

目标：把现有 motion/runtime 的 CPU command / renderer IR 消费链扩展出 WebGL 后端，让 motion、PSB 图元和后续 texture-heavy 场景不再长期依赖 CPU raster / canvas copyback。

执行详案见 `WEBGL_RENDERER_PLAN_2026-05-18.md`。后续 DeepSeek 或其它执行者应按该文档分阶段实现；Codex 验收也按该文档的阶段验收点进行。

### CPU-IR 定位

状态：保留为第二语义。

用途：

- 行为对照：WebGL 输出应能和 CPU-IR 的 command / texture / order 摘要对齐；
- fallback：WebGL init 失败、texture missing、unsupported command 时回退；
- 回归护栏：标题 motion、正文 `mono_loop`、角色立绘、movie 路径不能因 WebGL 接入而回退；
- 诊断：继续输出 command count、sourceKey、draw rect、opacity、z/order、blend/mask/stencil 标记、texture-missing。

### WebGL 第一刀

第一刀只做可开关、可回退、可截图验收的最小后端：

1. 增加 `motion_renderer_backend=webgl`，并保留 `cpu-ir` / `static-psb` 显式选择。
2. 建 WebGL texture registry，打通 sourceKey -> decoded bitmap / layer source -> texture。
3. 消费 CPU-IR 已能表达的安全子集：textured quad、opacity、basic alpha blend、z/order stable sorting、viewport/scaling。
4. 产出 CPU-IR 与 WebGL 双截图，并保存 texture-missing / unsupported-command 摘要。
5. WebGL 失败必须 fail-visible 或回落 CPU-IR，不允许静默黑屏。

主要入口：

- `src/plugins/emoteplayer/emoteplayer_stub.cpp`
- `src/plugins/psb/psb_gfx_loader.cpp`
- `external/krkrz/visual/LayerIntf.cpp`
- `external/krkrz/visual/LayerManager.cpp`
- `web-krkr/index.html`
- 后续新增或扩展 `web-krkr/diag_webgl_renderer.js`

### WebGL 后续队列

WebGL 第一刀通过后，再按真实命中推进：

1. type=1 / bp 资源闭环；
2. mask / stencil；
3. blend modes；
4. z/order 与多层排序；
5. dynamic texture update / cache / LRU；
6. timeline / physics 对 draw command 的影响；
7. 视频帧或 AlphaMovie 进入 WebGL texture 的可能性。

## 已完成主线：Audio / Video 真实状态机

目标：让商业包中的 BGM / SE / voice / movie wait 不再依赖 stub 或静默 no-op。当前路线采用 **FFmpeg-first**：FFmpeg 负责 demux/decode，Web Audio / KRKR Layer 只负责最终输出设备和浏览器桥接。

执行详案和兼容性记录见 `FFMPEG_AV_IMPLEMENTATION_PLAN_2026-05-17.md` 与 `FFMPEG_INTEGRATION.md`。该方向不再是当前下一步。

### Audio 第一阶段

状态：direct `WaveSoundBuffer` 阶段 3A/3B 已完成。

已完成：

- `WaveSoundBuffer` 在 WASM 端接 FFmpeg 解码后的 PCM；
- OpenAL/WebAudio queue 作为最终出声设备；
- KRKR 侧可观察语义已覆盖 `status`、`paused`、`loop`、`position`、`samplePosition`、natural end、stop；
- 真实包体 `bgm.xp3>BGM01.ogg` 和 `voice.xp3>azu/azu205_021.ogg` 已通过 `diag_audio_state.js` phase3B；
- 浏览器原生 codec 能力只保留为对照和降级信息，不参与路线决策。

本轮 artifact：

```text
/Users/xiabin/my_work1/web-krkr/diag-results/audio-phase3-2026-05-18T07-15-30-145Z/summary.json
```

边界：

- 这证明 direct TJS `WaveSoundBuffer` 真实资源契约，不证明游戏原脚本主流程已自动命中全部 BGM/SE/voice。
- `wuvorbis.dll` facade、原脚本 BGM/SE/voice 路由、长流程音量/声像/seek/wait/label 仍是待做。

### Video / Movie Audio 第一阶段

状态：direct `VideoOverlay` movie A/V phase4 已完成，并已通过用户手动 EXTRA -> movie 验收。

已完成：

- `VideoOverlay` 在 WASM + FFmpeg enabled 路径下不再快速 no-op；
- `FFmpegVideoPlayer` 通过 KRKR Storage / XP3 AVIO 解码真实 `.wmv`；
- video frame 经 `libswscale` 转 BGRA 并写回 KRKR Layer；
- movie audio 走独立 audio AVIO / FFmpeg decode / `libswresample`，再进入 OpenAL/WebAudio `iTVPSoundBuffer` queue；
- `audioVolume`、`audioBalance`、`numberOfAudioStream`、`enabledAudioStream`、`selectAudioStream`、`disableAudioStream` 已转发；
- `diag_video_wait.js` 已通过 `mono_loop.wmv` 和 `OP_low.wmv`，并产出截图 artifact；
- 用户手动验收确认：进入首页后点击 EXTRA -> movie -> 第一个解锁 movie，视频可见且有声音，点击后会停止并返回游戏界面。

最小验收：

```text
open -> ready -> play -> time advances -> complete event
movie audio stream -> PCM queued -> output started
stop/pause state observable
waitvideo/wm 不提前也不卡死
```

本轮 artifact：

```text
/Users/xiabin/my_work1/web-krkr/diag-results/video-phase4-2026-05-18T07-26-41-001Z/summary.json
/Users/xiabin/my_work1/web-krkr/diag-results/video-phase4-oplow-2026-05-18T07-26-41-001Z/summary.json
```

边界：

- `OP_low.wmv` 已证明长 OP 类资源可以 video + WMAPRO audio 输出，但自然结束等待本轮没有等 98 秒完整跑完。
- KAG `wb` / `waitvideo` / `wm` 的 reason 区分、stop/error 释放、AlphaMovie/AMV、格式 profile 扩展和 longrun A/V drift 仍是待做 / 待优化项。

主要入口：

- `src/core/visual/sdl2/FFmpegVideoPlayer.cpp`
- `src/core/visual/sdl2/FFmpegVideoPlayer.h`
- `src/core/visual/sdl2/VideoOvlImpl.cpp`
- `src/core/base/sdl2/PluginImpl.cpp`
- `external/KAGParserEx/KAGParserEx.cpp`（仅用于识别现有临时 wait bridge 的影响，不继续扩大 parser bridge）
- `web-krkr/videoutil.js`
- `web-krkr/diag_video_wait.js`

### FFmpeg 已完成与后续状态

已完成：

- FFmpeg wasm 静态链接方案建立，当前真实 `.wmv` probe 使用 GPL codec experiment，许可证边界见 `FFMPEG_INTEGRATION.md`。
- media inventory 已产出 `/Users/xiabin/my_work1/web-krkr/diag-results/phase0-inventory-2026-05-18/`。
- KRKR Storage / XP3 -> FFmpeg `AVIOContext` 已通过真实 `bgm.xp3>BGM01.ogg` 与 `video.xp3>mono_loop.wmv` probe，artifact 见 `/Users/xiabin/my_work1/web-krkr/diag-results/ffmpeg-avio-phase2-2026-05-17T17-22-21-683Z/summary.json`。
- Audio 阶段 3A/3B 已完成 direct `WaveSoundBuffer` -> FFmpeg decode PCM -> OpenAL/WebAudio queue -> KRKR 状态可观测，artifact 见 `/Users/xiabin/my_work1/web-krkr/diag-results/audio-phase3-2026-05-18T07-15-30-145Z/summary.json`。
- Video 阶段 4 已完成 direct `VideoOverlay` -> FFmpeg decode frame -> KRKR Layer 输出，并补齐 movie WMAPRO audio -> OpenAL/WebAudio queue；artifact 见 `/Users/xiabin/my_work1/web-krkr/diag-results/video-phase4-2026-05-18T07-26-41-001Z/summary.json` 和 `/Users/xiabin/my_work1/web-krkr/diag-results/video-phase4-oplow-2026-05-18T07-26-41-001Z/summary.json`。

转入待做 / 待优化：

1. 主流程 audio 命中 survey：确认原游戏 KAG/TJS 自然触发 BGM/SE/voice 时走同一 `WaveSoundBuffer`/FFmpeg 路径，而不是只靠 direct probe。
2. media wait reason：区分 natural ended、click-skip、stop、error，并确保释放 wait。
3. longrun A/V 稳定性：音画是否明显漂移、停止后资源是否清理、后续脚本是否稳定继续。
4. AlphaMovie/AMV 与透明视频语义。
5. FFmpeg wasm compatibility profile：按新包体扩展 demuxer/decoder/parser，不盲目追全量 FFmpeg。

## 待做 / 待优化项

这些内容仍重要，但不是当前 WebGL 下一步。

### Motion / Emote 通用化

状态：主样本可见播放路径已可用，通用语义仍未完成。

待做：

- 退场 P0-004：`KAGParserEx.cpp` 不应长期承载 motion/video wait bridge；
- logo motion click-skip：补回 waitLayerMotion/click-to-stop 语义，但不要写成 logo 资源名特化；
- QuickMenu / MSGWIN：若手动可用而 `diag_system_click.js` 红，优先校准诊断和截图 artifact；
- WebGL 第一刀之外的 renderMethod 残项：真实 type=1/bp、source/texture alias、stencil、blend、z/order；
- P4 timeline、P5 renderMethod/draw pipeline、P6 physics/wind/outerForce、P7 serialize/cache/lifecycle；
- 跨 PSB attach runtime 变量/timeline 传播。

边界：

- 当前要接入 WebGL，但不整块搬 SDL3 GL/FBO/tessellation；
- CPU-IR 已足够支撑主样本路径，后续作为第二语义、fallback 和 WebGL 对照；
- 静态 PSB compositor 只保留为显式诊断或非 motion fallback。

### 性能与线程模型

状态：默认日志降噪、layout merge 本地索引、probe resource 轻量化、LOAD input-ready gap 已推进。

待优化：

- `config.psb` / `save_ui.psb` 是否仍有重复冷 parse、重复 `TVPLoadGraphic`；
- slot 缩略图、目录扫描、IDBFS 写入是否还有卡点；
- motion progress/draw 主线程压力；
- XP3 range I/O 异步化、prefetch、LRU cache；
- canvas copyback 升级到 OffscreenCanvas / ImageBitmap，`putImageData` 只保留降级路径。

触发条件：

- 页面打开或点击可交互空档仍明显超过可接受阈值；
- 长流程出现掉帧、卡死或内存膨胀；
- audio/video 实现后暴露新的 I/O 或线程瓶颈。

### 视觉验收与长流程

由 `web-krkr` 持有。引擎侧要求继续提供状态、日志和诊断表面。

待做：

- logo/title/mono_loop/character/SAVE/LOAD/SYSTEM/QuickMenu 截图 artifact；
- 5-10 分钟 longrun；
- audio/video 状态和可听/可见结果进入 longrun 摘要；
- 结构日志不能单独替代截图或人工/多模态判断。

### 去目标游戏特例

待做：

- `web-krkr/index.html` 启动入口改 manifest / URL 参数，diag 显式传入 game root；
- `-krkr-dracu-bringup` 拆成可观测、可关闭、可删除的独立开关；
- `PluginImpl.cpp` fallback 分级：internal hit / known-safe stub / dangerous no-op / unknown ignored；
- 加入至少 1-2 款其它商业 KRKR 包交叉验证。

## 最低保护闸门

引擎侧较大改动至少保持：

```bash
cmake --build /Users/xiabin/my_work1/krkrsdl2/build_web -j 8
git -C /Users/xiabin/my_work1/krkrsdl2 diff --check
node /Users/xiabin/my_work1/web-krkr/diag_visual_capture.js
node /Users/xiabin/my_work1/web-krkr/diag_title_click.js
node /Users/xiabin/my_work1/web-krkr/diag_progress_first_line.js
```

涉及 audio/video 或 FFmpeg 回归时再跑：

```bash
node /Users/xiabin/my_work1/web-krkr/diag_audio_state.js
node /Users/xiabin/my_work1/web-krkr/diag_video_wait.js
```

涉及 QuickMenu / 菜单 / 存档时再跑：

```bash
node /Users/xiabin/my_work1/web-krkr/diag_system_click.js
node /Users/xiabin/my_work1/web-krkr/diag_system_menu.js --scenario=save,load,system
node /Users/xiabin/my_work1/web-krkr/diag_suite.js survey --fail-fast
```

当前 `diag_system_click.js` 若与手动验收冲突，先校准诊断和截图口径；任何触及 MSGWIN / QuickMenu / hit-test 的 runtime 改动仍必须正面回归。

## 不要做的事

1. 不要把 CPU-IR 删除或贬成无用旧路径；它是 WebGL 的第二语义、fallback 和回归对照。
2. 不要整块搬 krkrsdl3 的 OpenGL/FBO/tessellation renderer；SDL3 renderer 只能作为语义参考，WebGL2 后端需要适配浏览器能力。
3. 不要把 WebGL 失败写成 silent black screen；必须 fail-visible 或回落 CPU-IR。
4. 不要把 audio/video 的 stub 假成功当作真实状态机。
5. 不要把浏览器原生 codec 能力当作是否接 FFmpeg 的决策条件；FFmpeg 已经是媒体底座，后续只按兼容性 profile 扩展。
6. 不要为了 motion 可见性在静态 PSB compositor / fallback 路径里继续堆补丁。
7. 不要在脚本加载期继续扩大 TJS 改写；现有注入必须有 debug flag 和退场条件。
8. 不要把 `-krkr-dracu-bringup` 当作长期容器。

## 关键代码入口

| 类别 | 关键文件 |
|---|---|
| 冻结前 WebGL renderer | `src/plugins/emoteplayer/emoteplayer_stub.cpp`、`src/plugins/psb/psb_gfx_loader.cpp`、`external/krkrz/visual/LayerIntf.cpp`、`external/krkrz/visual/LayerManager.cpp`、`web-krkr/index.html` |
| 已完成 / 待回归 audio/video | `src/core/sound/sdl2/WaveImpl.cpp`、`src/core/visual/sdl2/VideoOvlImpl.cpp`、`src/core/base/sdl2/PluginImpl.cpp` |
| motion 待做 | `src/plugins/emoteplayer/emoteplayer_stub.cpp`、`src/plugins/psb/psb_gfx_loader.cpp`、`external/KAGParserEx/KAGParserEx.cpp`、`external/krkrz/base/ScriptMgnIntf.cpp`、`external/krkrz/visual/LayerIntf.cpp`、`external/krkrz/visual/LayerManager.cpp` |
| performance 待优化 | `external/krkrz/visual/LayerManager.cpp`、`src/core/base/sdl2/StorageImpl.cpp`、`external/krkrz/base/XP3Archive.cpp`、`web-krkr/index.html` |
| generalization 待做 | `src/core/base/sdl2/PluginImpl.cpp`、`external/krkrz/base/ScriptMgnIntf.cpp`、`web-krkr/index.html` |

## 必读文档

- `/Users/xiabin/my_work1/krkrsdl2/PROJECT_CONTEXT_ENGINE.md` — 当前事实
- `/Users/xiabin/my_work1/krkrsdl2/WEBGL_RENDERER_PLAN_2026-05-18.md` — WebGL renderer 执行计划
- `/Users/xiabin/my_work1/krkrsdl2/FFMPEG_INTEGRATION.md` — FFmpeg 集成与兼容性记录
- `/Users/xiabin/my_work1/krkrsdl2/ENGINE_COMPAT_MATRIX.md` — 插件/API 能力矩阵
- `/Users/xiabin/my_work1/krkrsdl2/STUB_HACK_AUDIT.md` — stub/hack 风险
- `/Users/xiabin/my_work1/krkrsdl2/KRKRSDL3_REFERENCE.md` — 外部参考索引
- `/Users/xiabin/my_work1/web-krkr/PORTING_STRATEGY_WEB.md` — Web 验收策略
