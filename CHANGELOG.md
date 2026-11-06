# Changelog

## 1.0.0 - 2026-11-06

首个可用版本。

### 安装

- 原版 Minecraft：版本 JSON、client jar、libraries、assets、logging 配置、launcher_profiles.json
- Fabric / Quilt：profile 安装，不运行安装器
- Forge：现代版本走 staging + 官方 installer，legacy 版本走 ZIP 手动提取
- NeoForge：1.20.1 使用 `forge` 工件，1.20.2+ 使用 `neoforge` 工件
- 安装完成后把 Java 绑定到 `versions\<实例>\java` 和 PCL2 `Setup.ini`

### 下载

- BMCLAPI + Mojang 版本清单并行请求、合并去重
- 版本清单离线缓存：`cache/version_manifest.json`
- 同一 URL 失败按 1/2/4 秒指数退避重试，再切换下一个镜像
- 下载文件 SHA1 + 大小校验；`.part` 写入，校验后原子重命名
- 支持取消；取消保留 `.part`
- 镜像：BMCLAPI、Mojang、Forge/Fabric/NeoForge Maven 回退

### 安全与数据保护

- 版本 ID / 实例名过滤 `..`、路径分隔符、盘符和 Windows 保留名
- 拒绝程序目录和 Windows 系统目录作为 `.minecraft`
- 覆盖旧版本前移动到 `.mcdownloader-backup`，失败/取消尝试恢复
- 父版本不完整时拒绝静默覆盖
- 配置/日志目录不可写时回退到用户配置目录

### 界面

- Qt Quick/QML，版本列表虚拟化，四分组折叠
- 加载器选择、文件夹选择、Java 选择、进度/错误提示
- 单顶层 `McDownloader` 目录的绿色分发包

### 已知限制

- 不自动下载 Java
- Forge 版本列表只有 BMCLAPI 一个源；Quilt 列表/profile 只有官方源
- 没有跨进程安装锁
- 不包含 OptiFine
- 加载器完整网络安装/启动矩阵尚未在 1.0.0 重新验收
