# KRKR reference index

文档标志位：`KRKRSDL3_REFERENCE`
文档类型：外部实现调研和参考索引，不是当前状态快照。
适用范围：`krkrsdl2` WASM 移植时查阅 `/Users/xiabin/my_work1/krkrsdl3` 和其它专项参考。
更新日期：2026-05-13

## 一句话结论

`krkrsdl3` 仍是当前工作区最有价值的 KRKR 语义参考，但不能整块搬进 wasm。它适合回答"原语义是什么、TJS API 面是什么、生命周期怎么走"，不适合作为 WebGL2/WASM 的现成实现。

参考链定位：

| 参考源 | 用途 |
|---|---|
| `/Users/xiabin/my_work1/krkrsdl3` | motion/emote、PSBFile、KAGParserEx、LayerExDraw、VideoOverlay、插件注册语义 |
| `storycraft/emote-psb-rs` | PSB/MDF/value/resource/extra resource 结构校验 |
| `storycraft/scn-tool` + `krkrz_textrenderguide` | 后续 SCN/TextRender/文本命令参考 |
| `krkrz/krkrz` / `krkrz/krkr2` | canonical API、win32 插件源码、layerEx/extrans 等 |
| `KrKr2-Next` | 未来 WebGL/texture bridge 思路参考，注意 GPL-3.0 许可 |

本文同时承担专项参考链索引和 krkrsdl3 emote/motion 语义索引；需要外部实现时先从这里分流。

## krkrsdl3 仓库轮廓

本地路径：

```text
/Users/xiabin/my_work1/krkrsdl3
```

主要目录：

| 目录 | 参考价值 |
|---|---|
| `cpp/core/archive` | storage、XP3、路径和 archive 语义 |
| `cpp/core/main` | application、事件队列、生命周期 |
| `cpp/core/media/graphics` | Layer、DrawDevice、RenderManager、软件纹理 |
| `cpp/core/media/movie` | VideoOverlay / krmovie 状态事件 |
| `cpp/core/media/sound` | wave decoder、音频队列状态 |
| `cpp/core/script` | TJS native class 注册面 |
| `cpp/environ` | SDL3 窗口、输入、音频、平台桥接 |
| `cpp/plugins` | 内置插件集合 |

不宜照搬：SDL3 平台输出层、OpenCV、OpenGL 4.3 / GLES 3.2 tessellation、SDL_mixer/native audio 链路。音视频里程碑采用 FFmpeg-first，但需要按 WASM / KRKR Storage / 浏览器输出设备重新接入，不能机械复制 SDL3 平台层；当前下一阶段主线已切到 WebGL renderer，SDL3 renderer 只能作为语义参考。

## 插件加载模型

关键文件：

```text
cpp/plugins/TVPPlugin.cpp
cpp/plugins/CMakeLists.txt
cpp/plugins/ncbind
```

可借鉴点：

- 内部插件优先，不在 wasm 中真实加载 DLL；
- `TVPLoadInternalPlugin` 应接受完整路径和 basename；
- 插件矩阵必须区分"注册面存在"和"语义可信"。

## PSBFile / PSB storage

关键文件：

```text
cpp/plugins/psbfile/PSBFile.cpp
cpp/plugins/psbfile/psbData.cpp
cpp/plugins/psbfile/psbMedia.cpp
cpp/plugins/emoteplayer/emotefile.cpp
```

可借鉴点：

- `PSBFile` 是脚本可见 native class，不应只是内部图像工具；
- `psb://file.psb/chunkName` storage media 对 motion resource、嵌套图片、extra chunk 重要；
- MDF、lzfs/LZ4、decrypt hook、resource/extra resource 需要统一索引；
- 文件结构优先用 `emote-psb-rs` 交叉校验，krkrsdl3 更适合作 TJS/API/storage 形态参考。

## Emote / Motion runtime

关键文件：

```text
cpp/plugins/emoteplayer/emoteplayer.cpp
cpp/plugins/emoteplayer/emoteplayerclass.cpp
cpp/plugins/emoteplayer/emotefile.cpp
cpp/plugins/emoteplayer/emotefile.h
```

TJS 绑定面：

```text
Motion
Motion.ResourceManager
Motion.SeparateLayerAdaptor
Motion.EmotePlayer
Motion.Player
```

重点参考语义：

- `ResourceManager.load/unload/unloadAll`；
- `Player.play/progress/draw/clear/serialize/unserialize`；
- `playing/allplaying/animating/loopTime/tickCount/speed/motionKey/motion/chara/variableKeys`；
- `setVariable/getVariable`、timeline play/stop/query、sync/end 状态；
- MDF/lzfs/decrypt、metadata/object/source/motion/timeline/variable 解析；
- `progress()` 中对 `lastTime`、`syncTime`、timeline/variable 的处理。

专项差距表：

- `MOTION_RUNTIME_SDL3_SEMANTIC_GAP_2026-05-16.md` 记录 SDL2/WASM 与 SDL3 在 motion/runtime 外延上的函数级差距、当前 SYSTEM/config UI 刷新红点归属，以及迁移到“语义等同 SDL3”的工程表。

渲染注意：

- krkrsdl3 的 renderer 依赖 tessellation shader：`GL_TESS_CONTROL_SHADER`、`GL_TESS_EVALUATION_SHADER`、`GL_PATCHES`；
- WebGL2 不支持 tessellation，因此 wasm 端要保留当前 CPU mesh/raster 或另写 WebGL2 兼容 mesh 路径；
- krkrsdl3 里 `contains`、`skip`、`assign`、physics/wind、timeline blend 等也有 TODO 或弱实现，不要当完整官方行为。

## KAGParserEx

关键文件：

```text
cpp/plugins/KAGParserEx/KAGParserEx.cpp
cpp/plugins/KAGParserEx/KAG.cpp
cpp/core/script/tjsNativeKAGParser.cpp
```

当前 `krkrsdl2` 已静态集成 KAGParserEx，后续只在遇到这些问题时回查：

- label/call/return 栈；
- macro/inline script；
- scenario cache / label cache；
- 存档栞数据；
- KAGParserEx 专属标签差异。

不要把当前 wasm 的 P0-004 motion bridge 当作 KAGParserEx 语义；那是临时层次反转，退场目标在 `STUB_HACK_AUDIT.md`。

## Layer / drawing / input

关键文件：

```text
cpp/core/media/graphics/LayerBitmap.cpp
cpp/core/media/graphics/LayerManager.cpp
cpp/core/media/graphics/RenderManager.cpp
cpp/core/media/graphics/DrawDevice.cpp
cpp/plugins/LayerExDraw/general/LayerExDraw.cpp
```

可借鉴点：

- DrawDevice 只转发输入和 invalidation，LayerManager 负责实际 hit-test / draw buffer；
- LayerExDraw 的 Blend2D 方向比 Windows GDI+ 更接近 wasm 可移植思路；
- LayerExImage/Raster/Save/extrans 在 krkrsdl2 已有移植，后续应以行为对齐和测试为主。

风险：

- 插件不允许覆盖已有 native `Layer.drawText` 等方法；
- `windowEx.cpp` 在 krkrsdl3 也偏 stub，只能借 API 名称和少量兼容脚本。

## Video / audio

关键文件：

```text
cpp/core/script/tjsNativeVideoOverlay.cpp
cpp/core/media/movie
cpp/core/media/sound
cpp/environ/WaveMixer.cpp
```

可借鉴语义：

- `VideoOverlay` 状态：`unload / ready / play / pause / stop`；
- 播放结束发 `EC_COMPLETE`，帧更新发 `EC_UPDATE`；
- 音频需要回答播放状态、队列耗尽、loop/segment 是否结束。

wasm 实现方向：

- FFmpeg-first：FFmpeg 负责 demux/decode，KRKR Storage 通过自定义 AVIO 接入；
- 音频输出用 Web Audio，但解码和状态机不依赖浏览器原生 codec；
- 视频输出写回 KRKR video/layer 路径；HTMLVideoElement 只作对照/降级信息；
- 短期可以后置 AlphaMovie / 透明视频画面细节，但 wait/end/status 必须可信。

## 查阅索引

| 问题 | 优先看 |
|---|---|
| 内部插件注册 | `cpp/plugins/TVPPlugin.cpp` |
| PSBFile TJS API | `cpp/plugins/psbfile/PSBFile.cpp` |
| PSB storage media | `cpp/plugins/psbfile/psbMedia.cpp` |
| PSB type/header/value | `cpp/plugins/psbfile/psbData.cpp`、`emote-psb-rs` |
| MDF/lzfs/decrypt | `cpp/plugins/emoteplayer/emotefile.cpp` |
| Motion TJS 绑定 | `cpp/plugins/emoteplayer/emoteplayer.cpp` |
| Player 生命周期 | `cpp/plugins/emoteplayer/emoteplayerclass.cpp` |
| motion tree/timeline/变量 | `cpp/plugins/emoteplayer/emotefile.h/.cpp` |
| Layer 更新和输入 | `cpp/environ/MainWindowLayer.cpp`、`cpp/core/media/graphics/DrawDevice.cpp` |
| 视频 complete/status | `cpp/core/script/tjsNativeVideoOverlay.cpp`、`cpp/core/media/movie/KRMovieLayer.cpp` |
| 音频队列 | `cpp/environ/WaveMixer.cpp`、`cpp/core/media/sound/WaveIntf.cpp` |
| KAGParserEx 语义 | `cpp/plugins/KAGParserEx/KAGParserEx.cpp` |

## 许可和复制规则

- krkrsdl3 许可含额外分发条件；复制代码前必须确认许可证和保留声明。
- KrKr2-Next 为 GPL-3.0，当前只作架构参考，不复制代码。
- 优先按语义重写或小范围移植并记录来源。
