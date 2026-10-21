# PCL2 下载器

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

## 构建

```powershell
cd build
qmake ..\downloader.pro
mingw32-make -j4
```

## 测试

```powershell
cd tests\build
qmake ..\tests.pro
mingw32-make -j4
.\release\tst_main.exe
```

## 命令行安装（验收用）

```powershell
cd tools\installer-cli\build
qmake ..\installer-cli.pro
mingw32-make -j4
.\release\installer-cli.exe --mc 1.20.1 --dir D:\PCL2-reverse\run\downloader-test\.minecraft --loader vanilla
```

网络集成测试需要显式开启：

```powershell
$env:PCL2_RUN_NETWORK_TESTS = "1"
```

## 目录

- `src/core`：路径安全、校验、日志、版本模型与分组
- `src/download`：下载队列、镜像规则、JSON 请求
- `src/installers`：原版与各加载器的安装流程
- `src/models`：QML 使用的模型
- `src/platform`：平台相关路径与进程
- `qml`：界面
- `tests`：QtTest 用例
- `tools/installer-cli`：命令行安装入口，用于验收

设计稿与实施计划见上级仓库 `D:\PCL2-reverse\docs\superpowers\`。
