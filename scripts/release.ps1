[CmdletBinding()]
param(
    [string]$Version = '1.0.0',
    [switch]$SkipTests,
    [switch]$Push,
    [switch]$Publish
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

    if ($Publish) {
        $cred = "protocol=https`nhost=github.com`n`n" | git credential fill 2>$null
        $token = ($cred | Where-Object { $_ -like 'password=*' } | Select-Object -First 1) -replace '^password=',''
        if (-not $token) { throw 'GCM 没有返回 github.com 凭据，无法创建 Release' }

        $remote = (git remote get-url origin).Trim()
        if ($remote -notmatch 'github\.com[:/]([^/]+)/([^/]+?)(?:\.git)?$') {
            throw "无法从 origin 解析 GitHub 仓库：$remote"
        }
        $owner = $matches[1]
        $repo = $matches[2]
        $api = "https://api.github.com/repos/$owner/$repo"
        $headers = @{
            Authorization = "Bearer $token"
            Accept = 'application/vnd.github+json'
            'X-GitHub-Api-Version' = '2022-11-28'
            'User-Agent' = 'McDownloader-release'
        }

        $release = $null
        try {
            $release = Invoke-RestMethod -Uri "$api/releases/tags/$tag" -Headers $headers -Method Get
        } catch {
            if ($_.Exception.Response.StatusCode.value__ -ne 404) { throw }
            $log = Get-Content (Join-Path $root 'CHANGELOG.md') -Raw
            $start = $log.IndexOf("## $Version")
            if ($start -lt 0) { throw "CHANGELOG 中找不到 $Version 段落" }
            $next = $log.IndexOf("`n## ", $start + 10)
            if ($next -lt 0) { $next = $log.Length }
            $notes = $log.Substring($start, $next - $start).Trim()
            $payload = @{
                tag_name = $tag
                name = "McDownloader $Version"
                body = $notes
                draft = $false
                prerelease = $false
                generate_release_notes = $false
            } | ConvertTo-Json -Depth 3
            $release = Invoke-RestMethod -Uri "$api/releases" -Headers $headers -Method Post `
                -ContentType 'application/json; charset=utf-8' `
                -Body ([Text.Encoding]::UTF8.GetBytes($payload))
            Write-Host "已创建 Release：$($release.html_url)"
        }

        $assetName = Split-Path $asset -Leaf
        $assets = Invoke-RestMethod -Uri "$api/releases/$($release.id)/assets" -Headers $headers -Method Get
        if ($assets | Where-Object { $_.name -eq $assetName }) {
            Write-Host "Release 资产已存在：$assetName"
        } else {
            $upload = $release.upload_url -replace '\{\?name,label\}', "?name=$assetName"
            Invoke-RestMethod -Uri $upload -Headers $headers -Method Post -ContentType 'application/zip' -InFile $asset | Out-Null
            Write-Host "已上传 Release 资产：$assetName"
        }
        Write-Host "Release 页面：$($release.html_url)"
    }
}
finally {
    Pop-Location
}
