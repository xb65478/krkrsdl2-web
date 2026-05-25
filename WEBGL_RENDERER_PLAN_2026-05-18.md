# WebGL Renderer Plan

文档标志位：`WEBGL_RENDERER_PLAN`
文档类型：执行交接计划。
更新日期：2026-05-18

适用范围：

- Engine repo: `/Users/xiabin/my_work1/krkrsdl2`
- Web repo: `/Users/xiabin/my_work1/web-krkr`
- 当前主样本：`DRACU-RIOT!`

## 当前决策

Audio / Video 已跨过当前可玩性里程碑：direct `WaveSoundBuffer` audio phase3B、direct `VideoOverlay` movie A/V phase4、以及 EXTRA -> movie 手动播放有声并可点击返回均已完成。下一阶段主线切到 **WebGL renderer 接入**。

CPU-IR 不删除，也不再作为下一步扩写主线。它从现在起定位为：

- 第二语义：保留当前已经可见的 motion/render 行为，作为 WebGL 的行为对照；
- fallback backend：WebGL 不可用、命中未支持语义或诊断需要时回退；
- trace/reference backend：输出 command、draw item、texture key、z/order、blend/mask 信息，供 WebGL 对齐；
- 回归护栏：WebGL 改动不能破坏 CPU-IR 已经通过的标题、正文、角色立绘、movie 播放路径。

## 目标

把现有 motion/runtime 的 CPU command / renderer IR 消费链扩展出 WebGL 后端，让 motion、PSB 图元和后续 texture-heavy 场景不再长期依赖 CPU raster / canvas copyback。

第一阶段目标不是一次性实现 SDL3 OpenGL 等价，而是在浏览器里建立可开关、可回退、可截图验收的 WebGL path：

```text
Motion / PSB runtime
  -> renderer command entries
  -> sourceKey / resource image resolve
  -> WebGL texture registry
  -> ordered draw pass
  -> canvas visible frame
  -> screenshot/manual or multimodal review
```

## 非目标

- 不整块搬 SDL3 的 OpenGL/FBO/tessellation/glReadPixels 平台层。
- 不删除 CPU-IR；CPU-IR 是第二语义和回退路径。
- 不把静态 PSB compositor 重新作为 motion 动画主路径。
- 不靠像素统计替代视觉验收；Web 侧产出截图，由人或多模态模型判断。
- 不在第一阶段追求所有 physics、timeline、多 motion attach、AMV/AlphaMovie、视频帧 WebGL 上传。

## 第一刀

1. 增加 WebGL backend 开关。

```text
motion_renderer_backend=webgl
motion_renderer_backend=cpu-ir
motion_renderer_backend=static-psb
```

默认切换时必须保留回退：WebGL init 失败、texture missing、unsupported command 时能显式回落 CPU-IR，并记录原因。

2. 固化 CPU-IR 对照输出。

CPU-IR 每帧至少能导出：

- command count；
- sourceKey / texture candidate；
- draw rect / uv / opacity；
- z/order；
- blend/mask/stencil 标记；
- texture-missing 列表。

3. 建 WebGL texture registry。

第一刀只覆盖已能显示的资源路径：

- PSB / motion resource image；
- sourceKey -> decoded bitmap / layer source；
- texture upload / update / dispose；
- atlas 或单纹理策略先按最小可用实现，后续再优化。

4. WebGL 消费现有 renderer command。

先消费 CPU-IR 已经能表达的安全子集：

- textured quad；
- opacity；
- basic alpha blend；
- z/order stable sorting；
- canvas/frame clear；
- viewport/scaling 与当前 `1280x720` backing 对齐。

5. 截图验收。

Web 侧同一场景保留 CPU-IR 与 WebGL 两份截图：

- logo/title motion；
- 正文 `mono_loop`；
- 正文角色立绘；
- SYSTEM/menu 局部；
- 失败时保存 texture-missing / unsupported-command 摘要。

## 后续语义队列

WebGL 第一刀通过后，再按真实命中推进：

1. type=1 / bp 资源闭环；
2. mask / stencil；
3. blend modes；
4. z/order 与多层排序；
5. dynamic texture update / cache / LRU；
6. timeline / physics 对 draw command 的影响；
7. 视频帧或 AlphaMovie 进入 WebGL texture 的可能性。

## 验收

最低验收：

```bash
cmake --build /Users/xiabin/my_work1/krkrsdl2/build_web -j 8
cp /Users/xiabin/my_work1/krkrsdl2/build_web/krkrsdl2.js /Users/xiabin/my_work1/web-krkr/krkrsdl2.js
cp /Users/xiabin/my_work1/krkrsdl2/build_web/krkrsdl2.wasm /Users/xiabin/my_work1/web-krkr/krkrsdl2.wasm
node /Users/xiabin/my_work1/web-krkr/diag_visual_capture.js
node /Users/xiabin/my_work1/web-krkr/diag_title_click.js
node /Users/xiabin/my_work1/web-krkr/diag_progress_first_line.js
```

WebGL 专项验收需要新增或扩展：

```text
web-krkr/diag_webgl_renderer.js
web-krkr/diag-results/<run-id>/webgl-cpu-ir/
```

artifact 至少包含：

- CPU-IR 截图；
- WebGL 截图；
- renderer command summary；
- texture registry summary；
- unsupported command / texture-missing summary；
- 浏览器 console 中 WebGL init / context lost / shader compile 错误。

通过标准：

- WebGL path 能在浏览器中显示同一场景，不黑屏；
- 没有 silent texture missing；
- CPU-IR fallback 可用且可观测；
- 标题 Start、正文首句、角色立绘不回退；
- 截图经人工或多模态验收确认主要视觉正确。

## 关键入口

| 领域 | 文件 |
|---|---|
| Motion runtime / CPU-IR | `src/plugins/emoteplayer/emoteplayer_stub.cpp` |
| PSB static / image source | `src/plugins/psb/psb_gfx_loader.cpp` |
| Layer / invalidation / output | `external/krkrz/visual/LayerIntf.cpp`、`external/krkrz/visual/LayerManager.cpp` |
| Browser canvas host | `/Users/xiabin/my_work1/web-krkr/index.html` |
| Visual diagnostics | `/Users/xiabin/my_work1/web-krkr/diag_visual_capture.js`、`diag_logo_dynamic_draw.js`、`diag_motion_state.js` |

## 与 FFmpeg 的关系

FFmpeg 当前不再是下一阶段主线。已完成内容保留为回归：

- `diag_audio_state.js`：direct `WaveSoundBuffer` phase3B；
- `diag_video_wait.js`：direct `VideoOverlay` movie A/V phase4；
- `FFMPEG_INTEGRATION.md`：格式兼容、许可证和 wasm build profile 记录。

后续只有在 WebGL 相关路径需要视频帧作为 texture、或真实主流程暴露 wait/skip/longrun blocker 时，才重新打开 FFmpeg 专项。
