# 几何工坊 · Geometry Studio

基于 **C++17 + EasyX + GDI+** 的 Windows 简单几何图形绘制系统。
EasyX 管理窗口、消息与画布，GDI+ 提供抗锯齿绘制。

## 下载使用（无需编程环境）

**支持 Windows 10 / 11，x64 电脑。** 不需要安装 Visual Studio、EasyX 或单独的 VC++ 运行库。

- **[下载安装包](https://github.com/wcy2432320756-a11y/wcy/releases/download/v1.0.0/GeometryStudio-1.0.0-Windows-x64-Setup.exe)**：下载后双击，按照向导安装，通过开始菜单启动。可在 Windows 设置中卸载。
- **[下载免安装 ZIP](https://github.com/wcy2432320756-a11y/wcy/releases/download/v1.0.0/GeometryStudio-1.0.0-Windows-x64-Portable.zip)**：解压后双击 `geodraw.exe`。
- [所有版本与校验文件](https://github.com/wcy2432320756-a11y/wcy/releases)

请下载 Release 附件，而非 GitHub 自动生成的 Source code 压缩包。
安装包目前没有代码签名，Windows 可能提示“未知发布者”；请确认来源和 SHA-256 校验值，不要关闭系统安全保护。
安装器为英文向导，软件界面为中文。暂不提供 macOS、Linux 和 Windows 32 位版本。

## 开发者：运行与编译

- 双击 `build\geodraw.exe` 启动。
- 双击 `build.bat` 编译。需要 Visual Studio 2022 Build Tools 的 C++ 工具链与 EasyX x64。
- 构建脚本中的 `VCVARS` 是本机工具链路径；其他电脑需要按安装位置调整。
- 固定客户区大小为 1240 × 800，推荐至少 1280 × 900 的可用桌面空间。

## 操作

| 操作 | 快捷键 |
|---|---|
| 直线 / 矩形 / 椭圆 / 圆 / 三角形 / 铅笔 | 1 / 2 / 3 / 4 / 5 / 6 |
| 撤销 | Ctrl+Z |
| 重做 | Ctrl+Y 或 Ctrl+Shift+Z |
| 导出 PNG | Ctrl+S 或 S |
| 清空（可以撤销） | Delete |
| 切换辅助点阵 | G |
| 切换填充 | F |
| 取消正在绘制的图形 | Esc 或鼠标右键 |

在画布内按下并拖动鼠标左键绘制，释放完成。
圆以按下位置为圆心、拖动距离为半径；其他封闭图形由拖拽框决定。
按住 Shift 可绘制正方形、正圆椭圆框、等宽高三角形框，或水平、垂直、45° 直线。
右侧样式用于接下来绘制的图形，不会修改已有图形。

PNG 使用系统保存对话框选择路径，尺寸为 744 × 574，不含工具栏、辅助网格和拖拽预览。
覆盖已有文件会要求确认，导出成功或失败会显示提示。
文档目前仅保存在内存中，退出前请导出；PNG 不保留可编辑图形数据。
撤销最多保留最近 100 次绘图／清空操作，撤销后新绘制会丢弃重做分支。

## 文件

- `src/main.cpp`：交互、文档历史、画布缓存、PNG 导出。
- `src/ui.h`、`src/ui.cpp`：界面布局、绘制与命中检测。
- `src/shape.h`、`src/shape.cpp`：图形模型与抗锯齿绘制。
- `smoke-test.ps1`：启动一个新的应用窗口，通过真实鼠标与键盘操作绘制示例，并保存界面截图到 `build`。执行时请不要操作鼠标。
- `backup-original-*`：改版前的源码、脚本与可执行文件备份。
