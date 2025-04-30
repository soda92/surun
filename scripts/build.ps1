$vswherePath = Join-Path -Path $PSScriptRoot -ChildPath "vswhere.exe"
$vs = & $vswherePath -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
$name = & $vswherePath -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property displayName

Push-Location "$vs\VC\Auxiliary\Build"

cmd /c "vcvars64.bat & set" |
	ForEach-Object {
		if ($_ -match "=")
		{
			$v = $_.split("=", 2); Set-Item -Force -Path "ENV:\$($v[0])" -Value "$($v[1])"
		}
	}
Pop-Location
Write-Host "$name amd64 Command Prompt variables set." -ForegroundColor Green

$root = Split-Path -Path $PSScriptRoot -Parent
$surun = Join-Path -Path $root -ChildPath "PC/SuRunVC9.sln"
$install_surun = Join-Path -Path $root -ChildPath "PC/InstallSuRunVC9.vcxproj"

# for faster build, comment the next line and change "/t:Rebuild" to "/t:Build"
# & "msbuild.exe" $surun /t:clean 1>NUL 2>NUL
& "msbuild.exe" $surun /t:Build /p:Configuration="x64 Unicode Debug" /p:Platform=x64
& "msbuild.exe" $surun /t:Build /p:Configuration="SuRun32 Unicode Debug" /p:Platform=Win32

& "msbuild.exe" $install_surun /t:Build /p:Configuration="Debug" /p:Platform=Win32