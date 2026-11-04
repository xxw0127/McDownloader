# McDownloader 验收记录

日期：2026-11-04  
范围：Minecraft 原版 + Fabric、Quilt、Forge、NeoForge 安装。OptiFine 不在本期范围。

## 结论

- 原版安装链路通过（2026-10-21 验收证据保留）。
- Fabric/Quilt/Forge/NeoForge 的安装代码、UI 和单元测试已恢复并通过；完整网络安装/启动矩阵尚未在本次复验中重跑，状态标记为“待复验”。
- 2026-11-04 重跑 QtTest：146 个测试全部通过（含网络测试）。

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

2026-11-04 结果：

- `scripts\build.ps1`：退出码 0，生成 `build\release\McDownloader.exe`。
- `scripts\run-tests.ps1 -Network`：18 个套件、146 个测试全部通过。
- UI 冒烟：`--ui-smoke --screenshot` 成功，窗口正常渲染。
- 分发包：`dist\McDownloader-win64.zip`（单顶层 `McDownloader` 文件夹）。

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

## 加载器安装

```powershell
.\release\installer-cli.exe --mc 1.20.1 --dir ... --loader fabric --loader-version 0.20.1
.\release\installer-cli.exe --mc 1.20.1 --dir ... --loader quilt --loader-version <latest>
.\release\installer-cli.exe --mc 1.20.1 --dir ... --loader forge --loader-version 47.3.0
.\release\installer-cli.exe --mc 1.20.1 --dir ... --loader neoforge --loader-version 47.1.100
```

实现要点：

- Fabric/Quilt 通过 profile JSON 生成 `inheritsFrom` 版本，不合并父 JSON。
- Forge 现代版通过 staging + 官方安装器；legacy 通过 ZIP 手动提取。
- NeoForge 显式传客户端参数，禁止无参数运行。
- 已存在版本先移动到 `.mcdownloader-backup`；失败/取消尝试恢复。

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

| 目标 | 安装 | PCL2 启动 | 证据/状态 |
| --- | --- | --- | --- |
| 原版 1.20.1 | 通过 | 通过 | 2026-10-21 验收；`Minecraft 1.20.1`，实例 Java 17 |
| 原版 26.3 | 通过 | 通过 | 2026-10-21 验收；`Minecraft 26.3`，实例 Java 25 |
| 原版 1.7.10 | 通过（基础版本） | 未单独启动 | 2026-10-21 验收 |
| 原版 1.12.2 | 通过（基础版本） | 未单独启动 | 2026-10-21 验收 |
| 原版 1.21.1 | 通过（基础版本） | 未单独启动 | 2026-10-21 验收 |
| Fabric 1.20.1 | 待复验 | 待复验 | 单元测试通过；2026-10-21 曾安装成功 |
| Quilt 1.20.1 | 待复验 | 待复验 | 单元测试通过；2026-10-21 曾安装成功 |
| Forge 1.20.1 | 待复验 | 待复验 | 单元测试通过；2026-10-21 曾安装成功 |
| Forge 1.7.10 / 1.12.2 | 待复验 | 待复验 | 单元测试通过；2026-10-21 曾安装成功 |
| NeoForge 1.20.1 / 1.21.1 | 待复验 | 待复验 | 单元测试通过；2026-10-21 曾安装成功 |

## 清理验证

- 干净目录解压 `McDownloader-win64.zip`
- 使用仅包含 Windows 系统目录的 PATH 启动
- 窗口标题为 `McDownloader`
- `platforms\qwindows.dll`、Qt DLL、QML 运行时均存在

## 已知限制

- 不自动下载 Java；需要用户准备或运行 `scripts\prepare-java8.ps1` / `scripts\prepare-java21.ps1`
- 覆盖旧版本前自动备份，没有交互式确认弹窗
- 下载使用空闲超时和镜像 URL 回退；未实现同一 URL 的多次指数退避重试
- 未实现 manifest 本地缓存（两个版本源都失败时直接报错）
- Forge 版本列表只有 BMCLAPI 一个源；Quilt 列表/profile 只有官方源
- 没有跨进程安装锁
- OptiFine 未包含
