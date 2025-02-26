$Env:MSYSTEM = "MINGW32"
$prof = $Env:USERPROFILE
$shell = "bash.exe"

& "$prof/scoop/apps/msys2/2024-12-08/usr/bin/$shell" -c $args[0]
