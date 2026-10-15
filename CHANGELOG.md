# 更新记录 / Changelog

## v1.3.0 — 2026-10-15

- 新增英语和日语界面，与中文一起可在顶部直接切换，并记住语言选择。
- 完成按钮、词库、图层列表、方向后缀、编号设置、保存确认、删除确认和状态提示翻译。
- 切换界面语言不修改图层名称、用户词库或未保存的 PSD 改动，并保留图层列表的替换输入。
- 工具栏根据语言文字长度调整控件宽度，窄窗口自动换行；同时检查深浅色模式。
- 工具及可执行文件统一命名为 PSD-Renamer，清理旧 Electron 依赖、历史重复目录与反馈入口。
- 增加翻译覆盖、占位符一致性、英日界面布局及语言切换数据保持测试。

- Add English and Japanese interfaces alongside Chinese, with instant language switching and a saved preference.
- Translate buttons, dictionary management, layer lists, side suffixes, numbering, save/delete confirmations, and status messages.
- Switching languages preserves layer names, user dictionaries, unsaved PSD edits, and pending replacement inputs.
- Adapt control widths to translated text and wrap the toolbar in narrow windows; verify light and dark modes.
- Unify the application and executable name as PSD-Renamer; remove obsolete Electron dependencies, duplicate folders, and feedback entry points.
- Add translation coverage, placeholder consistency, English/Japanese layout, and document-preservation tests.

## v1.2.0 — 2026-10-15

- 新增 Ctrl+F 图层列表：树状结构、文件夹优先显示、单击编辑名称、多选、全选切换和批量替换。
- 主界面新增图层搜索框，保留命中项的父文件夹并自动展开；清空后恢复展开状态。
- 词库支持完全匹配和包含匹配，应用前预览改名前后名称；匹配方式随方案保存。
- 新增独立编号设置窗口，支持数字或字母编号，分隔符原样保留，留空不添加分隔符。
- 文件夹可连同内部内容删除，删除前显示名称与子项数量并再次确认。
- 保存与另存为均先展示所有改动；使用新的文件名，保留源 PSD。
- 精简工具栏与图层列表，优化图标、名称编辑框、行分隔线和窄窗口布局。
- 免费开源，项目自有代码采用 MIT 许可；PSD 处理在本地完成，仅手动检查更新时联网。

- Add a Ctrl+F layer list with a folder-first tree, single-click name editing, multi-selection, select-all toggle, and batch replacement.
- Add toolbar layer search that retains and expands matching ancestors and restores expansion when cleared.
- Add exact and contains dictionary matching, a before/after rename preview, and per-profile matching preferences.
- Add a dedicated numbering settings dialog with numeric or alphabetic sequences and literal separators, including empty strings and spaces.
- Allow deleting groups with their contents after a confirmation showing the name and descendant counts.
- Review all changes before Save or Save As; write a new file and preserve the source PSD.
- Simplify toolbar and layer-list controls; improve icons, inline editors, row separators, and narrow-window layouts.
- Free and open source under MIT for original project code. PSD processing is offline; only manual update checks contact GitHub.


## v1.1.4 · 2026-10-14

首个公开 Windows x64 版本。 / First public Windows x64 release.

- 可编辑英语字典和多套自定义方案。 / Editable English dictionary and custom naming profiles.
- 自定义方向后缀、分割符、中轴偏差和应用范围。 / Custom suffixes, separator, center tolerance, and layer/group scope.
- 数字／字母编号及再次点击撤销。 / Numeric/alphabetic numbering and second-click undo.
- 树状图层管理、选择、拖拽、新建与删除。 / Layer tree, selection, drag-and-drop, creation and deletion.
- 缩略图、可缩放画布、显隐控制及预览开关。 / Thumbnails, zoomable canvas, visibility and preview switches.
- 深浅色模式、改动确认和另存新 PSD。 / Light/dark themes, change review, and saving to a new PSD.
- 缩略图与画布开关位于方向后缀设置后。 / Preview switches follow direction settings in the toolbar.
- 手动检查 GitHub Releases 更新。 / Manual update checks via GitHub Releases.

[下载 / Download](https://github.com/Miroirkk-arch/PSD-Renamer/releases/tag/v1.1.4)
