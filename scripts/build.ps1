$vs = (.\vswhere -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath)
$name = (.\vswhere -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property displayName)

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

msbuild SuRunVC9.sln /t:clean 1>NUL 2>NUL
msbuild SuRunVC9.sln /t:Rebuild /p:Configuration="x64 Unicode Debug" /p:Platform=x64
msbuild SuRunVC9.sln /t:Rebuild /p:Configuration="SuRun32 Unicode Debug" /p:Platform=Win32
msbuild SuRunVC9.sln /t:Rebuild /p:Configuration="Unicode Debug" /p:Platform=Win32
