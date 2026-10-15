$ErrorActionPreference = 'Stop'
$taskRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$taskMapping = (& subst) -join "`n"
if ($taskMapping -notmatch '(?m)^R:') { & subst R: $taskRoot }
if (-not (Test-Path -LiteralPath 'R:\native\app.pro')) { throw 'R: is not the project compilation drive' }
$taskMetadata = Get-Content -LiteralPath (Join-Path $taskRoot 'package.json') -Raw -Encoding UTF8 | ConvertFrom-Json
$taskVersion = $taskMetadata.version
if ($taskVersion -notmatch '^\d+\.\d+\.\d+$') { throw 'Invalid release version' }
$taskHeader = '#pragma once' + "`n" + '#define PSD_APP_VERSION "' + $taskVersion + '"' + "`n"
$taskHeaderPath = Join-Path $PSScriptRoot 'version.h'
if ((Get-Content -LiteralPath $taskHeaderPath -Raw -Encoding UTF8) -ne $taskHeader) { Set-Content -LiteralPath $taskHeaderPath -Value $taskHeader -Encoding utf8 -NoNewline }
$taskRoot = 'R:\'
$taskQt = Join-Path $taskRoot '.native-tools\Qt\6.8.3\mingw_64'
$taskCompiler = Join-Path $taskRoot '.native-tools\Qt\Tools\mingw1310_64\bin'
$env:PATH = "$taskCompiler;$taskQt\bin;$env:PATH"
$taskBuild = Join-Path $taskRoot 'native-build'
New-Item -ItemType Directory -Path $taskBuild -Force | Out-Null
Push-Location $taskBuild
try {
    & "$taskQt\bin\qmake.exe" 'R:\native\app.pro'
    if ($LASTEXITCODE -ne 0) { throw 'qmake failed' }
    & "$taskCompiler\mingw32-make.exe" -j4
    if ($LASTEXITCODE -ne 0) { throw 'C++ compilation failed' }
} finally { Pop-Location }
$taskStage = Join-Path $taskRoot 'release\PSD-Renamer'
New-Item -ItemType Directory -Path $taskStage -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $taskBuild 'PSD-Renamer.exe') -Destination (Join-Path $taskStage 'PSD-Renamer.exe') -Force
& "$taskQt\bin\windeployqt.exe" --release --no-translations --no-opengl-sw --no-system-d3d-compiler (Join-Path $taskStage 'PSD-Renamer.exe')
if ($LASTEXITCODE -ne 0) { throw 'Qt deployment failed' }
$taskCodec = Join-Path $taskStage 'codec'
New-Item -ItemType Directory -Path $taskCodec -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'psd-bridge.cjs') -Destination $taskCodec -Force
Copy-Item -LiteralPath 'C:\Program Files\nodejs\node.exe' -Destination $taskCodec -Force
New-Item -ItemType Directory -Path (Join-Path $taskCodec 'node_modules') -Force | Out-Null
foreach ($taskPackage in @('ag-psd','pako','base64-js')) {
    Copy-Item -LiteralPath (Join-Path $taskRoot "node_modules\$taskPackage") -Destination (Join-Path $taskCodec 'node_modules') -Recurse -Force
}
Copy-Item -LiteralPath (Join-Path $taskRoot 'assets') -Destination $taskStage -Recurse -Force
if (Test-Path -LiteralPath (Join-Path $PSScriptRoot 'licenses')) { Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'licenses') -Destination $taskStage -Recurse -Force }
Copy-Item -LiteralPath (Join-Path $taskRoot 'dictionary.json') -Destination $taskStage -Force
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'update-source.json') -Destination $taskStage -Force
Copy-Item -LiteralPath (Join-Path $taskRoot 'LICENSE') -Destination $taskStage -Force
Copy-Item -LiteralPath (Join-Path $taskRoot 'THIRD_PARTY_NOTICES.md') -Destination $taskStage -Force
Write-Output (Join-Path $taskStage 'PSD-Renamer.exe')
