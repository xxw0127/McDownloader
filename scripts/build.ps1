[CmdletBinding()]
param(
    [string]$QtDir = 'D:\Qt\5.15.2\mingw81_64',
    [string]$MingwDir = 'D:\Qt\Tools\mingw810_64',
    [switch]$Deploy,
    [switch]$Package
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$qmake = Join-Path $QtDir 'bin\qmake.exe'
$make = Join-Path $MingwDir 'bin\mingw32-make.exe'
$windeployqt = Join-Path $QtDir 'bin\windeployqt.exe'

foreach ($tool in @($qmake, $make)) {
    if (-not (Test-Path $tool)) {
        throw "缺少工具：$tool"
    }
}

$env:PATH = "$(Join-Path $QtDir 'bin');$(Join-Path $MingwDir 'bin');$env:PATH"

$build = Join-Path $root 'build'
New-Item -ItemType Directory -Force $build | Out-Null

Push-Location $build
try {
    & $qmake '..\downloader.pro'
    if ($LASTEXITCODE -ne 0) { throw 'qmake 失败' }

    & $make -j4
    if ($LASTEXITCODE -ne 0) { throw 'mingw32-make 失败' }
}
finally {
    Pop-Location
}

$exe = Join-Path $build 'release\McDownloader.exe'
if (-not (Test-Path $exe)) {
    throw "未生成可执行文件：$exe"
}

$sslFiles = @('libssl-1_1-x64.dll', 'libcrypto-1_1-x64.dll')
$missingSsl = @()
foreach ($name in $sslFiles) {
    if (-not (Test-Path (Join-Path $QtDir "bin\$name"))) {
        $missingSsl += $name
    }
}
if ($missingSsl.Count -gt 0) {
    Write-Warning "Qt bin 缺少 OpenSSL DLL：$($missingSsl -join ', ')。先运行 scripts\prepare-openssl.ps1。"
}

if ($Deploy -or $Package) {
    if (-not (Test-Path $windeployqt)) {
        throw "缺少 windeployqt：$windeployqt"
    }

    $release = Join-Path $build 'release'

    # 本机 windeployqt 会把 release 插件误判为 debug 并拒绝部署；
    # 这里直接复制运行所需的 Qt DLL/QML/插件，确保干净目录可启动。
    if (Test-Path $windeployqt) {
        $deployArgs = @('--qmldir', (Join-Path $root 'qml'), '--release', '--no-translations',
                        '--compiler-runtime', '--no-plugins', $exe)
        Start-Process -FilePath $windeployqt -ArgumentList $deployArgs -Wait -NoNewWindow -ErrorAction SilentlyContinue | Out-Null
        # 本机 windeployqt 会把 release 插件误判为 debug 并以非 0 退出；
        # 后面的手工部署会补齐 DLL/QML/插件，因此这里不能把它的退出码当成构建失败。
        $global:LASTEXITCODE = 0
    }

    Copy-Item (Join-Path $QtDir 'bin\Qt5*.dll') $release -Force
    foreach ($name in @('libEGL.dll', 'libGLESv2.dll', 'd3dcompiler_47.dll', 'opengl32sw.dll')) {
        $source = Join-Path $QtDir "bin\$name"
        if (Test-Path $source) { Copy-Item $source $release -Force }
    }
    foreach ($name in @('libgcc_s_seh-1.dll', 'libstdc++-6.dll', 'libwinpthread-1.dll')) {
        Copy-Item (Join-Path $MingwDir "bin\$name") $release -Force
    }
    foreach ($name in $sslFiles) {
        $source = Join-Path $QtDir "bin\$name"
        if (Test-Path $source) { Copy-Item $source $release -Force }
    }

    foreach ($qmlDir in @('QtQuick.2', 'QtQuick', 'QtQml', 'QtGraphicalEffects')) {
        Copy-Item (Join-Path $QtDir "qml\$qmlDir") $release -Recurse -Force
    }
    foreach ($pluginDir in @('platforms', 'styles', 'imageformats')) {
        $target = Join-Path $release $pluginDir
        New-Item -ItemType Directory -Force $target | Out-Null
        Copy-Item (Join-Path $QtDir "plugins\$pluginDir\*.dll") $target -Force
    }

    if (-not (Test-Path (Join-Path $release 'Qt5Core.dll'))) {
        throw '部署后缺少 Qt5Core.dll'
    }
}

if ($Package) {
    $dist = Join-Path $root 'dist'
    $stage = Join-Path $dist 'McDownloader'
    New-Item -ItemType Directory -Force $dist | Out-Null
    if (Test-Path $stage) { Remove-Item $stage -Recurse -Force }
    New-Item -ItemType Directory -Force $stage | Out-Null

    $zip = Join-Path $dist 'McDownloader-win64.zip'
    if (Test-Path $zip) { Remove-Item $zip -Force }

    $release = Join-Path $build 'release'
    Copy-Item (Join-Path $release 'McDownloader.exe') $stage -Force
    Copy-Item (Join-Path $release '*.dll') $stage -Force
    Get-ChildItem $release -Directory |
        Where-Object { $_.Name -ne 'logs' } |
        ForEach-Object { Copy-Item $_.FullName $stage -Recurse -Force }

    # 保留顶层 McDownloader 目录，解压后不会把 Qt/QML 子目录散落在下载目录。
    Compress-Archive -Path $stage -DestinationPath $zip -Force
    Write-Host "打包完成：$zip"
}

Write-Host "构建完成：$exe"
