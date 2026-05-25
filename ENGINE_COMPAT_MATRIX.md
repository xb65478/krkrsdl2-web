# krkrsdl2 engine compatibility matrix

文档标志位：`ENGINE_COMPAT_MATRIX`
文档类型：engine 侧插件 / API 能力矩阵。
权威规则：本文记录"要补什么、当前实现到哪、风险在哪里"；stub/hack 退场见 `STUB_HACK_AUDIT.md`；路线见 `PORTING_STRATEGY_ENGINE.md`。
更新日期：2026-05-18

## 用法

这份矩阵合并了三类来源：

- 当前 wasm 运行证据：`web-krkr` gate/survey 日志；
- 参考源码：`krkrsdl3`、`krkrz/krkr2`、其它专项仓库；
- 闭源 Android/Kirikiroid2 逆向结论：只保留对插件覆盖和迁移优先级有用的结果。

闭源逆向只在本矩阵保留对插件覆盖和迁移优先级有用的结论；函数地址、Ghidra 过程和已否定方向不作为长期工作入口。

## 状态标记

| 状态 | 含义 |
|---|---|
| native | 已有 C++/ncbind/native 实现，主语义可执行 |
| partial native | 有 C++ 实现或 parser，但只覆盖部分语义 |
| cpp stub | C++ 注册面存在，主要用于防崩或 fallback |
| tjs stub | Emscripten `PluginImpl.cpp` 注入 TJS 兜底 |
| no-op | 返回成功、空行为或空数据，可能误导脚本 |
| missing | 未实现或没有 stub |

## 当前证据快照

DRACU-RIOT! 当前主样本已经覆盖：

- `motionload title.psb` / `m2logo.psb` / `yuzulogo.psb`，logo 和标题 motion 可见；
- KAGParserEx 替换原 KAGParser 后，标题点击可进入 `start.ks` / `*envplay`；
- 正文主流程 `mono_loop.psb` 栏杆循环动画已手动验收通过；`speed=1.0` 作为正常倍率处理；
- 正文 `cg_直太.STAND` 已触发 `.stand`、`直太a_info.txt`、`直太a_*` 图块加载，角色立绘可见；
- SAVE / LOAD / SYSTEM 菜单和存档读写纵向 survey 已通过；
- Audio direct `WaveSoundBuffer` phase3B 已完成；VideoOverlay movie A/V phase4 已完成，用户手动验收确认 EXTRA -> movie 第一个解锁 movie 可见、有声、点击可停止返回。当前下一步是 WebGL renderer 接入；CPU-IR 保留为第二语义、回归对照和 fallback。主流程 BGM/SE/voice 命中 survey、media wait reason、AlphaMovie/AMV、longrun、QuickMenu / MSGWIN、性能和跨包通用化保留为待做 / 待优化项。

## Plugin matrix

| DLL / API | 能力层 | 当前状态 | 逆向/参考结论 | 风险 | 下一步 / 验收 |
|---|---|---|---|---|---|
| `motionplayer.dll` / `emoteplayer.dll` | L5 motion/emote | partial native。默认可用语义是 `cpu-ir`；logo、标题 motion、正文 `mono_loop` 循环、正文角色立绘已可见。当前下一步是 WebGL renderer 接入，CPU-IR 保留为第二语义、fallback 和对照。 | Android libgame.so 内建 M2 Motion 类族；公开参考以 `krkrsdl3/cpp/plugins/emoteplayer` 为主，`emote-psb-rs` 校验 PSB/MDF 结构。`speed=1.0` 在当前样本按正常播放倍率处理。 | 高。WebGL texture registry、sourceKey -> texture、type=1/bp、stencil、blend、z/order、physics、timeline 多 motion 叠加、长 replay 稳定性未完整闭环。 | 当前主线：WebGL 第一刀，产出 CPU-IR/WebGL 双截图、texture-missing、unsupported-command、fallback reason。待做 / 待优化：QuickMenu / MSGWIN 诊断固化、退 P0-004、真实 bp/type=1、depth/stencil、blend/physics/timeline。入口：`WEBGL_RENDERER_PLAN_2026-05-18.md`、`diag_logo_dynamic_draw.js`、`diag_motion_state.js`。 |
| `psbfile.dll` / `PSBFile` / `psb://` | L3/L5 PSB | partial native。PSBFile 和 media provider 已注册，PSB dump 与图像 fallback 可用。 | Android/Kirikiroid2 内建 PSBFile/PSBValue/psb media；`krkrsdl3` 有 TJS API 与 storage 参考。 | 高。root/resource/extra/lzfs/MDF/decrypt 若不全，会让 motion/env restore 静默偏差。 | 补 PSB root/resource/extra survey。入口：`diag_psb_dump.js --no-click`。 |
| `KAGParserEx.dll` | L1 script/KAG | native。静态集成并替换原 KAGParser，class 名仍为 `"KAGParser"`。当前临时承载 P0-004 motion bridge。 | `krkrsdl3/cpp/plugins/KAGParserEx` 是完整源码参考；Android 逆向确认 KAGParserEx/KAGParserExb 属于内建解析器族。 | 中。Parser 本体可用，但 motion bridge 层次反转必须退场。 | motion 待做项：长流程遇到 label/call/macro/栞差异再查，并退 P0-004。入口：`diag_title_click.js`、`diag_progress_first_line.js`、longrun。 |
| `layerExDraw.dll` | L2 drawing/UI | tjs stub，避免覆盖已有 `Layer.drawText`。 | Android 逆向显示其是 GDI+/绘图替代层；`krkrsdl3` 用 Blend2D/OpenCV 实现部分能力。 | 高。覆盖原生 Layer 或 no-op 绘图会造成文字/UI 静默缺失。 | 审计 attach 到 `Layer` 的方法，按真实调用点补最小软件绘图。 |
| `windowEx.dll` | L2/L3 window/input | tjs stub，含 `PassThroughDrawDevice`、`dt*` 常量、部分输入/窗口 API。 | Android 逆向显示大量 Win32 window/message API 在移动端是 mock。`krkrsdl3` 也偏 stub。 | 中高。draw device、按键状态、系统菜单交互可能偏。 | 只按真实调用点补，不模拟无意义 Win32 窗口。 |
| `krmovie.dll` / `VideoOverlay` | L4 video | partial native。`VideoOverlay` WASM 路径已接 `FFmpegVideoPlayer`：真实 `.wmv` 可 FFmpeg decode、写回 KRKR Layer，并输出 WMAPRO movie audio 到 OpenAL/WebAudio queue；`mono_loop.wmv` 和 `OP_low.wmv` 已通过 phase4 diagnostics；用户手动验收确认 EXTRA -> movie 第一个解锁 movie 可见、有声、点击可停止返回。`krmovie.dll` facade 仍偏 stub。 | Android 逆向/krkrsdl3 都表明视频关键在 ready/play/complete/status 事件，不只是画面；本轮已证明 direct VideoOverlay A/V 和实际 UI 入口最小闭环，脚本 wait reason 仍需继续贴近。 | 中。direct probe 和手动 UI 入口可信，但 waitvideo/wb/wm 的 natural ended/click-skip/stop/error reason、AlphaMovie/AMV、longrun A/V drift 仍未闭合。 | 待做 / 待优化：补 wait reason 和 longrun；保留 `diag_video_wait.js` phase4 回归，artifact `/Users/xiabin/my_work1/web-krkr/diag-results/video-phase4-2026-05-18T07-26-41-001Z/summary.json`、`/Users/xiabin/my_work1/web-krkr/diag-results/video-phase4-oplow-2026-05-18T07-26-41-001Z/summary.json`。 |
| `AlphaMovie.dll` | L4 alpha video | tjs stub，当前 gate 未见明确主线调用。 | Android libgame.so 内建 AlphaMovie；公开参考有 `krkrsdl3/cpp/plugins/AlphaMovie.cpp` 和 AMV decoder。 | 中。可能影响 logo/OP/透明视频 wait。 | 若主线命中，先补状态/wait，再考虑解码。 |
| `WaveSoundBuffer` / `wuvorbis.dll` | L4 audio | partial native。Direct `WaveSoundBuffer` 已经通过 FFmpeg wave decode、OpenAL/WebAudio queue、wasm pump 的 phase3B 验收；position/pause/resume/stop/natural end/loop 可观测。`wuvorbis.dll` facade 仍未完成。 | Android/SDL3 都说明真实包体音频不能靠 stub；当前 Web 端按 FFmpeg-first 解码，OpenAL/WebAudio 只作输出设备。 | 中。Direct probe 可信，但游戏原脚本 BGM/SE/voice 自动命中、wuvorbis facade、longrun wait/label/seek/volume 仍未闭合。 | 待做 / 待优化：扩主流程 audio survey，确认原 KAG/TJS 自然命中同一 `WaveSoundBuffer`/FFmpeg 链；保留 `diag_audio_state.js` phase3B 回归，artifact `/Users/xiabin/my_work1/web-krkr/diag-results/audio-phase3-2026-05-18T07-15-30-145Z/summary.json`。 |
| `varfile.dll` / `saveStruct.dll` / storage helpers | L0/L3 storage | native/partial native；存档与配置 survey 已通过。 | 多数为常规插件能力。 | 中。异常恢复、长流程 reload、压力测试未覆盖。 | 保持 save/load survey，补 longrun reload。 |
| `extrans.dll` | L2 transitions | native 移植多种 transition，Emscripten 仍有 provider fallback。 | Android 内建核心转场；公开 krkr2/krkrsdl3 可参考。 | 中。长流程转场视觉/timing 可能偏。 | 长流程截图和调用日志确认。 |
| `extNagano.dll` | L3 vendor/ext | tjs no-op，当前未见 gate blocker。 | Android 逆向显示核心转场类能力有实现，但公开可靠源码不足。 | 中。当前样本没暴露不代表可长期 no-op。 | 静态扫描和跨包 longrun 命中后补 API。 |
| `yuzuex.dll` | L3 vendor/ext | tjs no-op，当前未见调用 blocker。 | Android 逆向显示其偏 YuzuSoft 选项/专有扩展，未内建。 | 中。可能影响设置或厂商功能。 | 静态扫描成员调用；跨包验证。 |
| `kagexopt.dll` | L1/L3 options | tjs no-op / 候选。 | Android 逆向显示未移植到 libgame.so，偏 Windows 桌面选项描述。 | 低中。通常不是主流程 blocker。 | 命中具体 API 再升级。 |
| `multiimage.dll` | L2 image utility | tjs no-op，当前未见 blocker。 | Android 逆向显示基础多图合成能力存在。 | 低中。特殊 UI/转场可能缺图。 | 静态扫描 API 使用名。 |
| `PackinOne.dll` | L0 packaging/tool | tjs no-op。 | Android 逆向显示多数文件系统工具可内建，但运行时通常低优先。 | 低。 | 保持延期，命中后再处理。 |
| `win32dialog.dll` | L3 platform dialog | native 存在；浏览器中许多 dialog 语义需 web 化。 | Android 逆向显示 Win32 控件多为 mock。 | 低中。 | 调用文件选择/消息框时映射到 web 可观测 UI。 |
| `layerExBtoA.dll` | L2 image utility | missing，日志曾出现 `No stub for: layerExBtoA.dll`。 | `krkrsdl3/cpp/plugins/LayerExBTOA.cpp` 可查。 | 中。可能是可选图像处理。 | 查调用点，决定 stub 或 native。 |

## Native plugin inventory

本节只回答"当前 krkrsdl2 是否已有原生实现"；优先级仍以主矩阵运行证据为准。

| DLL / module | 源文件 | 状态 |
|---|---|---|
| `addFont.dll` | `src/plugins/addFont.cpp` | native/low priority |
| `csvParser.dll` | `src/plugins/csvParser.cpp` | native |
| `fstat.dll` | `src/plugins/dirlist.cpp` | native |
| `fftgraph.dll` | `src/plugins/fftgraph.cpp` | native/low priority |
| `getSample.dll` | `src/plugins/getSample.cpp` | native/audio 后续复查 |
| `saveStruct.dll` | `src/plugins/saveStruct.cpp` | native/save survey 覆盖 |
| `varfile.dll` | `src/plugins/varfile.cpp` | native |
| `win32dialog.dll` | `src/plugins/win32dialog.cpp` | native |
| `wutcwf.dll` | `src/plugins/wutcwf.cpp` | native/audio 后续复查 |
| `xp3filter.dll` | `src/plugins/xp3filter.cpp` | native/preload |
| `scriptsEx.dll` | `src/plugins/scriptsEx.cpp` | native |
| `shrinkCopy.dll` | `src/plugins/shrinkCopy.cpp` | native |
| `layerExRaster.dll` | `src/plugins/layerExRaster.cpp` | native |
| `layerExImage.dll` | `src/plugins/layerExImage.cpp` | native |
| `layerExSave.dll` | `src/plugins/layerExSave.cpp` | native |
| `extrans.dll` | `src/plugins/extrans/` | native |

## Core API matrix

| API / class | 当前状态 | 风险 / 下一步 |
|---|---|---|
| `Layer` | core native，插件可 attach 扩展。 | 禁止 stub 覆盖已有 native 方法；补 Layer attach 审计。 |
| `Window` / draw device | core native + `windowEx` stub。 | QuickMenu / MSGWIN 当前红，需截图 artifact + action 双 gate。 |
| `Storages` / XP3 / Range | 主路径可用，AutoPath sibling archive 已修复。 | 异步 I/O、异常恢复、长流程 reload 未完成。 |
| `KAGParser` / scenario | KAGParserEx native，可到正文并显示角色立绘。 | P0-004 bridge 退场；长流程脚本栈继续观察。 |

## External reference index

| 仓库 / 资料 | 用途 |
|---|---|
| `/Users/xiabin/my_work1/krkrsdl3` | 最重要的 KRKR 语义参考：PSBFile、emoteplayer、KAGParserEx、LayerExDraw、VideoOverlay、插件注册。 |
| `storycraft/emote-psb-rs` | PSB/MDF/value/resource/extra resource 结构校验。 |
| `storycraft/scn-tool` + `pantsudev/krkrz_textrenderguide` | 后续 SCN/TextRender/文本命令参考。 |
| `krkrz/krkrz` / `krkrz/krkr2` | canonical API / win32 插件源码。 |
| `KrKr2-Next` | 未来 WebGL/texture bridge 方向参考，不可直接复制 GPL 代码。 |
| `uyjulian/krmovie`、`wuffmpeg`、`krmpv` | 视频插件 API/状态参考。 |
| `xmoezzz/amv_decoder` | AlphaMovie 解码后置参考。 |

## 维护规则

- 每新增 stub/fallback，必须登记到本矩阵和 `STUB_HACK_AUDIT.md`。
- 每次 gate/survey 发现新 DLL/API/脚本调用点，补矩阵行，不只在代码里临时修。
- 优先级按"运行证据 + 用户可见风险 + 低层 gate 风险"排序，不按 DLL 是否存在排序。
- 引入第二款商业包后，证据列必须区分 DRACU 与其它包。
