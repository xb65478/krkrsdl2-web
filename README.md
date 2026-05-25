# 吉里吉里 SDL2 WebAssembly 移植版

本仓库是基于 `krkrsdl2` 的浏览器/WASM 移植快照，用于把吉里吉里运行时移植到网页环境中，并保留当前可运行状态所需的源码、文档和已生成的 `build_web/` 产物。

原始项目 `krkrsdl2` 是吉里吉里 Z 的 SDL2 移植版，可在 macOS、Linux 等支持 SDL2 的平台运行。原项目页面见：

- https://krkrsdl2.github.io/krkrsdl2/

## 重要提示：不建议基于本项目继续构建

本仓库只是当前移植过程的快照和资料留存，不建议看到本仓库的人以它为基础继续做新的 Web/KRKR 运行时开发。GitHub 上已经有进展更快、能力更完整的相关项目，建议优先参考或 fork 这些项目：

- [fenghengzhi/kirikiroid2-web](https://github.com/fenghengzhi/kirikiroid2-web)：相较于本项目，已经包含 WebGL 迁移、较多插件的深度逆向实现等更多能力；如果要继续开发，最建议优先 fork 这个项目。
- [AetherKiri/AetherKiri](https://github.com/AetherKiri/AetherKiri)
- [reAAAq/KrKr2-Next](https://github.com/reAAAq/KrKr2-Next)
- [2468785842/krkr2](https://github.com/2468785842/krkr2)
- [luxiaoling-mc/krkrsdl3](https://github.com/luxiaoling-mc/krkrsdl3)

## 当前定位

本仓库重点面向浏览器运行，不是原版桌面 `krkrsdl2` 的通用发布包。当前快照包含：

- WASM/浏览器运行相关源码；
- `build_web/krkrsdl2.js` 与 `build_web/krkrsdl2.wasm` 当前生成产物；
- FFmpeg 音视频解码接入代码；
- MotionPlayer/EmotePlayer 运行时代码；
- 当前移植状态、兼容性矩阵和推进文档。

## 构建说明

完整本地构建前，需要先准备 FFmpeg WASM 静态库。否则 CMake 仍可生成 WASM，但音频/视频解码会以占位实现方式编译，不是完整 FFmpeg 解码路径。

常规构建流程：

```bash
./build_ffmpeg_wasm.sh
emcmake cmake -S . -B build_web -DKRKRSDL2_EMSCRIPTEN_ENABLE_THREADS=ON -DCMAKE_BUILD_TYPE=MinSizeRel
cmake --build build_web -j 8
```

如果目标包体需要 WMV/WMA/VC-1 等实验性格式支持，需要显式启用 GPL 编解码器配置：

```bash
KRKRSDL2_FFMPEG_ENABLE_GPL_CODECS=1 ./build_ffmpeg_wasm.sh
emcmake cmake -S . -B build_web -DKRKRSDL2_EMSCRIPTEN_ENABLE_THREADS=ON -DCMAKE_BUILD_TYPE=MinSizeRel
cmake --build build_web -j 8
```

`external/ffmpeg/install` 由 `build_ffmpeg_wasm.sh` 生成，不随仓库提交。其他人从仓库拉取后若要复现完整音视频能力，需要先运行该脚本。

## MotionPlayer/EmotePlayer 参考来源

本仓库的 `src/plugins/emoteplayer/` 运行时实现参考了 `krkrsdl3` 中 MotionPlayer/EmotePlayer 的语义、生命周期和接口行为，再按当前 WASM/浏览器运行环境做了适配。

参考仓库：

- https://github.com/luxiaoling-mc/krkrsdl3.git

## 商业游戏运行说明

本仓库用于移植和兼容性验证，不承诺支持未经修改的商业游戏完整运行。若只是运行原始商业游戏，通常应优先考虑 Wine、Kirikiroid2 或其他成熟运行方案。

## 许可证

`src` 目录中的吉里吉里 SDL2 源码遵循 MIT 许可证，详情见 `LICENSE`。

本项目包含多个第三方组件，各组件的许可证以其所在目录中的许可证文件为准。启用 FFmpeg GPL 编解码器配置时，需要额外注意对应编解码器和发布方式的许可证边界。
