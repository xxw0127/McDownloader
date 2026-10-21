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

$testExe = Join-Path $testsBuild 'release\tst_main.exe'
if (-not (Test-Path $testExe)) {
    throw "未生成测试程序：$testExe"
}

$marker = Join-Path $testsBuild 'network.enabled'
if ($Network) {
    '1' | Set-Content $marker -Encoding ASCII
}
else {
    Remove-Item $marker -Force -ErrorAction SilentlyContinue
}

Get-ChildItem $testsBuild -Filter '*.txt' -ErrorAction SilentlyContinue | Remove-Item -Force -ErrorAction SilentlyContinue

& $testExe
$exitCode = $LASTEXITCODE
Remove-Item $marker -Force -ErrorAction SilentlyContinue

foreach ($suite in @('paths', 'checksum', 'logger', 'version', 'versionlistmodel', 'download', 'jsonfetcher', 'mirror', 'vanilla', 'installcontroller', 'appcontroller', 'java', 'process')) {
    $file = Join-Path $testsBuild "$suite.txt"
    if (Test-Path $file) {
        $totals = Select-String -Path $file -Pattern '^Totals:' | Select-Object -Last 1
        if ($totals) { Write-Host "$suite : $($totals.Line)" }
    }
}

exit $exitCode
