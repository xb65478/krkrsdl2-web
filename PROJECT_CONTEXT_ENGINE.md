# krkrsdl2 引擎侧当前状态

文档标志位：`CURRENT_SNAPSHOT_ENGINE`
文档类型：旧主线冻结事实快照，不是未来路线。
权威规则：本文只记录 krkrsdl2 冻结前"能用什么 / 坏什么 / 关键代码位置"；新主线推进见 `/Users/xiabin/my_work1/kirikiroid2-web/PROJECT_CONTEXT_WEB.md` 与 `/Users/xiabin/my_work1/kirikiroid2-web/PORTING_STRATEGY_WEB.md`。
更新日期：2026-05-24

## 2026-05-24 主线冻结

`/Users/xiabin/my_work1/krkrsdl2` 从 2026-05-24 起冻结为参考线，不再继续承担新的 renderer / compatibility feature 实现。后续兼容性推进主线切到 `/Users/xiabin/my_work1/kirikiroid2-web`。

本仓库仍保留以下迁移资产：

- HTTP/HTTPS storage media、`tTVPHTTPRangeStream`、HEAD/Range/list 桥接、SharedArrayBuffer + worker/main-thread 同步读取；
- XP3 read hint、JS 64MB Range LRU、C++ XP3 segment cache、sequential read 识别和性能诊断；
- sibling archive AutoPath，例如 `data.xp3>fgimage/直太/` 映射到 `fgimage.xp3>直太/`；
- read-only seed + writable browser overlay、存档/读档/配置写回 survey；
- click-first/mobile-first 浏览器输入契约和截图优先视觉验收契约。

冻结原因：旧线后续主要成本集中在 partial motion renderer、CPU-IR/WebGL 接入、stub/hack 退场和目标游戏特化；kirikiroid2-web 已具备 cocos2d-x/WebGL 显示链、MotionPlayer/EmotePlayer 逆向对齐和 differential/oracle 体系，作为新主线收益更高。

## 本仓库职责

`/Users/xiabin/my_work1/krkrsdl2` 是引擎源码仓库：

- 编译 WASM 版 `krkrsdl2`；
- 提供 SDL/KRKR 到浏览器 Canvas 的像素输出链路；
- 提供 TJS / Layer / Storage / plugin 等运行时兼容补丁；
- 提供 `.psb` 图像、PSB storage、motion/emote runtime；
- 输出 `krkrsdl2.js` / `krkrsdl2.wasm` 给 `web-krkr` 加载。

不负责浏览器宿主页面、诊断脚本、截图验收和测试结果归档；这些由 `/Users/xiabin/my_work1/web-krkr` 持有。

## 冻结前里程碑

- 自然启动两段 logo motion（`yuzulogo.psb` / `m2logo.psb`）可见并放行到标题页；本轮已修掉播放前后混入静态错误帧的问题。
- 标题页 motion 图元可自主出现，`Start` 可命中并进入 `start.ks` / `*envplay`。
- 正文主流程 `mono_loop.psb` 栏杆循环动画已手动验收通过。`Motion.Player.speed=1.0` 已按正常播放倍率解释，避免把显式 speed 当成内部分母导致 20 倍级过快播放。
- 正文角色立绘可见。2026-05-13 修复 archive subfolder AutoPath 后，`data.xp3>fgimage/直太/` 能映射到同级 `fgimage.xp3>直太/`，`直太a_info.txt` 与 `直太a_*` 图块可加载。
- KAGParserEx 已静态集成并替换原 KAGParser，TJS class 名仍为 `"KAGParser"`。
- 可推进到序章 `★プロローグa（始まり）.ks*prologue_A`，包括第 49 步对白 `いやー、楽しみだなー`。
- SAVE / LOAD / SYSTEM 三菜单可开闭；单槽、覆盖、双槽隔离、Q-SAVE/Q-LOAD、清除/删除、SYSTEM toggle 写 `.cfu` 等 survey 已通过。
- `?loglevel=1|2|3` 控制日志量；默认 L1 已从历史 firehose 降到可读规模。
- Audio / Video 已跨过冻结前可玩性里程碑：direct `WaveSoundBuffer` audio 阶段 3B 已完成，`VideoOverlay` movie A/V 阶段 4 已完成，用户手动验收确认 EXTRA -> movie 可播放画面、有声音，点击可停止并返回游戏界面。冻结前推进主线曾切到 WebGL renderer 接入；CPU-IR 保留为第二语义、回归对照和 fallback。FFmpeg 兼容性扩展、主流程 audio survey、wait reason、AlphaMovie/AMV 与 longrun 均作为迁移参考。

## 冻结前边界

| 领域 | 当前状态 | 后续风险 |
|---|---|---|
| Motion renderer | 默认可用语义是 `cpu-ir`；motion 资源不能再依赖静态 `loaded-layer-image` 作为过渡态。`static-psb` 只应作为显式诊断 backend / 非 motion fallback。标题 motion、正文 `mono_loop` 循环、正文角色立绘冻结前均可见。冻结前下一步曾以 WebGL renderer 为主线，CPU-IR 作为第二语义保留，用于对照、fallback 和回归。 | WebGL 需要补齐 sourceKey -> texture 映射、texture lifecycle、mask/stencil/blend/z/order、type=1/bp、截图对照和回退策略；physics、timeline 多 motion 叠加均作为迁移参考。 |
| 标题/正文视觉 | logo、标题 motion、正文角色立绘已进入可用里程碑。 | 需要把手动验收固化为截图 artifact，避免只靠结构日志判绿。 |
| QuickMenu / MSGWIN | 历史上曾命中过 `QuickMenu -> MotionButton -> SystemAction`；2026-05-13 复跑 `diag_system_click.js` 当前红，底栏截图和 action path 需单独回归。 | MSGWIN/QuickMenu 视觉截图与 6 按钮 action path 要作为后续独立 gate。 |
| Web input | Emscripten/browser build now treats input as click-first: SDL mouse motion updates last coordinates for diagnostics but is not forwarded as KRKR `onMouseMove`; click down/up still dispatch normally, and mouse-up clears hover state instead of rechecking under-cursor hover. | Desktop-style hover effects are intentionally not a wasm target; future menu fixes should not restore continuous hover/mousemove on the default browser path. |
| Audio / Video | Audio direct `WaveSoundBuffer` 已完成 phase3B：真实 `bgm.xp3>BGM01.ogg` 与 `voice.xp3>azu/azu205_021.ogg` 可经 FFmpeg wave decode、OpenAL/WebAudio queue 出声并提供 position/pause/resume/stop/natural end/loop 状态。`VideoOverlay` movie A/V phase4 已通过 `mono_loop.wmv` 与 `OP_low.wmv` diagnostics；用户手动验收确认 EXTRA -> movie 第一个解锁 movie 可见、有声、点击可停止返回。 | 主流程原脚本 BGM/SE/voice 命中、`wuvorbis.dll` facade、click-skip/wait reason、AlphaMovie/AMV、longrun A/V drift 仍待验证，但不再作为 krkrsdl2 当前主线。 |
| 性能 | 当前能跑样本路径。 | motion progress/draw、XP3 range I/O、canvas copyback 仍有主线程压力。 |
| 通用化 | DRACU-RIOT! 是主样本。 | 宿主入口、bring-up flag、插件 no-op 仍有目标游戏倾向，需要跨包验证。 |

冻结前下一步曾以 `PORTING_STRATEGY_ENGINE.md` 和 `WEBGL_RENDERER_PLAN_2026-05-18.md` 为准；冻结后这些内容只作为旧线参考，不再作为未来实现主线。FFmpeg 静态链接、media inventory、KRKR Storage -> FFmpeg AVIO、direct audio phase3B、direct `VideoOverlay` movie A/V phase4 与 EXTRA/movie 手动验收已完成；剩余 FFmpeg 兼容性、主流程 audio survey、wait reason、AlphaMovie/AMV 和 longrun 均作为迁移参考项处理。

## 已解决问题索引

| 问题 | 最终根因 | 修复位置 |
|---|---|---|
| 正文角色立绘不可见 | AutoPath 只注册了 `data.xp3>fgimage/直太/` 和 `fgimage.xp3>`，没有注册 `fgimage.xp3>直太/`，短名查询 `直太a_info.txt` 失败。 | `external/krkrz/base/StorageIntf.cpp::TVPResolveSiblingArchiveAutoPath()` |
| 标题 motion 不自主出现 | `Motion.Player.getCommandList()` playing 期间返回空数组，TJS `Scripts.equalStruct(old,new)` 一直认为没有变化，`update -> onPaint -> draw` 链不触发。 | `src/plugins/emoteplayer/emoteplayer_stub.cpp::getCommandList()` |
| logo / M2 motion 错误静态帧 | motion 播放前后曾允许静态 PSB 帧混入 live motion 路径。 | `src/plugins/emoteplayer/emoteplayer_stub.cpp::draw()` 的 CPU-IR / static fallback 分流 |
| 正文 `mono_loop` 栏杆循环过快 / 跳变 | 游戏配置传入 `speed=1.0` 表示正常倍率；wasm 端曾把它直接当内部分母，`progress(dt)` 等价于 `tick += dt`，导致 120ms duration 的循环频繁 wrap。 | `src/plugins/emoteplayer/emoteplayer_stub.cpp::MotionEffectiveSpeedDivisor()` / `progress()` |
| KAGParserEx 集成后 logo/title 回退 | 临时 motion bridge 仍在旧 KAGParser，替换为 KAGParserEx 后 `[ev storage=*.psb]` / `[waitmovie]` 没有进入 bridge。 | `external/KAGParserEx/KAGParserEx.cpp` 临时承载 P0-004 bridge |

## 关键代码入口

| 领域 | 文件 |
|---|---|
| Motion / emote runtime | `src/plugins/emoteplayer/emoteplayer_stub.cpp` |
| PSB static compositor / PSB plugin | `src/plugins/psb/psb_gfx_loader.cpp`、`src/plugins/psb/psb_plugin.cpp` |
| KAGParserEx | `external/KAGParserEx/KAGParserEx.cpp`、`external/KAGParserEx/KAGParserEx.hpp` |
| KAG/TJS motion bridge 注入 | `external/KAGParserEx/KAGParserEx.cpp`、`external/krkrz/base/ScriptMgnIntf.cpp` |
| AutoPath / XP3 / storage | `external/krkrz/base/StorageIntf.cpp`、`external/krkrz/base/XP3Archive.cpp`、`src/core/base/sdl2/StorageImpl.cpp` |
| Layer / input / invalidation | `external/krkrz/visual/LayerIntf.cpp`、`external/krkrz/visual/LayerManager.cpp` |
| Plugin fallback | `src/core/base/sdl2/PluginImpl.cpp`、`src/plugins/InternalPlugins.cpp` |
| Audio / video | `src/core/sound/sdl2/WaveImpl.cpp`、`src/core/sound/sdl2/FFmpegWaveDecoder.cpp`、`src/core/visual/sdl2/VideoOvlImpl.cpp` |
| WebGL renderer / CPU-IR fallback | `src/plugins/emoteplayer/emoteplayer_stub.cpp`、`src/plugins/psb/psb_gfx_loader.cpp`、`external/krkrz/visual/LayerIntf.cpp`、`external/krkrz/visual/LayerManager.cpp` |

## 构建与同步（冻结前旧线）

```bash
cmake --build /Users/xiabin/my_work1/krkrsdl2/build_web -j 8
cp /Users/xiabin/my_work1/krkrsdl2/build_web/krkrsdl2.js /Users/xiabin/my_work1/web-krkr/krkrsdl2.js
cp /Users/xiabin/my_work1/krkrsdl2/build_web/krkrsdl2.wasm /Users/xiabin/my_work1/web-krkr/krkrsdl2.wasm
```

构建后从 `web-krkr` 侧跑 gate / survey。

## 必读文档

- `/Users/xiabin/my_work1/krkrsdl2/PORTING_STRATEGY_ENGINE.md` — 引擎路线
- `/Users/xiabin/my_work1/krkrsdl2/WEBGL_RENDERER_PLAN_2026-05-18.md` — WebGL renderer 下一阶段计划
- `/Users/xiabin/my_work1/krkrsdl2/FFMPEG_AV_IMPLEMENTATION_PLAN_2026-05-17.md` — FFmpeg 音视频实现交接与验收计划
- `/Users/xiabin/my_work1/krkrsdl2/ENGINE_COMPAT_MATRIX.md` — 插件 / API 能力矩阵
- `/Users/xiabin/my_work1/krkrsdl2/STUB_HACK_AUDIT.md` — stub/hack 风险与退场条件
- `/Users/xiabin/my_work1/krkrsdl2/KRKRSDL3_REFERENCE.md` — 外部参考索引
- `/Users/xiabin/my_work1/krkrsdl2/MOTION_RUNTIME_SDL3_SEMANTIC_GAP_2026-05-16.md` — motion/runtime SDL3 函数级语义差距与迁移表
- `/Users/xiabin/my_work1/web-krkr/PROJECT_CONTEXT_WEB.md` — 浏览器侧事实快照
- `/Users/xiabin/my_work1/web-krkr/PORTING_STRATEGY_WEB.md` — 浏览器侧验收策略
