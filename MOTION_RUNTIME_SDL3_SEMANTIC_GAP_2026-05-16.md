# Motion Runtime SDL3 Semantic Gap

文档标志位：`MOTION_RUNTIME_SDL3_SEMANTIC_GAP`
文档类型：专项语义差距表和迁移工程表，不是流水账。
更新日期：2026-05-17

适用范围：

- SDL2/WASM 当前实现：`/Users/xiabin/my_work1/krkrsdl2/src/plugins/emoteplayer/emoteplayer_stub.cpp`
- SDL2/WASM TJS/bridge：`/Users/xiabin/my_work1/krkrsdl2/external/krkrz/base/ScriptMgnIntf.cpp`
- SDL3 参考实现：`/Users/xiabin/my_work1/krkrsdl3/cpp/plugins/emoteplayer/`
- SDL3 TJS 包装：`/Users/xiabin/my_work1/krkrsdl3/Res/D3DEmote.tjs`

## 结论

如果把“真正 Emote motion runtime”狭义定义为：

```text
PSB/motion 资源能读 -> Motion.Player.play/progress 能推进时间
-> draw 能产出可见动画 -> logo/title/正文 motion 肉眼接近 SDL3
```

那当前 SDL2/WASM 可以说已经 **基本完整**，成熟度大约是 80% 左右。这个判断和手动观察一致：动画已经能正常播，主流程观感已经接近 SDL3。

但如果把“真正 Emote motion runtime”按 SDL3 的外延定义为：

```text
TJS 包装层 + ResourceManager + EmotePlayer/Player
+ emotefile 对象图 + 变量/selector/parameterize
+ timeline/physics/wind/outerForce
+ layout/hit/LayerGetter
+ renderMethod/draw pipeline
+ serialize/cache/lifecycle
```

那当前 SDL2/WASM 还不能称为语义等同 SDL3。它现在是 **可见播放主线强、API 壳层宽、P1 变量/selector/parameterized frame 闭包已通过 SYSTEM 手动回归、P1.5 SYSTEM 设置持久化已通过、P2 object graph/source icon/同资源 nested motion refs 结构闭包已通过自动验收、P3 SYSTEM/config 控件覆盖已通过、timeline/physics/renderMethod/跨 PSB attach runtime 传播仍未完全闭合**。按“语义等同 SDL3”估算，整体大约在 70-78% 区间。

P1 已补齐 SYSTEM/config 设置项的 `button.getVariable/state` -> `mplayer/runtime variableValues` -> nested child motion local `state` -> parameterized frame selection -> draw rebuild 链路。用户手动回归确认“未读跳过”“已读自动跳过”有正确 UI 反馈。

P1.5 已证明 SYSTEM 设置点击后的持久化链路成立：自然点击 `tg_未読スキップ` 后，`kag.allskip/scflags.allskip` 从 `0` 变为 `1`，`datasc.ksd` / `datasu.ksd` 更新，默认不强制 sync 的 reload 后值保持。

2026-05-17 追加验证：持久化设置后刷新进入主流程的卡死已闭合。该问题不是 `.ksd`/IDBFS 读回失败，而是读回后的启动时序触发 `ResourceManager.load()` hot path 返回对象过重。SDL2/WASM 现在把 `ResourceManager.load()` 收窄为轻量 runtime root/metadata 返回，`rendererPrep`、完整 motion/layout/render dump 等重型诊断只保留在 `getResourceDump()`。

P3 已证明当前 SYSTEM/config 控件面的 layout/hit/LayerGetter 闭包成立：toggle、slider drag、tab、back/title/reset 走 click-first 链路，slider 仅依赖 held-button mousemove，不恢复 passive hover。

P2 当前 slice 已通过独立自动验收：SDL2/WASM 的 `MotionResource` dump 现在能导出 source icons、render methods、attachment refs 和 object graph 汇总；`title.psb/main.psb/config.psb/save_ui.psb/extra.psb` 的 source key 闭包为 0 unresolved，同资源 nested motion refs 可区分 resolved/external。边界是：这证明对象图和资源引用已可解释，不等于 SDL3 `addEmoteFile()` 式跨 PSB attach runtime 变量/timeline 传播已经完整。

## 外延定义

本文把 motion/runtime 外延分成 9 层：

| 层 | 内容 | 是否属于“完整 runtime” |
|---|---|---|
| L0 TJS 包装层 | `D3DEmote.tjs`、`AnimKAGLayer`、`MotionButton`、`MotionValue`、`calcUpdate()`、`notifyOwner()` | 是。不是 C++，但游戏脚本通过它观察 runtime |
| L1 API 壳层 | `Motion.ResourceManager`、`Motion.EmotePlayer`、`Motion.Player`、`Motion.SeparateLayerAdaptor` | 是 |
| L2 资源解析 | PSB/MDF/lzfs/decrypt、metadata/object/source/motion/layout/timeline/variable | 是 |
| L3 播放状态 | `play/progress/stop/skip/pass`、tick、duration、sync、loop、playing/allplaying/animating | 是 |
| L4 变量系统 | `setVariable/getVariable`、selector、parameterize、scoped/local key、nested/attach file | 是 |
| L5 layout/hit | `contains/getLayerGetter/getLayerMotion`、front-state、当前帧 layout | 是 |
| L6 timeline/physics | timeline control、blend、wind、outerForce、bust/hair/parts physics | 是 |
| L7 render/draw | renderMethod、texture、mesh/control points、stencil、blend、Layer 回写 | 是 |
| L8 生命周期 | serialize/unserialize、cache/unload、save/load/reload 后状态恢复 | 是 |

所以“动画已经播得像 SDL3”主要覆盖 L2/L3/L7 的 happy path，不自动覆盖 L4/L5/L6/L8。

## 层级边界

`krkrsdl3/Res/D3DEmote.tjs` 是引擎附带的 TJS 包装层，不是游戏包体脚本。它负责把游戏/KAG 的 motion 操作转交给 C++ 暴露出来的 `Motion.EmotePlayer` 或 `Motion.Player`。

归属关系：

| 调用/语义 | 所在层 |
|---|---|
| `D3DEmote.tjs::setVariable(name, value, time, accel)` | TJS 包装层 |
| `_player.setVariable(name, value)` | 跨入 C++ API |
| SDL3 `EmotePlayer::setVariable()` | C++ 壳层，继续写入 `emotefile::setVariable()` |
| SDL3 `emotefile::setVariable()` | 真正变量表/selector/attach file 语义 |
| `notifyOwner("entryFlip")` / `calcUpdate()` | TJS/Layer 刷新通知 |
| SDL2/WASM `getCommandList()` | 为当前游戏 TJS `workMotion()` 补的兼容观察面，SDL3 没有同名 API |

因此，UI 命中/layout、变量驱动刷新、motion draw 都属于 runtime 外延，只是位于不同层。它们不能简单归入“真正 C++ Emote draw runtime”，但缺任何一层，游戏看到的 motion 语义都会不完整。

## API 壳层对照

SDL3 注册的核心类：

- `Motion`
- `Motion.ResourceManager`
- `Motion.SeparateLayerAdaptor`
- `Motion.EmotePlayer`
- `Motion.Player`

SDL2/WASM 当前注册的核心类：

- `Motion`
- `Motion.ResourceManager`
- `Motion.SeparateLayerAdaptor`
- `Motion.MotionHitShape`
- `Motion.EmotePlayer`
- `Motion.Player`

SDL2/WASM 的 API 表面并不窄，甚至比 SDL3 多出大量兼容/诊断 API，例如 `MotionHitShape`、`getCommandList()`、`resolveFrontHit()`、`getRendererFrontState()`、`getRendererBackend()`、`loadPSBFromMemory()`、`getResourceDump()` 等。当前瓶颈不是“函数名没迁完”，而是很多函数背后的对象图和状态传播还不是 SDL3 等价。

## 函数级差距表

| 功能组 | SDL3 参考函数/文件 | SDL2/WASM 当前函数/文件 | 当前状态 | 距离语义等同 SDL3 还差什么 |
|---|---|---|---|---|
| 插件/API 注册 | `emoteplayer.cpp` 注册 `ResourceManager/SeparateLayerAdaptor/EmotePlayer/Player/Motion` | `emoteplayer_stub.cpp` 注册同名类，额外注册 `MotionHitShape` 和诊断 API | 壳层基本覆盖，SDL2 更宽 | 需要把“存在函数名”升级成“每个函数有 SDL3 对应语义或明确 Web 特化语义” |
| `Motion.getD3DAvailable/enableD3D` | SDL3 返回本地 D3D/GL 可用语义 | SDL2/WASM 返回 false/状态位 | 壳层兼容 | Web 后端不应伪装本地 D3D；需要文档化为 Web renderer backend 能力 |
| `ResourceManager.load` | `ResourceManager::load()` 创建 `emotefile`，`setSeed/setFun`，`emotefile::load()`，缓存 `TVPGetPlacedPath()`，返回 `emotefile.root()` | `ResourceManager::load()` 读 storage bytes，`ParseMotionResource()`，缓存 `MotionResource`，返回轻量 root/metadata；完整结构诊断走 `getResourceDump()` | 能读真实包体并解析 motion/layout/render metadata；2026-05-17 已把 hot path 上的 `rendererPrep` / `motions` 诊断返回移到 `getResourceDump()`；P2 已补 source icon/renderMethod/attachment refs/objectGraph dump | 解析模型仍不是 SDL3 `emotefile::GenerateAniTree()` 同构对象；跨 PSB attach file 的变量/timeline runtime 传播需要继续校准 |
| `ResourceManager.unload/unloadAll` | 删除 `emotefile` cache | 删除 `MotionResource` cache | 基本可用 | 资源卸载后 player 当前指针、runtimeLayout/draw cache、static bridge texture snapshot 需要长流程验收 |
| decrypt hooks | `setEmotePSBDecryptSeed/Func` 传入 `emotefile` | 有壳层字段 | 壳层存在 | decrypt function/seed 是否对所有 MDF/lzfs/extra resource 生效未证明 |
| memory load | SDL3 没有主注册面 | SDL2 有 `loadFromMemory/loadPSBFromMemory` 但基本空 | Web 特化壳层 | 要么补真实 bytes parser，要么标为 unsupported，避免游戏误以为可用 |
| `SeparateLayerAdaptor` | 包装 KRKR Layer，维护 FBO/texture，`checkDrawArea()` | 包装/持有 Layer，`assign()` 走 `AssignImages()`，`clear()` 更新 Layer | Web 可用性优先 | 不等同 SDL3 FBO 生命周期；Web 端需要自己的 canvas/Layer 回写契约 |
| `absolute` | SDL3 `set_absolute()` 写 `SetAbsoluteOrderIndex()` | SDL2 同样尝试写 Layer absolute | 基本可用 | 需和 LayerManager z/order、motion overlay 顺序做截图验收 |
| `EmotePlayer.play` | 选择 `_currentfile/_currmotion`，处理 motionKey/chara/motion/attach file | `playRaw/play()` 选择 current motion，重置 tick，设置 playing/allplaying/animating | 可见播放主线强 | `PlayFlagForce`、多 file attach、chara/motion fallback 选择规则需要按 SDL3 对齐 |
| `progress` | 更新时间、eye control、timeline control，调用 `_currmotion->progress()` | 更新时间、loop/duration/sync、timeline variables、effect state，驱动 runtime draw resource | 主流程强 | duration/sync/loop/variable-driven resource 的边界仍需跨包验收；logo safety-net 仍是样本恢复逻辑 |
| `draw` | GL/FBO + `_currmotion->draw()` + `glReadPixels` 回写 Layer | CPU-IR/static bridge/visibleRenderBridge，构建 renderer commands 写 Layer | 已能产出可见 motion | WebGL/CPU 后端还未完整消费 SDL3 renderMethod 语义，type=1/bp、stencil、blend、z/order、texture alias 仍是主缺口 |
| `clear` | 清 FBO/Layer target | SDL2 当前 `clear()` 基本空或只更新 Layer | 对主线影响低 | 需要明确哪些 TJS 调用依赖 clear 后透明/neutralColor |
| `update/render/assign` | SDL3 主注册面没有 `update/render`，有 `assign()` | SDL2 提供兼容函数 | Web/TJS 兼容 | 需要定义为等价 `progress/draw/assign` 还是仅兼容旧脚本 |
| transform | `setCoord/setScale/setRotate/setDrawAffineTranslateMatrix/setCameraOffset` 写 GL transform | SDL2 写 coord/scale/rotation/affine/camera 并影响 draw cache | 大体可用 | `coordZ`、camera、affine 与 Layer 坐标、hit/layout 的共同坐标系仍需校准 |
| color/opacity/visible | SDL3 `setColor()` 是 TODO，TJS 也有 Layer 包装 | SDL2 `setColor/getColor/opacity/visible` 存状态 | SDL2 壳层更完整 | 需要进入 renderer blend/color，不只是 serialize/front-state |
| variable basic | SDL3 `EmotePlayer::setVariable()` -> `emotefile::setVariable()` -> `_metadata->_varList` | SDL2 `setVariable()` 写 UTF-8 canonical `variableValues`，range clamp，invalidate cache，`variableRevision++` | P1 已闭合当前 SYSTEM toggle 路径 | 仍需跨资源/attach file 场景验收 scoped key 与变量生命周期 |
| selector control | SDL3 `emotefile::setVariable()` 检查 `_selectorControl`，`selectValue()` 写 option on/off variables | SDL2 `findSelectorControlForKey()` + `applySelectorControlValues()`，effective map 保留 selector key 并展开 option labels | P1 已闭合当前 SYSTEM toggle 路径；P2 已让 attachment refs/source graph 可诊断 | selector 作用域和跨 PSB attached resource 传播仍需 runtime 级覆盖 |
| parameterize | SDL3 `emotemotion::getTickByIdx()` 调 `emotefile::getTickByName()`，再 `transToTick()` | SDL2 `MotionSelectNodeLayoutFrame()` / runtime layout build 用 local/scoped/object 候选解析 variable map 选帧 | P1 已闭合当前 SYSTEM toggle；P3 已覆盖 SYSTEM slider dry-run；P2 已导出 nested ref 结构 | 仍需覆盖多参数 motion、跨资源 attached/nested runtime 传播 |
| `getVariable` | SDL3 直接读 `_metadata->_varList` | SDL2 读 `variableValues` 或变量 range 默认值 | 当前 button/TJS wrapper 路径可用 | 仍需核对默认值、range、serialize 后恢复语义 |
| `setMotionVariable/getMotionVariable` | SDL3 主注册面无此名 | SDL2 兼容 alias | 兼容层 | 应保持 alias，不当成 SDL3 原生差距 |
| `setVariableRange/getVariable*` | SDL3 从 metadata variableList 获取 min/max/name | SDL2 从 parsed `MotionVariableInfo` 和 manual range 获取 | 可观测 | range 与真实 metadata division/default/current value 仍需核对 |
| `getCommandList` | SDL3 无此 API | SDL2 为 `AnimKAGLayer.tjs::workMotion()` 输出 progress/variable/stop | 当前标题 motion 和变量刷新依赖它 | 必须只表达真实 runtime 状态变化；不能用 forced update 或 hover 注入伪造刷新 |
| hit basic | SDL3 local `EmotePlayer::contains()` 当前还是 TODO/弱实现 | SDL2 `containsRaw()`、layout-derived hit、group layout、front layout | SDL2 对当前 Web UI 更实用；P3 已覆盖 SYSTEM/config 控件面 | 不能把 SDL3 local TODO 当目标；目标应是游戏需要的 motion-derived hit 语义，后续扩到 QuickMenu/SAVE/LOAD 长流程 |
| LayerGetter | SDL3 TJS 侧依赖 Layer/owner 包装；C++ 没有 `getLayerGetter()` 注册 | SDL2 提供 `getLayerGetter/getLayerMotion/MotionHitShape` | Web 兼容扩展；P3 已验证 descendant leaf / front resolve 在 SYSTEM/config probes 上闭合 | 仍需更广资源下的 nested group、proxy alias、alpha/stencil eligible 验收 |
| timeline start/stop | SDL3 `playTimeline/stopTimeline` -> `emotefile::startTimeline/stopTimeline`，attach file 传播 | SDL2 active timeline map | 壳层和基础状态可用 | attach resource、empty-name stop all、parallel/difference flags、start tick 与 loop window 需要对齐 |
| timeline update | SDL3 `emotefile::updateTimelineControl()` 对变量逐帧插值，跳过 selector 变量 | SDL2 `updateTimelineVariables()` 对 parsed timeline variable 插值并 blend | 部分可用 | 变量插值、blend、selector skip、difference/parallel 混合与 SDL3 仍未完整证明 |
| timeline blend | SDL3 `setTimelineBlendRatio()` 是 TODO，`fadeInTimeline()` 近似 play，`fadeOutTimeline()` 近似 stop | SDL2 实现 blendRatio/fade state | SDL2 比本地 SDL3 更“宽” | 目标不是逐字复刻 SDL3 TODO，而是满足 TJS/游戏脚本预期并记录差异 |
| timeline query | `getTimelinePlaying/getLoopTimeline/getTimelineTotalFrameCount/getMainTimelineLabelList/getDiffTimelineLabelList/getPlayingTimelineInfoList/getVariableFrameList` | SDL2 有同名或更详细输出 | 基本可观测 | 输出字段与 SDL3 array/dictionary 形态、label 匹配和 frame dump 需稳定 |
| physics init | SDL3 `initPhysics()` 在 player 层 TODO，但 `emotefile` 有 bust/hair simulator 和 `updatePhysics()` | SDL2 保存 `physicsMetadata`，有 gravity/wind 状态 | 壳层/状态记录 | 真正把 physics metadata 驱动到变量表还没有闭合 |
| wind/outerForce | SDL3 player 层 `setOuterForce/startWind/stopWind` TODO，`HairSwaySimulator` 有 wind 模型 | SDL2 有简化 `windState/outerForces` | 状态可见，物理不等价 | 要补到变量驱动，而不是只保存状态 |
| serialize/unserialize | SDL3 保存 coord/angle/scale 等少量状态 | SDL2 保存 tick/currentFrame/motion/variables/timelines/wind/outerForce/front-state 等更多状态 | SDL2 更宽 | 宽不等于等价；save/load/reload 后 runtime cache、timeline、变量和 draw cache 要验收 |
| object graph | SDL3 `emotefile::GenerateAniTree()` 建 `emoteobject/emotemotion/emotenode/emoteframe` | SDL2 `ParseMotionResource()` 建 `MotionResource/MotionInfo/MotionLayoutInfo/renderMethodIR`，并在 `getResourceDump()` 导出 `sourceIcons/renderMethods/attachmentRefs/objectGraph` | P2 当前 slice 已过自动验收：五个真实资源的 source key 和同资源 nested motion refs 可解释 | 对象图仍不是 SDL3 同构；跨 PSB attach file、selector/timeline runtime 传播还没有完整等价 |
| renderMethod tree | SDL3 `emotenode::progress/draw`、`emotemotion::progress/draw` 消费 frame/content/mesh/stencil/blend | SDL2 build runtime draw resource + renderer command entries | 可见主线可用 | 需要完整消费 `src/mask/bm/opa/bp/cc/zcc/ccc`、type 1/2/3、stencil、blend、order |

## P1 已闭合的 SYSTEM UI 语义链

P1 前手动点击 `tg_未読スキップ`、`tg_既読スキップ` 时，日志证明：

- hit 到了正确控件；
- TJS/system 值有变化；
- `kag.allskip`、`kag.enterSkipOnReadedLabel` 等配置值能更新；
- 差距位于变量驱动的 nested draw rebuild。

P1 后目标语义链已经成立，并由用户手动回归确认 UI 有正确即时反馈：

```text
MotionButton / MotionValue
  -> updateData(newvalue)
    -> setVariable(varname, newvalue)
      -> mplayer.setVariable("tg_未読スキップ/state", 1)
        -> runtime variableValues
          -> child motion "tg_未読スキップ/toggle" local "state"
            -> parameterTick changes
              -> selected frame/layout changes
                -> getCommandList() reports real variable change
                  -> workMotion/calcUpdate/onPaint
                    -> draw-build rebuilds runtime draw resource
                      -> UI visible state writes back
```

本次闭合的核心不是“点击后加一次 hover”或“强制 update”，而是中间这段变量语义：

```text
scoped key: tg_未読スキップ/state
local key:  state
selector key / option labels
nested child motion scope
parameterized frame selection
draw cache invalidation and rebuild
```

这也确认了一个边界：SYSTEM toggle 的即时 UI 写回属于 L4/L5 runtime 语义，持久化写盘只负责刷新后恢复，不应作为当帧 UI 刷新的前置条件。

## 能否说“motion runtime 基本完整”

可以，但必须限定口径：

| 说法 | 是否成立 | 说明 |
|---|---|---|
| “可见 motion 播放主线基本完整” | 成立 | logo/title/正文 motion 已经能读、能播、能画，观感接近 SDL3 |
| “API 壳层基本铺开” | 成立 | SDL2/WASM 函数面比 SDL3 更宽 |
| “完整 Emote runtime 已语义等同 SDL3” | 不成立 | P1 已闭合，但 attach file、timeline、physics、renderMethod、serialize/lifecycle 仍未闭合 |
| “当前 SYSTEM toggle UI 问题说明整个 motion runtime 不成熟” | 不再适用 | 该具体红点已通过手动回归，剩余问题应转入持久化、控件覆盖和更广 SDL3 语义 |
| “SDL3 原生 renderer 可以直接搬来 Web” | 不成立 | SDL3 依赖本地 GL/FBO/tessellation/glReadPixels，WebGL2 需要重写后端 |

推荐后续统一用两个术语：

- **可见播放等同**：play/progress/draw happy path 肉眼接近 SDL3。
- **语义等同 SDL3**：同一 API 在变量、selector、timeline、layout/hit、physics、renderMethod、Layer 更新和生命周期上产生同等可解释结果。

## 迁移完整语义工程表

### P0：语义基线和矩阵

目标：让“等同 SDL3”可测。

| 项 | 内容 |
|---|---|
| SDL3 参考 | `D3DEmote.tjs`、`emoteplayer.cpp` 注册面、`emoteplayerclass.cpp`、`emotefile.cpp/.h` |
| SDL2 落点 | `emoteplayer_stub.cpp`、`ScriptMgnIntf.cpp`、`web-krkr/diag_motion_state.js` |
| 任务 | 生成函数级能力矩阵：API exists / behavior implemented / behavior verified / web-specialized |
| 诊断输出 | resource、object、motion、tick、frame、variables、selectors、timelines、layout、hit、renderMethodIR、renderer command、front-state |
| 验收 | 不要求全绿；要求每个红点能归到 L0-L8 某一层 |

### P1：变量、selector、parameterize 闭合

状态：已完成当前 SYSTEM toggle 路径，并通过用户手动回归。

| 项 | 内容 |
|---|---|
| SDL3 参考函数 | `EmotePlayer::setVariable()`、`emotefile::setVariable()`、`emotefile::getTickByName()`、`emotemotion::getTickByIdx()` |
| SDL2 落点 | `EmotePlayer::setVariable/getVariable()`、`makeEffectiveVariableValues()`、`MotionBuildNestedVariableValues()`、`MotionSelectNodeLayoutFrame()`、`getCommandList()` |
| 已补语义 | UTF-8 canonical key、scoped key、child local key、selector option label、nested motion scope 的统一映射 |
| 禁止路线 | click-time `checkMouseMove()`、press-time forced `update()`、当前游戏控件名硬编码 |
| 验收 | 点击 `tg_未読スキップ` / `tg_既読スキップ` 后 UI 肉眼写回，用户已确认回归成功 |

P1 后还要覆盖的外延转为：

- attached resource 中 selector/variable 是否跨 file 传播；
- save/load/reload 后变量、runtime layout cache、draw cache 是否重建正确。

### P1.5：SYSTEM 设置持久化闭合

状态：已完成当前 SYSTEM 设置点击后的写盘和 reload 回归。

| 项 | 内容 |
|---|---|
| SDL3 参考语义 | 本地文件系统中 `kag.saveSystemVariables()` 通过 `Dictionary.saveStruct` 写系统变量 |
| SDL2/WASM 落点 | `ScriptMgnIntf.cpp` 中 MotionValue kag 值变化后只 request pending save；`EventIntf.cpp` 的 `TVPFlushPendingSystemVariablesSave()` / `TVPDoSaveSystemVariables()` 统一进入 `kag.saveSystemVariables()`；Web 端 `/libsdl/krkrsdl2` IDBFS overlay 承接写入 |
| 已补语义 | UI 值变化后只改 runtime/TJS 值并请求保存；统一 flush 点调用游戏脚本保存；Web 端不手写 `.ksd`；写入后由主线程 `__krkrRequestSavedataSync()` 队列自然 `FS.syncfs(false)` |
| 验收 | `diag_config_write_readback.js --live-motion-probe` 通过；`sync.forcedBeforeReload=false`；`tg_未読スキップ` 使 `kag.allskip/scflags.allskip: 0 -> 1`；`datasc.ksd` / `datasu.ksd` 更新；reload 后保持 |
| 边界 | 持久化只负责刷新后恢复，不参与当帧 UI 写回；`.ksd` 格式仍由游戏脚本生成 |

补充判定：

- 点击后 `LocalFileStream` 写入 `/libsdl/krkrsdl2/datasc.ksd` / `datasu.ksd`，随后 `[IDBFS-SYNC] start/js-finish ok/finish` 闭合。
- reload 启动时 overlay 中的新 `.ksd` 已恢复，savedata seed 对已有文件走 `keep-existing`，不会覆盖回随包旧值。
- reload 期间游戏可按读出的 runtime 值再次重写 `.ksd`，因此 hash 可变化；验收以 runtime 值和 `.ksd` 字段语义保持为准。
- 已有 `.ksd` 的启动期 `MainWindow.saveSystemVariables()` 会被跳过：日志应出现 `stage=initial ... skipped=1` 与 `startup-direct-suppressed`，避免刷新时把已读设置重写回随包初值。
- WASM 不再从浏览器主线程 JS callback 直接调用 wasm finish 函数；页面队列 finish 写入完成计数，pthread run loop 轮询后调用 `FinishedSyncSavedata()`。
- 用户后续手动验收确认：持久化设置后刷新进入主流程不再卡死。这个卡死曾由持久化读回后的启动时序触发，但最终断点在 motion hot path：`ResourceManager.load()` 解析完 `yuzulogo.psb` 后构造过重诊断返回对象，未进入后续 `GFX-LOAD`。修复后 `load()` 仅返回轻量 root/metadata，完整诊断走 `getResourceDump()`，该边界应长期保持。

### P1.6：ResourceManager.load 热路径收窄

状态：已完成，并通过用户手动刷新回归。

| 项 | 内容 |
|---|---|
| SDL3 参考语义 | `ResourceManager::load()` 创建并缓存 `emotefile`，返回运行时资源 root；完整对象图由内部 `emotefile` 持有 |
| SDL2/WASM 落点 | `emoteplayer_resource_runtime.inc::MotionResourceManager::load()` / `getResourceDump()` |
| 已补语义 | `load()` 作为 logo/title/正文 motion 启动 hot path，只返回轻量 root/metadata；不在返回对象里构造 `rendererPrep` 或完整 `motions` 字符串数组 |
| 诊断出口 | 需要 renderer/layout/motion 完整 dump 时调用 `getResourceDump()`，不要借 `load()` 返回值夹带重型诊断 |
| 验收 | 用户手动确认：设置持久化后刷新进入主流程不再卡死；旧断点停在 `load parse-done path=yuzulogo.psb`，修复后可继续进入主流程 |
| 边界 | 这不是 `.ksd` 读写修复；持久化只改变启动时序并暴露 motion hot path 问题 |

### P2：object graph / attach file 对齐

状态：当前结构诊断 slice 已完成，并通过独立自动验收；跨 PSB attach runtime 传播仍留给后续 P4/P7 交叉推进。

目标：减少 SDL2 重建模型和 SDL3 `emotefile` 对象图之间的结构性差距。

| 项 | 内容 |
|---|---|
| SDL3 参考函数 | `emotefile::GenerateAniTree()`、`emotefile::addEmoteFile()`、`emoteobject::findVarByName()`、`emotemotion::getNodeByName()` |
| SDL2 落点 | `ParseMotionResource()`、`MotionResource`、`MotionInfo`、`MotionLayoutInfo`、`renderMethodIR` |
| 已补语义 | `MotionSourceIconInfo`、`MotionAttachmentRefInfo`、`MotionObjectGraphDiag`；resource dump 导出 `sourceIcons`、`attachmentRefs`、`renderMethods`、`objectGraph`；layout dump 增补 source/render/link 字段 |
| 自动验收 | `cd /Users/xiabin/my_work1/web-krkr && node diag_motion_state.js --group resource.objectGraph` |
| 验收结果 | 2026-05-17 通过；`resource.objectGraph.ok=true`；覆盖 `title/main/config/save_ui/extra`；`sourceKeysUnresolvedCount=0`，nested refs 满足 `resolved + external == motionRefCount` |
| 边界 | 当前完成的是对象图可解释、source icon/renderMethod 闭包、同资源 nested refs resolved/external 归类；不是完整 `emotefile::addEmoteFile()` 多文件 attach runtime 变量/timeline 传播 |

### P3：layout/hit/LayerGetter 等价

状态：SYSTEM/config 控件面已闭合；更广 QuickMenu/SAVE/LOAD 长流程仍可继续扩展。

目标：让 click-first Web 输入下的 UI 命中和当前帧 layout 一致。

| 项 | 内容 |
|---|---|
| SDL3 参考 | TJS `AnimKAGLayer` / Layer owner 语义；注意本地 SDL3 C++ `contains()` 仍偏 TODO |
| SDL2 落点 | `containsRaw()`、`contains()`、`getLayerGetter()`、`getLayerMotion()`、`MotionHitShape`、`resolveFrontHit()` |
| 已补语义 | `getLayerMotion/getLayerGetter` raw callback 注册；scoped descendant leaf lookup；front resolve 与 action 相关性；held-button mousemove 支持 slider drag |
| Web 规则 | 保持 click-first，不恢复 passive hover/continuous mousemove；只在按键保持时转发 mousemove |
| SYSTEM/config 验收 | `diag_system_menu.js --scenario=system` 通过；`blockingOk=true`；9 个 probes 覆盖 toggle、slider、tab、back/title/reset；live front-resolve 命中和 action 相关性均为 9/9 |
| 后续外延 | 主流程 QuickMenu save/load/qsave/qload/system/autoplay/skip 的长流程覆盖；更广资源下的 proxy alias、alpha/stencil eligibility、空白区域推进语义 |

### P4：timeline 语义

目标：timeline 不只是状态记录，而是能驱动变量、frame selection 和 draw。

| 项 | 内容 |
|---|---|
| SDL3 参考函数 | `playTimeline()`、`stopTimeline()`、`getTimelinePlaying()`、`emotefile::startTimeline()`、`stopTimeline()`、`updateTimelineControl()` |
| SDL2 落点 | `activeTimelines`、`playTimelineRaw()`、`stopTimelineRaw()`、`updateTimelineVariables()`、`getPlayingTimelineInfoList()` |
| 必补语义 | empty name stop all、attach file 传播、parallel/difference flags、loop begin/end、selector variable skip、timeline frame interpolation |
| 验收 | timeline/fade/blend 后 variable table、selected frame、draw-build 都可观察；save/load 后不漂移 |

### P5：renderMethod/draw pipeline 完整化

目标：从“能看见”走向“renderMethod 语义等价”。

| 项 | 内容 |
|---|---|
| SDL3 参考函数 | `emotenode::progress()`、`emotenode::draw()`、`emotemotion::progress()`、`emotemotion::draw()`、`EmotePlayer::draw()` |
| SDL2 落点 | `MotionBuildRuntimeDrawResource()`、`MotionBuildRendererCommandEntriesFromDrawItems()`、`MotionConsumeRendererCommands()`、CPU-IR / future WebGL backend |
| 必补语义 | texture lookup、source alias、mesh bp/control points、z/order、opa/bm/blend、mask/stencil、type 1/2/3 renderMethod |
| Web 规则 | 不直接搬 SDL3 GL/FBO/tessellation/glReadPixels；WebGL2/CPU 后端自行实现同等语义 |
| 验收 | logo/title/mono_loop/character/system UI 截图接近 SDL3；`texture-missing` 不再出现；type 1/2/3 command 均被消费 |

### P6：physics、wind、outerForce

目标：补角色动态细节，不只支持 UI/静态变量。

| 项 | 内容 |
|---|---|
| SDL3 参考函数 | `emotefile::updatePhysics()`、`HairSwaySimulator`、`BreastJiggleSimulator`、`startWind/stopWind/setOuterForce` 的目标语义 |
| SDL2 落点 | `initPhysics()`、`setPhysicsTimestep()`、`setPhysicsGravity()`、`setPhysicsWind()`、`startWind()`、`stopWind()`、`setOuterForce()` |
| 必补语义 | physics metadata -> runtime variables、hair/bust/parts scale、outer force label/path、wind direction/strength/progress |
| 验收 | 有 physics metadata 的资源能观察到变量变化；无 metadata 时 no-op 且无异常；截图人工验收 |

### P7：serialize/cache/lifecycle

目标：让 runtime 在 save/load、reload、IDBFS overlay、菜单往返后稳定。

| 项 | 内容 |
|---|---|
| SDL3 参考函数 | `serialize()`、`unserialize()`、`ResourceManager::unload/unloadAll()` |
| SDL2 落点 | `serialize/unserialize()`、`restoreVariableState()`、`restoreTimelineState()`、`runtimeLayoutResource`、draw cache、ResourceManager cache |
| 必补语义 | tick/currentFrame、motion/chara、variables、timelines、physics/wind、affine/camera、front-state、runtime draw cache 重新生成 |
| 验收 | save/load/reload 后 SYSTEM 状态、motion state、menu state 不漂移；缓存卸载后不会引用 stale resource |

### P8：退场临时桥和样本特化

目标：把“当前游戏能跑”变成“通用 KRKR motion 语义”。

| 项 | 内容 |
|---|---|
| 当前风险 | `ScriptMgnIntf.cpp` / KAG motion bridge、logo safety-net、static fallback、bring-up flag |
| 必补语义 | motion/video wait 由插件/TJS 原生语义处理；KAGParserEx 只产 raw tag；runtime 不硬编码当前游戏资源名或控件名 |
| 验收 | DRACU-RIOT! 仍绿；至少 1-2 个其它商业 KRKR 包能通过 motion/load/menu 基线 |

## Motion 待做顺序

本文是 motion/runtime 专项文档，不再承担全项目下一步排序。当前全项目下一步已切到 WebGL renderer 接入，CPU-IR 保留为第二语义、fallback 和回归对照；见 `PORTING_STRATEGY_ENGINE.md` 与 `WEBGL_RENDERER_PLAN_2026-05-18.md`。

如果后续回到 motion/runtime SDL3 语义等同，本专项内的待做顺序是：

1. P4 timeline 语义：减少表情/差分/角色 motion 隐患，并利用 P2 的 attachment refs 诊断跨资源传播。
2. P5 renderMethod/draw pipeline：继续提升视觉等价，尤其是 type 1/bp、mask/stencil、blend/order。
3. P7 serialize/cache/lifecycle：把 reload/save/load 后的 runtime graph、draw cache、变量/timeline 恢复收紧。
4. P6 physics/wind/outerForce，按真实包体需求推进。
5. P8 特化退场收尾。

并行但不混同的待做 / 待优化项：

- SAVE/LOAD/SYSTEM 性能：功能正确后单独查卡顿，不用它解释 SYSTEM UI 语义。2026-05-17 已完成第一刀默认 L1 日志降噪；自动回归 `diag_title_click.js` 和 `diag_system_menu.js --scenario=system` 通过。默认 25 秒采样 `/tmp/krkr-default-l1-sample.json` 中项目级调试流为 0，只剩浏览器资源 404/abort；L2 诊断显示 `config.psb` 解析约 8 秒、随后多次 `GFX-LOAD config.psb`，因此下一步应打 hot path 计时和缓存策略，而不是继续把性能问题归咎于 SYSTEM 变量语义或默认 console 输出。
- SKIP 图标生命周期：单独确认进入、动画、退出、隐藏四段状态。
- QuickMenu 长流程覆盖：在 P3 SYSTEM/config 已绿基础上，继续扩展 save/load/qsave/qload/system/autoplay/skip 的自然点击和副作用边界。

这个 motion 专项顺序的理由是：P1/P1.5/P2/P3 已经分别证明当前 SYSTEM 设置线的变量、持久化、对象图诊断、控件命中都成立；等项目回到 SDL3 语义等同时，接下来最值得补的是 timeline 与 renderMethod，因为它们会直接消费 P2 暴露出的 attachment/source/render 结构。

## 验收原则

- 结构日志只能说明链路到达，不能单独证明视觉正确。
- 视觉结论以截图 artifact + 人工/多模态读取为准。
- click-first 是 Web 默认输入契约，不用 hover 修 UI。
- 当前游戏资源名、控件名、坐标不能写进 runtime 修复。
- SDL3 是语义参考，不是 Web renderer 的 drop-in 代码。
