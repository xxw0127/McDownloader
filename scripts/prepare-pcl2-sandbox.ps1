[CmdletBinding()]
param(
    [string]$SourcePcl = 'D:\PCL2',
    [string]$Destination = 'D:\PCL2-reverse\run\pcl2-sandbox',
    [string]$MinecraftDir = 'D:\PCL2-reverse\run\downloader-test\.minecraft',
    [string]$FolderName = 'Test',
    [switch]$RegisterFolder,
    [switch]$Force
)

$ErrorActionPreference = 'Stop'

if (-not (Test-Path $SourcePcl)) {
    throw "PCL2 源目录不存在：$SourcePcl"
}
if (-not (Test-Path (Join-Path $MinecraftDir 'versions'))) {
    throw "测试 .minecraft 目录不存在：$MinecraftDir"
}

if (Test-Path $Destination) {
    if (-not $Force) {
        throw "目标沙箱已存在：$Destination（加 -Force 覆盖）"
    }
    Remove-Item $Destination -Recurse -Force
}
New-Item -ItemType Directory -Force $Destination | Out-Null

$sourceExe = Join-Path $SourcePcl 'Plain Craft Launcher 2.exe'
$sourcePclDir = Join-Path $SourcePcl 'PCL'
if (-not (Test-Path $sourceExe) -or -not (Test-Path $sourcePclDir)) {
    throw "PCL2 源目录结构不符合预期：$SourcePcl"
}

Copy-Item $sourceExe $Destination -Force
Copy-Item $sourcePclDir (Join-Path $Destination 'PCL') -Recurse -Force

$sandboxExe = Join-Path $Destination 'Plain Craft Launcher 2.exe'
Copy-Item $sandboxExe "$sandboxExe.bak" -Force

$setupPath = Join-Path $Destination 'PCL\Setup.ini'
$minecraft = $MinecraftDir.TrimEnd('\') + '\'
$setup = @"
WindowHeight:616
WindowWidth:981
LaunchFolderSelect:$minecraft
SystemSystemUpdate:0
"@
[IO.File]::WriteAllText($setupPath, $setup, (New-Object Text.UTF8Encoding($false)))

if ($RegisterFolder) {
    $backup = 'D:\PCL2-reverse\toolchain\registry-PCL-backup.reg'
    New-Item -ItemType Directory -Force (Split-Path -Parent $backup) | Out-Null
    if (-not (Test-Path $backup)) {
        reg export 'HKCU\Software\PCL' $backup /y | Out-Null
    }

    $entry = "$FolderName>$minecraft"
    $current = (Get-ItemProperty 'HKCU:\Software\PCL' -ErrorAction SilentlyContinue).LaunchFolders
    if ([string]::IsNullOrEmpty($current)) {
        $newValue = $entry
    }
    elseif ($current -notlike "*$entry*") {
        $newValue = $current + '|' + $entry
    }
    else {
        $newValue = $current
    }
    Set-ItemProperty 'HKCU:\Software\PCL' -Name LaunchFolders -Value $newValue
}

Write-Host "PCL2 沙箱已准备：$Destination"
Write-Host "游戏目录：$minecraft"
