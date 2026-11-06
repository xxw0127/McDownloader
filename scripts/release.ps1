[CmdletBinding()]
param(
    [string]$Version = '1.0.0',
    [switch]$SkipTests,
    [switch]$Push
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$zip = Join-Path $root 'dist\McDownloader-win64.zip'
$asset = Join-Path $root "dist\McDownloader-$Version-win64.zip"
$tag = "v$Version"

Push-Location $root
try {
    & (Join-Path $PSScriptRoot 'build.ps1') -Deploy -Package
    if ($LASTEXITCODE -ne 0) { throw '构建失败' }

    if (-not $SkipTests) {
        & (Join-Path $PSScriptRoot 'run-tests.ps1') -Network
        if ($LASTEXITCODE -ne 0) { throw '测试失败' }
    }

    if (-not (Test-Path $zip)) { throw "缺少分发包：$zip" }
    Copy-Item $zip $asset -Force
    Write-Host "发布物：$asset"

    if (git tag --list $tag) {
        Write-Warning "标签 $tag 已存在，未重新创建"
    } else {
        git -c user.name=openCode -c user.email=opencode@local tag -a $tag -m "McDownloader $Version"
        Write-Host "已创建标签：$tag"
    }

    if ($Push) {
        git push origin main
        git push origin $tag
        Write-Host "已推送 main 和 $tag"
    } else {
        Write-Host "未推送；需要发布时运行:git push origin main; git push origin $tag"
    }
}
finally {
    Pop-Location
}
