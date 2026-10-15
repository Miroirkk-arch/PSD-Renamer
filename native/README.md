# PSD-Renamer

界面、图层树、缩略图、拖拽和画布由 C++ / Qt Widgets 实现。程序不加载 Electron、Chromium 或 WebView。PSD 编解码使用独立的 Node.js + ag-psd 工具，只在打开和保存时运行，随后退出。原始图层像素存放在临时目录，通过内存映射按需读取；缩略图缓存上限为 4 MiB。

`npm run build` 使用项目 `.native-tools/Qt` 中的 Qt 6.8.3 和 MinGW 13.1，输出 `release/PSD-Renamer/PSD-Renamer.exe` 及配套 DLL。构建脚本使用临时 R: 盘映射规避 MinGW 对中文安装路径的处理问题。目录整体可移动，exe 所在目录的 `codec`、`assets`、Qt DLL 和插件必须随程序保留。

`npm run dev` 构建并打开原生版。本仓库仅包含当前原生版源码。

测试入口：生成 `node tests/native-fixture.cjs`，执行原生 exe 的 `--self-test <样例.psd绝对路径>`，检查 `test-results/smoke.json`，再运行 `node --test tests/native-codec.test.cjs` 验证另存文件像素、蒙版及图层属性。`--benchmark <PSD绝对路径>` 仅打开和缩放文件，不修改源 PSD，结果写入 `test-results/benchmark.json`。

Qt DLL 动态链接；相应许可证随发布目录 `licenses` 提供。Qt 源码可从 https://download.qt.io/archive/qt/6.8/6.8.3/single/ 获取。第三方 PSD 编解码库及 Node.js 的许可证位于其随附目录。

## 构建环境 / Build environment

- Windows x64; Node.js 24.14.0 at `C:\Program Files\nodejs\node.exe`.
- Qt 6.8.3 MinGW x64 at `.native-tools/Qt/6.8.3/mingw_64`.
- MinGW 13.1 at `.native-tools/Qt/Tools/mingw1310_64`.
- Run `npm ci`, then `npm run build` from the repository root. The build script temporarily uses drive `R:`; it must be available or already mapped to this checkout.
- Generated runtime files are written to `release/PSD-Renamer`. Use `npm run package:release` to produce the distributable ZIP in `release`.
- SVG icon sources are committed. Regenerating them with `node native/build-icons.cjs` is optional.
