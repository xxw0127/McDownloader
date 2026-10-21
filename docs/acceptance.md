# McDownloader 验收记录

日期：2026-10-21  
范围：只保留 Minecraft 原版下载/安装。Fabric、Quilt、Forge、NeoForge、OptiFine 已移除。

## 结论

原版安装链路通过。QtTest 全部通过，分发包可在干净目录启动，PCL2 可识别并启动原版实例。

## 环境

- Windows 10 x64
- Qt 5.15.2 MinGW 8.1：`D:\Qt\5.15.2\mingw81_64`、`D:\Qt\Tools\mingw810_64`
- OpenSSL 1.1.1w：已复制到 Qt `bin`
- 源码：`D:\PCL2-reverse\downloader`
- 验收 `.minecraft`：`D:\PCL2-reverse\run\downloader-test\.minecraft`
- PCL2 沙箱：`D:\PCL2-reverse\run\pcl2-sandbox`
- Java 8：`D:\PCL2-reverse\toolchain\java8\jdk8u504-b01`
- Java 21：`D:\PCL2-reverse\toolchain\java21\jdk-21.0.12.1+1`

## 构建与测试

```powershell
powershell -ExecutionPolicy Bypass -File scripts\build.ps1 -Deploy -Package
powershell -ExecutionPolicy Bypass -File scripts\run-tests.ps1
powershell -ExecutionPolicy Bypass -File scripts\run-tests.ps1 -Network
```

结果：

- QtTest 13 个套件、96 个测试全部通过（含网络测试）。
- 网络测试覆盖 BMCLAPI → Mojang 回退、请求超时、镜像失败回退。
- 构建产物：`build\release\McDownloader.exe`
- 分发包：`dist\McDownloader-win64.zip`（单顶层 `McDownloader` 文件夹）

## 原版安装

命令行示例：

```powershell
.\release\installer-cli.exe --mc 1.20.1 --dir D:\PCL2-reverse\run\downloader-test\.minecraft
```

安装器会：

1. 获取版本清单和版本 JSON
2. 下载 client jar、libraries、assets、logging 配置
3. 校验 SHA1 和大小
4. 写入 `launcher_profiles.json`
5. 校验版本完整性
6. 绑定实例 Java

## Java 版本

安装器从版本 JSON 读取 `javaVersion.majorVersion`；缺失时按版本回退：

| Minecraft | 需要 Java |
| --- | --- |
| 1.7.10 / 1.12.2 | 8 |
| 1.20.1 | 17 |
| 1.21.1 | 21 |
| 26.3 | 25（版本 JSON 指定） |

安装完成后：

- 创建 `versions\<实例>\java` 目录链接
- 写入 `versions\<实例>\PCL\Setup.ini` 的 `VersionArgumentJavaV2:2` 和 `VersionArgumentJavaSelect`

## 验收结果

| 目标 | 安装 | PCL2 启动 | 证据 |
| --- | --- | --- | --- |
| 原版 1.20.1 | 通过 | 通过 | `Minecraft 1.20.1`，实例 Java 17 |
| 原版 26.3 | 通过 | 通过 | `Minecraft 26.3`，实例 Java 25 |
| 原版 1.7.10 | 通过（基础版本） | 未单独启动 | 版本文件完整 |
| 原版 1.12.2 | 通过（基础版本） | 未单独启动 | 版本文件完整 |
| 原版 1.21.1 | 通过（基础版本） | 未单独启动 | 版本文件完整 |

PCL2 版本列表截图：`D:\PCL2-reverse\run\pcl2-sandbox\pcl2-test-versions.png`

## 清理验证

- 干净目录解压 `McDownloader-win64.zip`
- 使用仅包含 Windows 系统目录的 PATH 启动
- 窗口标题为 `McDownloader`
- `platforms\qwindows.dll`、Qt DLL、QML 运行时均存在

## 已知限制

- 不自动下载 Java；需要用户准备或运行 `scripts\prepare-java8.ps1` / `prepare-java21.ps1`
- 覆盖旧版本前自动备份，没有交互式确认弹窗
- 下载使用空闲超时和镜像 URL 回退；未实现同一 URL 的多次指数退避重试
- 没有跨进程安装锁
- 不再支持任何模组加载器和 OptiFine
