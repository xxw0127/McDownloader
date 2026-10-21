[CmdletBinding()]
param(
    [string]$ToolchainRoot = 'D:\PCL2-reverse\toolchain'
)

$script = Join-Path $PSScriptRoot 'prepare-java.ps1'
& $script -Major 8 -ToolchainRoot $ToolchainRoot
exit $LASTEXITCODE
