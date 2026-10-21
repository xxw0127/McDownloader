param(
    [switch]$Force
)

$ErrorActionPreference = 'Stop'

$toolRoot = 'D:\PCL2-reverse\toolchain'
$installDir = Join-Path $toolRoot 'openssl-1.1.1w'
$installer = Join-Path $toolRoot 'Win64OpenSSL_Light-1_1_1w.exe'
$url = 'https://slproweb.com/download/Win64OpenSSL_Light-1_1_1w.exe'
$qtBin = 'D:\Qt\5.15.2\mingw81_64\bin'

New-Item -ItemType Directory -Force $toolRoot | Out-Null

if ($Force -or -not (Test-Path $installer)) {
    Write-Host "下载 $url"
    Invoke-WebRequest -Uri $url -OutFile $installer -UseBasicParsing
}

Write-Host "静默安装到 $installDir"
$args = @('/VERYSILENT', '/SUPPRESSMSGBOXES', '/NORESTART', '/SP-', "/DIR=$installDir")
$proc = Start-Process -FilePath $installer -ArgumentList $args -Wait -PassThru
if ($proc.ExitCode -ne 0) {
    throw "OpenSSL 安装器退出码 $($proc.ExitCode)"
}

$srcBin = Join-Path $installDir 'bin'
$libssl = Join-Path $srcBin 'libssl-1_1-x64.dll'
$libcrypto = Join-Path $srcBin 'libcrypto-1_1-x64.dll'
if (-not (Test-Path $libssl) -or -not (Test-Path $libcrypto)) {
    throw "安装后没有找到 DLL：$srcBin"
}

Copy-Item $libssl $qtBin -Force
Copy-Item $libcrypto $qtBin -Force
Write-Host "已复制 OpenSSL DLL 到 $qtBin"
Write-Host "源文件保留在 $srcBin"
exit 0
