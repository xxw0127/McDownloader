[CmdletBinding()]
param(
    [string]$QtDir = 'D:\Qt\5.15.2\mingw81_64',
    [string]$MingwDir = 'D:\Qt\Tools\mingw810_64',
    [switch]$Network
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$qmake = Join-Path $QtDir 'bin\qmake.exe'
$make = Join-Path $MingwDir 'bin\mingw32-make.exe'

$env:PATH = "$(Join-Path $QtDir 'bin');$(Join-Path $MingwDir 'bin');$env:PATH"

$testsBuild = Join-Path $root 'tests\build'
New-Item -ItemType Directory -Force $testsBuild | Out-Null

Push-Location $testsBuild
try {
    & $qmake '..\tests.pro'
    if ($LASTEXITCODE -ne 0) { throw 'qmake tests 失败' }

    & $make -j4
    if ($LASTEXITCODE -ne 0) { throw '编译测试失败' }
}
finally {
    Pop-Location
}

if ($Network) {
    $env:PCL2_RUN_NETWORK_TESTS = '1'
}
else {
    Remove-Item Env:\PCL2_RUN_NETWORK_TESTS -ErrorAction SilentlyContinue
}

$testExe = Join-Path $testsBuild 'release\tst_main.exe'
if (-not (Test-Path $testExe)) {
    throw "未生成测试程序：$testExe"
}

if ($Network) {
    # PowerShell 5.1 的 & 调用在本环境下没有把新环境变量传给子进程，用 cmd 启动。
    cmd /c "`"$testExe`""
    $exitCode = $LASTEXITCODE
}
else {
    & $testExe
    $exitCode = $LASTEXITCODE
}

foreach ($suite in @('paths', 'checksum', 'logger', 'version', 'versionlistmodel', 'download', 'jsonfetcher', 'mirror', 'vanilla', 'installcontroller', 'appcontroller', 'loader', 'java', 'process', 'staging', 'forge', 'legacyforge', 'neoforge')) {
    $file = Join-Path $testsBuild "$suite.txt"
    if (Test-Path $file) {
        $totals = Select-String -Path $file -Pattern '^Totals:' | Select-Object -Last 1
        if ($totals) { Write-Host "$suite : $($totals.Line)" }
    }
}

exit $exitCode
