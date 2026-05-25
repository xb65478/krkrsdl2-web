# krkrsdl2 stub / hack audit

文档标志位：`STUB_HACK_AUDIT`
文档类型：stub、hack、fallback、silent no-op 审计表。
权威规则：本文只回答"当前哪些临时实现有风险、能留到什么时候、退场条件是什么"；能力覆盖见 `ENGINE_COMPAT_MATRIX.md`；路线见 `PORTING_STRATEGY_ENGINE.md`。
更新日期：2026-05-18

## 分级规则

| 等级 | 含义 | 发布要求 |
|---|---|---|
| P0 | 改变运行时语义，且带目标游戏硬编码、脚本注入或点击/action 强绑定 | 默认运行路径必须移除，或默认关闭并受显式 bring-up flag 控制 |
| P1 | 静默 no-op、假成功、假空结果，可能让脚本继续但效果缺失 | 必须可观测，关键 API 退化要 fail-visible |
| P2 | 部分实现或兼容 fallback，能支撑阶段性 gate，但语义不完整 | 可保留，但要有验收入口和退场路径 |
| P3 | 诊断脚本、debug-only 探针或测试坐标硬编码 | 可保留在测试路径，不能进入 runtime 语义 |

## 当前总览

motion 已跨过一个可用里程碑：logo/title motion 可见，正文 `mono_loop` 栏杆循环已手动验收通过，正文角色立绘可见，KAGParserEx native 集成可推进到序章。当前最危险的残留不再是"motion 完全不可见"，而是三类长期债：

- P0 级脚本/KAG bridge 仍在默认 bring-up 路径里改变语义；
- P1 级插件 fallback 仍会把 DLL 加载成功伪装成能力完成；
- P2 级 renderer / media / storage 仍有 partial 语义，必须由 web 侧截图验收和状态 artifact 看住。

## P0：必须退场的运行时 hack

| ID | 位置 | 当前行为 | 风险 | 保留条件 | 退场条件 |
|---|---|---|---|---|---|
| P0-001 | `external/krkrz/base/ScriptMgnIntf.cpp` | 脚本加载期改写/注入 `Initialize.tjs`、`animkaglayer.tjs`、`kagenvplayer.tjs`、`system.tjs`，补 motion manager、诊断日志、部分 KAG 环境兼容；SAVE dry-run 仅诊断开关使用。 | 引擎脚本加载器仍能成为目标游戏补丁器，掩盖真实 KAG/SystemAction/motion button 语义。 | DRACU bring-up、结构化诊断、SAVE dry-run survey。 | 注入改 debug flag；motion/QuickMenu/SystemAction 由原脚本和插件语义自然工作。 |
| P0-002 | `external/krkrz/visual/LayerManager.cpp` | 已退场：`tTVPMainPsbButtonAction` / `TVPTryMainPsbButtonAction` 已删除。 | — | — | 保持删除；旧桥回潮时 `diag_system_click.js` 应失败。 |
| P0-003 | `src/plugins/emoteplayer/emoteplayer_stub.cpp` | title/main 手写 fallback 表已删除；仍有 DRACU logo fallback 诊断输出 `[HACK-P0] id=P0-003 feature=logo-fallback`。 | logo fallback 是目标游戏路径；layout/hit 仍偏静态矩形，不等于真实 mesh/alpha/timeline hit shape。 | 仅限 DRACU bring-up 和 motion 回归排查。 | 补真实 hit shape、截图验收和跨包路径后删除 logo fallback。 |
| P0-004 | `external/KAGParserEx/KAGParserEx.cpp` | KAGParserEx 在 Emscripten 路径识别 `motionload` / `[ev storage=*.psb]` / `[waitmovie]`，转发到 `__krkrMotionEventBridge`，`waitmovie` 期间合成短 wait。 | Parser 知道 motion/logo 具体实现，层次反转；真实 motion/video wait 状态可能被跳过。 | 本轮为修复 KAGParserEx 集成后 logo/title 回退而临时保留。 | motion 待做项：KAGParserEx 只产 raw tag，motion/video/logo wait 由真实 plugin 处理。 |
| P0-005 | `web-krkr/index.html` | 宿主默认硬编码 DRACU-RIOT! 数据路径。 | 宿主不可复用，跨包验证困难。 | 当前主样本验证。 | manifest / URL 参数入口，diag 显式传入 game root。 |
| P0-006 | `external/krkrz/visual/LayerManager.cpp` | 已退场：QuickMenu 匿名子层 reroute 已删除。 | — | — | 保持删除；旧 reroute 回潮时 gate 应失败。 |

## P1：静默 no-op 或假成功

| ID | 位置 | 当前行为 | 风险 | 退场条件 |
|---|---|---|---|---|
| P1-001 | `src/core/base/sdl2/PluginImpl.cpp` | internal registry miss 后落到 TJS stub；未知 DLL 可被 ignored。 | "Registered stub" 易被误读成能力完成。 | fallback 输出结构化等级：internal hit / known-safe stub / dangerous no-op / unknown ignored；危险 no-op gate 标红。 |
| P1-002 | `motionplayer.dll` TJS fallback | 若 internal native 未命中，会注册假 `Motion.ResourceManager` / `Motion.Player`。 | motion 退化成假 runtime，wait/draw/hit-test 全不可信。 | `motionplayer.dll` native miss 必须红灯，不可静默退化。 |
| P1-003 | `AlphaMovie.dll` / `krmovie.dll` / `VideoOverlay` | `VideoOverlay` WASM 路径已从快速 no-op 升级为 partial native：`FFmpegVideoPlayer` 可解码真实 `.wmv`、写回 KRKR Layer，并将 WMAPRO movie audio 送入 OpenAL/WebAudio queue。用户手动验收确认 EXTRA -> movie 第一个解锁 movie 可见、有声、点击可停止返回。`krmovie.dll` facade 与 `AlphaMovie.dll` 仍未完整退场。 | direct movie A/V 和实际 UI 入口已可信，但脚本 wait/end/skip reason、AlphaMovie/AMV、longrun 仍可能提前、卡死或清理不完整。 | 保留 `diag_video_wait.js` phase4 回归；补 natural ended/click-skip/stop/error wait reason、AlphaMovie/AMV 命中处理和 longrun。 |
| P1-004 | `csvParser.dll` / `fstat.dll` fallback | fallback 可能返回 true 或空数据。 | internal miss 时脚本拿到假空结果。 | native registry miss 不能静默，diag summary 标红。 |
| P1-005 | `layerExDraw` / `layerExSave` / `layerExImage` / `layerExRaster` fallback | 部分绘图/图像函数为空或返回假成功。 | UI 或文字绘制静默缺失。 | Layer attach 方法列出 native/no-op，禁止覆盖已有 native 方法，按调用点补实现。 |
| P1-006 | vendor/tool DLL fallback | `extNagano`、`kagexopt`、`multiimage`、`PackinOne`、`yuzuex`、`wuvorbis` 等多为 no-op 或只日志。 | 当前样本通过不代表长流程/跨包可用。 | 静态扫描 + 跨包验证；命中后补 API 或显式失败。 |
| P1-007 | `src/core/sound/sdl2/WaveImpl.cpp`、`src/core/sound/sdl2/FFmpegWaveDecoder.cpp` | 已从 Emscripten audio stub 升级为 partial native：direct `WaveSoundBuffer` 真实 BGM/voice 资源 phase3B 通过，position/pause/resume/stop/natural end/loop 可观测。 | 仍不能把 direct probe 等同于主流程完成；原游戏脚本 BGM/SE/voice 自动命中、`wuvorbis.dll` facade、wait/label/seek/volume/longrun 仍可能缺口。 | 扩主流程 audio survey，确认 KAG/TJS 自然命中同一 FFmpeg/WaveSoundBuffer 链；再按命中点补 facade 与长流程语义。 |
| P1-008 | `web-krkr/audioutil.js` | 停止音轨时清空 `src`。 | 可能绕开真实停止/结束事件。 | audio 里程碑完成后决定是否保留为宿主资源释放路径。 |
| P1-009 | `external/KAGParserEx/KAGParserEx.cpp` | 已从 no-op/缺失升级为 native 集成。 | 当前无本体风险；P0-004 bridge 单独审计。 | 保持 native，后续只查长流程语义差异。 |

## P2：可阶段性保留的 partial fallback

| ID | 位置 | 当前行为 | 风险 | 下一步 |
|---|---|---|---|---|
| P2-001 | `src/plugins/emoteplayer/emoteplayer_stub.cpp` | native partial Motion runtime；默认 `cpu-ir` draw；logo/title/mono_loop/character 当前可见。显式 `speed=1.0` 当前按正常倍率映射到内部默认 speed ratio。 | 不是完整 emote renderer；真实 bp/type=1、stencil、blend、physics、timeline 多 motion 叠加仍不足。 | QuickMenu / MSGWIN 回归 + 截图验收 + renderer 残项闭环。 |
| P2-002 | `src/plugins/psb/psb_gfx_loader.cpp` | PSB static compositor / fallback 仍存在。 | 非 motion 图像有用，但不能作为 motion 动画过渡态。 | 保留为显式诊断和非 motion fallback；motion 默认不靠它验收。 |
| P2-003 | `src/core/visual/sdl2/WindowImpl.cpp` | `Window.motion_manager` 映射全局对象；`skipToSync()` 直接成功。 | conductor/motion 同步语义未完全验证。 | 与 motion 状态机和 wait gate 一起收敛。 |
| P2-004 | `web-krkr/index.html` fallback renderer | SDL2 EM_ASM path 失效时 JS `putImageData`。 | 可救首帧但掩盖 SDL canvas 链路退化。 | OffscreenCanvas/ImageBitmap 后仅保留降级路径。 |
| P2-005 | 低优先插件 | `addFont`、`fftgraph`、`win32dialog` 等 partial/no-op。 | 当前不是 blocker。 | 命中调用点再升级。 |

## P3：允许的诊断硬编码

| ID | 位置 | 当前行为 | 保留条件 |
|---|---|---|---|
| P3-001 | engine debug logs | KAG tag、TJS load/snippet、input layer、motion trace 日志。 | 受 loglevel/debug flag 控制，不回到默认 firehose。 |
| P3-002 | `web-krkr/diag_*.js` | 诊断脚本硬编码点击坐标、场景推进点、截图采集位置。 | 只作为验收/诊断路径，不进入 runtime。 |
| P3-003 | `web-krkr/index.html` click trace | click_trace 模式手写标题按钮框。 | 仅 debug 开关启用。 |

## 当前主线与待做队列

当前主线：

1. WebGL renderer 接入；CPU-IR 保留为第二语义、fallback 和回归对照。
2. 第一刀打通 sourceKey -> texture registry -> ordered draw pass。
3. 产出 CPU-IR / WebGL 双截图、texture-missing、unsupported-command 和 fallback reason artifact。

Audio / Video 已完成里程碑：

1. 已完成 FFmpeg wasm 静态链接、许可证边界记录、media inventory、KRKR Storage / XP3 -> FFmpeg `AVIOContext`。
2. 已完成 direct `WaveSoundBuffer` / FFmpeg decode / OpenAL-WebAudio queue phase3B，避免 direct BGM/voice 靠 stub 假成功。
3. 已完成 direct `VideoOverlay` / FFmpeg decode / KRKR Layer 输出 / movie WMAPRO audio queue phase4，避免 direct movie 靠 no-op 假成功。
4. 用户手动验收确认 EXTRA -> movie 第一个解锁 movie 可见、有声、点击可停止返回。

待做 / 待优化项：

- 主流程 audio 命中 survey，确认原游戏 KAG/TJS 自然触发 BGM/SE/voice 时也走真实 audio 链。
- media natural ended / click-skip / stop/error 的 wait reason 和 longrun。
- AlphaMovie/AMV 与 FFmpeg 跨包 compatibility profile。
- QuickMenu / MSGWIN 回归固化：截图 artifact 和 `MotionButton -> SystemAction` 六按钮路径。
- motion 截图验收：把 logo/title/mono_loop/character 的手动验收转成截图和状态 artifact。
- P0-004 退场：KAGParserEx motion bridge 下沉到真实 plugin/脚本语义。
- P1-001 退场：Plugin fallback 分级并 fail-visible。
- P0-005 退场：web 宿主 manifest / URL 参数化。

## 发布前硬门槛

- P0 项不存在于默认运行路径，或默认关闭并明确标记 bring-up mode；
- P1 项命中时可观测，关键 DLL/API 不允许静默 no-op；
- 高风险 DLL 在 `ENGINE_COMPAT_MATRIX.md` 中有验收入口或明确延期理由；
- web 宿主不再硬编码单一游戏路径；
- 至少两款商业 KRKR 包能跑到正文 + 菜单，并保留截图验收和状态 artifact。
