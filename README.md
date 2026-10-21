# McDownloader

独立的 Minecraft 原版下载/安装器。界面为 Qt Quick/QML；安装完成后由 PCL2 或其他启动器启动。

当前只保留原版安装：Fabric、Quilt、Forge、NeoForge、OptiFine 相关功能和代码已移除。

## 工具链

- Qt 5.15.2（`D:\Qt\5.15.2\mingw81_64`）
- MinGW 8.1 64-bit（`D:\Qt\Tools\mingw810_64`）
- OpenSSL 1.1.1（Qt Network 需要，`scripts\prepare-openssl.ps1` 安装）

环境检查：

```powershell
powershell -ExecutionPolicy Bypass -File scripts\check-env.ps1
```

## 构建、测试、打包

```powershell
# 构建 + windeployqt/手工部署 + zip
powershell -ExecutionPolicy Bypass -File scripts\build.ps1 -Deploy -Package

# 全量 QtTest（-Network 额外跑真实版本清单）
powershell -ExecutionPolicy Bypass -File scripts\run-tests.ps1
powershell -ExecutionPolicy Bypass -File scripts\run-tests.ps1 -Network
```

输出：

- 程序：`build\release\McDownloader.exe`
- 分发包：`dist\McDownloader-win64.zip`（解压后只有一个顶层 `McDownloader` 文件夹）

本机 Qt 的插件元数据会被 windeployqt 误判为 debug；构建脚本保留 windeployqt 尝试，并自动手工部署 release DLL、QML 和平台插件。

## 命令行安装（验收用）

```powershell
cd tools\installer-cli\build
qmake ..\installer-cli.pro
mingw32-make -j4

.\release\installer-cli.exe --mc 1.20.1 --dir D:\PCL2-reverse\run\downloader-test\.minecraft
.\release\installer-cli.exe --mc 1.20.1 --dir ... --java <java17>\bin\java.exe
```

## Java 版本

安装器从版本 JSON 读取 `javaVersion.majorVersion`，并在安装完成后把对应 Java 绑定到实例：

- `versions\<实例>\java`：指向 Java 根目录的目录链接；
- `versions\<实例>\PCL\Setup.ini`：`VersionArgumentJavaV2:2` + `VersionArgumentJavaSelect`。

安装面板提供可选的 `java.exe` 路径；留空时自动扫描。重新安装已有版本时，旧版本会先移动到 `.mcdownloader-backup`，安装失败会尝试恢复。

准备 Java：

```powershell
powershell -ExecutionPolicy Bypass -File scripts\prepare-java8.ps1
powershell -ExecutionPolicy Bypass -File scripts\prepare-java21.ps1
```

## PCL2 沙箱

```powershell
powershell -ExecutionPolicy Bypass -File scripts\prepare-pcl2-sandbox.ps1 -Force -RegisterFolder
```

只复制 PCL2 程序与 `PCL` 配置目录，不修改原 `D:\PCL2`；测试目录会加入 PCL2 文件夹列表。

完整验收记录见 `docs\acceptance.md`。

## 目录

- `src/core`：路径安全、校验、日志、版本模型与分组、设置
- `src/download`：下载队列、镜像规则、JSON 请求
- `src/installers`：原版安装计划、校验、Java 选择
- `src/models`：QML 使用的版本模型
- `src/platform`：平台路径与进程
- `qml`：界面
- `tests`：QtTest 用例
- `tools/installer-cli`：命令行安装入口，用于验收
- `scripts`：构建、测试、Java/PCL2 沙箱准备
