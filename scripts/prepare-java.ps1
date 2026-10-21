[CmdletBinding()]
param(
    [int]$Major = 8,
    [string]$ToolchainRoot = 'D:\PCL2-reverse\toolchain'
)

$ErrorActionPreference = 'Stop'
if ($Major -notin @(8, 17, 21, 25)) {
    throw "Major 必须是 8/17/21/25，当前：$Major"
}

$runtimeName = switch ($Major) {
    8 { 'java-runtime-legacy' }
    21 { 'java-runtime-alpha' }
    17 { 'java-runtime-beta-local' }
    25 { 'java-runtime-epsilon-local' }
}

$dest = Join-Path $ToolchainRoot "java$Major"
New-Item -ItemType Directory -Force $dest | Out-Null
$zip = Join-Path $dest "temurin$Major.zip"

if (-not (Test-Path $zip)) {
    $url = "https://api.adoptium.net/v3/binary/latest/$Major/ga/windows/x64/jdk/hotspot/normal/eclipse"
    Write-Host "下载 Temurin $Major：$url"
    Invoke-WebRequest -Uri $url -OutFile $zip -UseBasicParsing -TimeoutSec 900
}

$java = Get-ChildItem $dest -Recurse -Filter java.exe -ErrorAction SilentlyContinue | Select-Object -First 1
if (-not $java) {
    Expand-Archive -Path $zip -DestinationPath $dest -Force
    $java = Get-ChildItem $dest -Recurse -Filter java.exe -ErrorAction SilentlyContinue | Select-Object -First 1
}
if (-not $java) {
    throw "解压后找不到 java.exe：$dest"
}

$versionOutput = & $java.FullName -version 2>&1 | Out-String
if ($versionOutput -notmatch "version `"1\.$Major|version `"$Major\.") {
    throw "Java 版本不符合预期：$versionOutput"
}

$runtimeRoot = Join-Path $env:APPDATA '.minecraft\runtime'
New-Item -ItemType Directory -Force $runtimeRoot | Out-Null
$link = Join-Path $runtimeRoot $runtimeName
$javaRoot = Split-Path -Parent (Split-Path -Parent $java.FullName)

if (Test-Path $link) {
    $existing = Join-Path $link 'bin\java.exe'
    if (-not (Test-Path $existing)) {
        cmd /c rmdir "$link" | Out-Null
    }
}
if (-not (Test-Path $link)) {
    cmd /c mklink /J "$link" "$javaRoot" | Out-Null
}

Write-Host "Java $Major 已准备：$($java.FullName)"
Write-Host "运行时映射：$link"
