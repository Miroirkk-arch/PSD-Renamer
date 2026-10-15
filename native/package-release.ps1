$ErrorActionPreference = 'Stop'
$taskRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$taskStage = Join-Path $taskRoot 'release\PSD-Renamer'
$taskVersion = (Get-Content -LiteralPath (Join-Path $taskRoot 'package.json') -Raw -Encoding UTF8 | ConvertFrom-Json).version
if ($taskVersion -notmatch '^\d+\.\d+\.\d+$') { throw 'Invalid release version' }
$taskDestination = Join-Path $taskRoot 'release'
$taskBundle = Join-Path $taskDestination ('package-' + [guid]::NewGuid().ToString('N'))
$taskContents = Join-Path $taskBundle 'PSD-Renamer'
New-Item -ItemType Directory -Path $taskContents -Force | Out-Null
$taskRequired = @('PSD-Renamer.exe','LICENSE','THIRD_PARTY_NOTICES.md','dictionary.json','update-source.json','assets','codec','licenses','platforms','imageformats','iconengines','styles','tls','networkinformation')
foreach ($taskItem in $taskRequired) {
    $taskSource = Join-Path $taskStage $taskItem
    if (-not (Test-Path -LiteralPath $taskSource)) { throw "Missing runtime file: $taskItem" }
    Copy-Item -LiteralPath $taskSource -Destination $taskContents -Recurse -Force
}
Get-ChildItem -LiteralPath $taskStage -Filter '*.dll' -File | ForEach-Object { Copy-Item -LiteralPath $_.FullName -Destination $taskContents -Force }
$taskArchive = Join-Path $taskDestination ("PSD-Renamer-v$taskVersion-win-x64.zip")
Add-Type -AssemblyName System.IO.Compression.FileSystem
$taskTemporary = Join-Path $taskDestination ('archive-' + [guid]::NewGuid().ToString('N') + '.zip')
[IO.Compression.ZipFile]::CreateFromDirectory($taskBundle,$taskTemporary,[IO.Compression.CompressionLevel]::Optimal,$false)
Move-Item -LiteralPath $taskTemporary -Destination $taskArchive -Force
$taskBundleAbsolute = [IO.Path]::GetFullPath($taskBundle)
$taskExpectedParent = [IO.Path]::GetFullPath($taskDestination) + [IO.Path]::DirectorySeparatorChar
if (-not $taskBundleAbsolute.StartsWith($taskExpectedParent,[StringComparison]::OrdinalIgnoreCase) -or (Split-Path $taskBundleAbsolute -Leaf) -notmatch '^package-[a-f0-9]{32}$') { throw 'Invalid package cleanup path' }
Remove-Item -LiteralPath $taskBundleAbsolute -Recurse -Force
Write-Output $taskArchive
