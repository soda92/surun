# using MSYS2 for both 32bit and 64bit

Golang package: <https://packages.msys2.org/base/mingw-w64-go>

## compile for 32bit

```
pacman -S --needed mingw-w64-i686-toolchain
pacman -S --needed mingw32/mingw-w64-i686-go
set -x GOROOT /mingw32/lib/go
go build -buildmode=c-shared -o lib.dll ./lib.go
```

## compile for 64bit

```
pacman -S --needed mingw-w64-ucrt-x86_64-toolchain
pacman -S --needed ucrt64/mingw-w64-ucrt-x86_64-go
set -x GOROOT /ucrt64/lib/go
go build -buildmode=c-shared -o lib.dll ./lib.go
```