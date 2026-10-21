param(
    [switch]$Quiet
)

$ErrorActionPreference = 'Continue'

$qtDir = 'D:\Qt\5.15.2\mingw81_64'
$mingwDir = 'D:\Qt\Tools\mingw810_64'

$failed = 0

function Report([string]$name, [bool]$ok, [string]$detail) {
    if ($ok) {
        Write-Host "OK   $name $detail"
    } else {
        Write-Host "FAIL $name $detail"
        $script:failed++
    }
}

$qmake = Join-Path $qtDir 'bin\qmake.exe'
if (Test-Path $qmake) {
    $v = & $qmake -v 2>&1 | Out-String
    Report 'qmake' ($v -match '5\.15\.2') ($v.Trim() -replace "`r?`n", ' | ')
} else {
    Report 'qmake' $false "missing $qmake"
}

$gpp = Join-Path $mingwDir 'bin\g++.exe'
if (Test-Path $gpp) {
    $v = (& $gpp --version 2>&1 | Select-Object -First 1) -join ''
    Report 'g++' ($v -match '8\.1\.0') $v
} else {
    Report 'g++' $false "missing $gpp"
}

$make = Join-Path $mingwDir 'bin\mingw32-make.exe'
if (Test-Path $make) {
    $v = (& $make --version 2>&1 | Select-Object -First 1) -join ''
    Report 'mingw32-make' ($v -match 'GNU Make') $v
} else {
    Report 'mingw32-make' $false "missing $make"
}

$ssl = Join-Path $qtDir 'bin\libssl-1_1-x64.dll'
$crypto = Join-Path $qtDir 'bin\libcrypto-1_1-x64.dll'
Report 'openssl-1.1.1' ((Test-Path $ssl) -and (Test-Path $crypto)) "$ssl ; $crypto"

if ($failed -gt 0) {
    Write-Host ""
    Write-Host "缺少 $failed 项。OpenSSL 缺失时运行 scripts\prepare-openssl.ps1。"
    exit 1
}

Write-Host ''
Write-Host '环境检查通过。'
exit 0
