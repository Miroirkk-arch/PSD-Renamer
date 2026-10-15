$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'build.ps1')
if ($LASTEXITCODE -ne 0) { throw 'Native build failed' }
$taskExe = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\release-native\PSD命名修改.exe'))
Start-Process -FilePath $taskExe
