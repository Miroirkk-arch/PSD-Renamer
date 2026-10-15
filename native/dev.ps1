$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'build.ps1')
if ($LASTEXITCODE -ne 0) { throw 'Native build failed' }
$taskExe = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\release\PSD-Renamer\PSD-Renamer.exe'))
Start-Process -FilePath $taskExe
