if ($args[0] -eq "-mingw32") {
    $Env:MSYSTEM = "MINGW32"
}

if ($args[0] -eq "-ucrt64") {
    $Env:MSYSTEM = "UCRT64"
}

& "${Env:USERPROFILE}/scoop/apps/msys2/2024-12-08/usr/bin/bash.exe" -l $args[1..$args.Length]
