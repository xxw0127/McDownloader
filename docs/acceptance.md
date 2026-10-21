# 第一期验收记录

日期：2026-10-21  
范围：原版、Fabric、Quilt、Forge、NeoForge（OptiFine 不在第一期）

## 结论

第一期验收通过。所有已安装版本都被 PCL2 沙箱识别；除 OptiFine 外的原版与加载器均已通过 PCL2 启动到游戏主界面。

## 环境

- Windows 10 x64
- Qt 5.15.2 MinGW 8.1：`D:\Qt\5.15.2\mingw81_64`、`D:\Qt\Tools\mingw810_64`
- OpenSSL 1.1.1w：已复制到 Qt `bin`
- 安装器源码：`D:\PCL2-reverse\downloader`
- 验收 `.minecraft`：`D:\PCL2-reverse\run\downloader-test\.minecraft`
- PCL2 沙箱：`D:\PCL2-reverse\run\pcl2-sandbox`
- Java 8：`D:\PCL2-reverse\toolchain\java8\jdk8u504-b01`
- Java 21：`D:\PCL2-reverse\toolchain\java21\jdk-21.0.12.1+1`

## 构建与测试

```powershell
powershell -ExecutionPolicy Bypass -File scripts\build.ps1 -Deploy -Package
powershell -ExecutionPolicy Bypass -File scripts\run-tests.ps1
```

网络测试：

```powershell
powershell -ExecutionPolicy Bypass -File scripts\run-tests.ps1 -Network
```

验收结果：

- QtTest 全部通过（路径、SHA1、日志、版本分组、下载、镜像、JSON、原版计划、安装校验、AppController、Fabric/Quilt、Java、进程、Staging、Forge、Legacy Forge、NeoForge）。
- 网络测试覆盖 BMCLAPI → Mojang 回退、请求超时和镜像失败回退；302 重定向未做自动化测试。
- 构建产物：`build\release\PCL2Downloader.exe`
- 打包产物：`dist\PCL2Downloader-win64.zip`

## Java 版本策略

安装器根据版本 JSON 的 `javaVersion.majorVersion` 选择 Java；缺失时按 Minecraft 版本回退：

| Minecraft | 需要 Java |
| --- | --- |
| 1.7.10 / 1.12.2 | 8 |
| 1.20.1 | 17 |
| 1.21.1 | 21 |
| 26.3 | 25（版本 JSON 指定） |

安装完成后，安装器会：

1. 在 `versions\<实例>\java` 创建指向所选 Java 的目录链接；
2. 在 `versions\<实例>\PCL\Setup.ini` 写入 `VersionArgumentJavaV2:2` 和 `VersionArgumentJavaSelect`。

因此 PCL2 即使全局 Java 选择指向 Java 25，也会对 1.21.1 使用 Java 21。Java 缺失时安装仍完成，但日志会提示需要手动准备对应 Java；可用 `scripts\prepare-java8.ps1` / `scripts\prepare-java21.ps1` 准备。

## 覆盖与回滚

- 原版、Fabric、Quilt 重新安装时，旧版本目录会先移动到 `.pcl2downloader-backup`；安装失败或取消会尝试恢复旧版本。
- Forge、NeoForge 使用 staging，合并时先备份旧版本；复制失败会回滚已替换的版本目录。
- 备份不会自动删除，便于用户回退。

## 安装命令

所有命令都在 `tools\installer-cli\build` 下执行：

```powershell
.\release\installer-cli.exe --mc 1.20.1 --dir D:\PCL2-reverse\run\downloader-test\.minecraft --loader vanilla
.\release\installer-cli.exe --mc 26.3   --dir ... --loader vanilla

.\release\installer-cli.exe --mc 1.20.1 --dir ... --loader fabric
.\release\installer-cli.exe --mc 1.20.1 --dir ... --loader quilt

.\release\installer-cli.exe --mc 1.20.1 --dir ... --loader forge --loader-version 47.2.0
.\release\installer-cli.exe --mc 1.7.10 --dir ... --loader forge --loader-version 10.13.4.1614
.\release\installer-cli.exe --mc 1.12.2 --dir ... --loader forge --loader-version 14.23.5.2860

.\release\installer-cli.exe --mc 1.20.1 --dir ... --loader neoforge --loader-version 47.1.106 --java <java17>\bin\java.exe
.\release\installer-cli.exe --mc 1.21.1 --dir ... --loader neoforge --loader-version 21.1.80  --java <java21>\bin\java.exe
```

## PCL2 沙箱

```powershell
powershell -ExecutionPolicy Bypass -File scripts\prepare-pcl2-sandbox.ps1 -Force -RegisterFolder
```

脚本只复制 `Plain Craft Launcher 2.exe` 和 `PCL` 目录，不修改原 `D:\PCL2`，并把测试文件夹加入 PCL2 的文件夹列表。

版本列表截图：`D:\PCL2-reverse\run\pcl2-sandbox\pcl2-test-versions.png`

## 验收矩阵

| 目标 | 安装 | PCL2 启动 | 证据 |
| --- | --- | --- | --- |
| 原版 1.20.1 | 通过：64 个库、3575 个资源对象 | 通过 | Java 进程 PID 23592，`Minecraft 1.20.1`，日志 18:06:17 |
| 原版 26.3 | 通过 | 通过 | PID 20668，`Minecraft 26.3`，日志 16:51 |
| 原版 1.7.10（Forge 父版本） | 通过 | 通过（经 Forge 1.7.10） | PID 11604，`Minecraft 1.7.10`，日志 16:54 |
| Fabric 0.20.1 / 1.20.1 | 通过 | 通过 | PID 3148，`Minecraft* 1.20.1`，日志 17:56:43 |
| Quilt 0.20.0-beta.9 / 1.20.1 | 通过 | 通过 | PID 5140，`Minecraft* 1.20.1`，日志 18:08:48 |
| Forge 47.2.0 / 1.20.1 | 通过 | 通过 | PID 16616，`Minecraft Forge* 1.20.1`，日志 16:52:36 |
| NeoForge 47.1.106 / 1.20.1 | 通过 | 通过 | PID 12672，`Minecraft NeoForge* 1.20.1`，日志 18:04:22 |
| Forge 10.13.4.1614 / 1.7.10 | 通过（手动解包） | 通过 | PID 11604，日志 16:54:21 |
| Forge 14.23.5.2860 / 1.12.2 | 通过（手动解包） | 通过 | PID 24408，日志 16:53:45 |
| NeoForge 21.1.80 / 1.21.1 | 通过 | 通过（Java 21） | PID 21040，`Minecraft NeoForge* 1.21.1`，日志 17:53:53 |

取消、镜像失败、请求超时和 SHA1 错误由 QtTest 的本地 HTTP 服务器覆盖；没有依赖外网状态。

## PCL2 版本列表

沙箱版本列表同时显示：

- 26.3、1.21.1、1.20.1、1.12.2、1.7.10
- Fabric 0.20.1
- Quilt 0.20.0-beta.9
- Forge 47.2.0
- NeoForge 47.1.106
- Forge 10.13.4.1614
- Forge 14.23.5.2860
- NeoForge 21.1.80

## 已知限制

- OptiFine 属于第二期。
- Forge 1.12.2 使用安装器 ZIP 内 `version.json` + `maven/` 手动安装，不运行 processors；已实测可进入主菜单。
- NeoForge 1.21.1 安装器需要 Java 21，并且处理器库必须能下载；安装器会先从 BMCLAPI 预下载 `install_profile.json` 中的库，失败会给出明确错误。
- 未构建 Android 版本；QML 与核心层已按可移植方式拆分。
