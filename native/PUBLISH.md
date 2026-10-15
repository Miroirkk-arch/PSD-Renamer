# 一键发布准备

配置已指向 `Miroirkk-arch/PSD-Renamer`。

## 准备一个新版本

1. 在 `native/release-notes/版本号.json` 填写本版的 `zh` 和 `en` 更新条目。
2. 运行：

```powershell
npm run release:prepare -- --version 1.1.5
```

这条命令检查线上版本、同步版本号、编译、运行原生功能和 PSD 保存验证、生成完整 ZIP、检查运行依赖与数据隔离，再把 ZIP 解压到隔离目录重新运行验证。成功后生成：

- `release/PSDNaming-v1.1.5-win-x64.zip`
- `release/prepared-v1.1.5/RELEASE.md`：中英发布说明
- `release/prepared-v1.1.5/SHA256SUMS.txt`
- `release/prepared-v1.1.5/READY.json`：验证结果与 ZIP 校验值

失败时停止；未完成的准备可加 `--resume` 重试。已经成功准备或公开发布的版本不能被该命令覆盖。

## 上传草稿

```powershell
npm run release:draft -- --version 1.1.5
```

此命令需要已登录的 GitHub CLI，或由你在本机配置的 `GH_TOKEN` / `GITHUB_TOKEN`。不要把凭据发到聊天中。没有上传凭据时，保留全部已验证文件，可在已登录的 GitHub 网页上传 ZIP 与 SHA256，并粘贴 RELEASE.md 内容。

上传命令只创建草稿，不会正式发布、删除版本或替换已有附件。使用 API 上传时，DRAFT.json 记录附件是否完整；发生部分失败时，应检查草稿后处理，不要直接再次创建同一版本。

## 正式发布前

检查草稿的版本号、更新说明、附件和校验值；实际下载 ZIP 后确认能运行，再点击 Publish release。另一个干净 Windows 环境的测试仍需要单独进行：隔离目录验证不等于干净电脑验证。

更新时关闭旧版，将新版解压到另一个文件夹。个人字典方案保存在 `%APPDATA%/PSD命名修改`，方向和编号设置保存在用户设置中，不在发布包内。

检查更新仅在主动点击按钮时访问 GitHub；不上传 PSD、字典和本地文件路径。下载通过浏览器完成，不自动覆盖正在运行的程序。

## 项目约定

本项目自有代码采用 MIT 许可证，发布包自动包含 LICENSE 与第三方组件说明。不进行数字签名。

日常开发只在本地修改和验证。仅在用户明确要求上传或发布后，才执行草稿上传或正式发布；准备命令默认不上传。应用源码尚未推送到公开仓库，下次获得上传指令时再整理并提交源码。
