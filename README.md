# McDownloader

独立的 Minecraft 下载/安装器（第一期：原版 + Fabric + Quilt + Forge + NeoForge）。
界面为 Qt Quick/QML，风格参考 Rufus 的极简桌面工具；安装完成后由现有 PCL2 或其他启动器负责启动。

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
# 构建 + windeployqt + zip
powershell -ExecutionPolicy Bypass -File scripts\build.ps1 -Deploy -Package

# 全量 QtTest（-Network 额外跑真实版本清单）
powershell -ExecutionPolicy Bypass -File scripts\run-tests.ps1
powershell -ExecutionPolicy Bypass -File scripts\run-tests.ps1 -Network
```

输出：

- 程序：`build\release\McDownloader.exe`
- 分发包：`dist\McDownloader-win64.zip`

本机 Qt 的插件元数据会被 windeployqt 误判为 debug；构建脚本保留 windeployqt 尝试，并自动手工部署 release DLL、QML 和平台插件。

## 命令行安装（验收用）

```powershell
cd tools\installer-cli\build
qmake ..\installer-cli.pro
mingw32-make -j4

.\release\installer-cli.exe --mc 1.20.1 --dir D:\PCL2-reverse\run\downloader-test\.minecraft --loader vanilla
.\release\installer-cli.exe --mc 1.20.1 --dir ... --loader fabric
.\release\installer-cli.exe --mc 1.20.1 --dir ... --loader quilt
.\release\installer-cli.exe --mc 1.20.1 --dir ... --loader forge --loader-version 47.2.0
.\release\installer-cli.exe --mc 1.20.1 --dir ... --loader neoforge --loader-version 47.1.106 --java <java17>\bin\java.exe
.\release\installer-cli.exe --mc 1.21.1 --dir ... --loader neoforge --loader-version 21.1.80 --java <java21>\bin\java.exe
```

旧版 Forge：

```powershell
.\release\installer-cli.exe --mc 1.7.10 --dir ... --loader forge --loader-version 10.13.4.1614
.\release\installer-cli.exe --mc 1.12.2 --dir ... --loader forge --loader-version 14.23.5.2860
```

## Java 版本

安装器从版本 JSON 读取 `javaVersion.majorVersion`，并在安装完成后把对应 Java 绑定到实例：

- `versions\<实例>\java`：指向 Java 根目录的目录链接；
- `versions\<实例>\PCL\Setup.ini`：`VersionArgumentJavaV2:2` + `VersionArgumentJavaSelect`。

这样 PCL2 会给 1.21.1 使用 Java 21，而不是全局默认的 Java 25。安装面板也提供可选的 `java.exe` 路径；留空时自动扫描。

重新安装已有版本时，旧版本会先移动到 `.mcdownloader-backup`，安装失败会尝试恢复。

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

完整验收矩阵和证据见 `docs\acceptance.md`。

## 目录

- `src/core`：路径安全、校验、日志、版本模型与分组、设置
- `src/download`：下载队列、镜像规则、JSON 请求
- `src/installers`：原版与各加载器安装、Java 选择、Staging、ZIP 解包
- `src/models`：QML 使用的模型
- `src/platform`：平台相关路径与进程
- `qml`：界面
- `tests`：QtTest 用例
- `tools/installer-cli`：命令行安装入口，用于验收
- `scripts`：构建、测试、Java/PCL2 沙箱准备
- `third_party/miniz`：ZIP 解包

设计稿与实施计划见上级仓库 `D:\PCL2-reverse\docs\superpowers\`。
