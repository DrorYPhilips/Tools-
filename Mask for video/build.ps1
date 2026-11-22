# Builds MaskVideo.exe and makes the portable release folder .\dist:
#   MaskVideo.exe, User Manual.html, ffmpeg.exe, ffmpeg-LICENSE.txt, READ ME.txt and
#   ".NET Framework 4.8 installer\ndp48-x86-x64-allos-enu.exe".
# Copy that folder anywhere and run MaskVideo.exe.
#
# The third-party files are not in git. Put them in .\tools first:
#   tools\ffmpeg.exe                   from https://www.gyan.dev/ffmpeg/builds/
#                                      (ffmpeg-release-essentials.zip, file bin\ffmpeg.exe)
#   tools\ffmpeg-LICENSE.txt           the LICENSE file from the same zip (ffmpeg is GPL)
#   tools\ndp48-x86-x64-allos-enu.exe  .NET Framework 4.8 offline installer from Microsoft
#                                      (https://go.microsoft.com/fwlink/?linkid=2088631)
#
# Usage:  powershell -ExecutionPolicy Bypass -File build.ps1

$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot

dotnet build -c Release
if ($LASTEXITCODE -ne 0) { exit 1 }

$dist = Join-Path $PSScriptRoot 'dist'
New-Item -ItemType Directory -Force $dist | Out-Null
Copy-Item 'bin\Release\net48\MaskVideo.exe' $dist -Force

$netDir = Join-Path $dist '.NET Framework 4.8 installer'
$extra = @(
    @{ From = 'tools\ffmpeg.exe';                  To = $dist },
    @{ From = 'tools\ffmpeg-LICENSE.txt';          To = $dist },
    @{ From = 'tools\ndp48-x86-x64-allos-enu.exe'; To = $netDir }
)
foreach ($f in $extra) {
    if (Test-Path $f.From) {
        New-Item -ItemType Directory -Force $f.To | Out-Null
        Copy-Item $f.From $f.To -Force
    } else {
        Write-Warning "$($f.From) not found. See the top of build.ps1."
    }
}

# User manual: one HTML file with the pictures inside, so it works alone and offline.
# The source is docs\UserManual.html with the pictures in docs\images\.
$manual = Get-Content 'docs\UserManual.html' -Raw -Encoding UTF8
$manual = [regex]::Replace($manual, 'src="images/([^"]+\.png)"', {
    param($m)
    $bytes = [IO.File]::ReadAllBytes((Join-Path $PSScriptRoot ('docs\images\' + $m.Groups[1].Value)))
    'src="data:image/png;base64,' + [Convert]::ToBase64String($bytes) + '"'
})
[IO.File]::WriteAllText((Join-Path $dist 'User Manual.html'), $manual, (New-Object Text.UTF8Encoding $false))

$readMe = @'
Video Mask - hide areas of a video (blur, pixelate, black box)

HOW TO START
  Double-click MaskVideo.exe. Nothing needs to be installed on Windows 10
  (version 1903 or newer) and Windows 11. Keep the files of this folder together.

FILES
  MaskVideo.exe        The program.
  User Manual.html     How to use the program. Opens in any web browser.
  ffmpeg.exe           Reads and writes the video. Keep it next to MaskVideo.exe.
  ffmpeg-LICENSE.txt   The license of ffmpeg (GPL). ffmpeg: https://ffmpeg.org
                       This build: https://www.gyan.dev/ffmpeg/builds/
  .NET Framework 4.8 installer\ndp48-x86-x64-allos-enu.exe
                       Only needed if MaskVideo.exe does not start (Windows 7 SP1,
                       Windows 8.1 or an old Windows 10). Run it once to install
                       .NET Framework 4.8 from Microsoft. It needs no internet.
'@
($readMe -replace "`r?`n", "`r`n") | Set-Content -Encoding UTF8 (Join-Path $dist 'READ ME.txt')

Get-ChildItem $dist -Recurse -File |
    Format-Table @{ n = 'File'; e = { $_.FullName.Substring($dist.Length + 1) } },
                 @{ n = 'Size (KB)'; e = { [int]($_.Length / 1KB) } } -AutoSize
