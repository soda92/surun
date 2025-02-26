$Env:MSYSTEM = "MINGW32"
$prof = $Env:USERPROFILE
$shell = "bash.exe"
$Env:CHERE_INVOKING=1

& "$prof/scoop/apps/msys2/2024-12-08/usr/bin/$shell" -l $args[0]
