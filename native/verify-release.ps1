param([Parameter(Mandatory=$true)][string]$Archive,[Parameter(Mandatory=$true)][string]$Destination)
$ErrorActionPreference='Stop'
$taskRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$taskExpected=[IO.Path]::GetFullPath((Join-Path $taskRoot 'native-build'))+[IO.Path]::DirectorySeparatorChar
$taskDestination=[IO.Path]::GetFullPath($Destination)
if (-not $taskDestination.StartsWith($taskExpected,[StringComparison]::OrdinalIgnoreCase) -or (Test-Path -LiteralPath $taskDestination)) {throw 'Invalid or existing verification directory'}
Add-Type -AssemblyName System.IO.Compression.FileSystem
[IO.Compression.ZipFile]::ExtractToDirectory([IO.Path]::GetFullPath($Archive),$taskDestination)
