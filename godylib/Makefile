prereq:
	pacman -S --needed mingw-w64-i686-toolchain
	pacman -S --needed mingw32/mingw-w64-i686-go
	pacman -S --needed mingw-w64-ucrt-x86_64-toolchain
	pacman -S --needed ucrt64/mingw-w64-ucrt-x86_64-go

lib32:
	GOROOT=/mingw32/lib/go GOPATH=~/go GOCACHE=~/go/pkg go build -buildmode=c-shared -o lib32.dll ./lib.go

lib64:
	GOROOT=/ucrt64/lib/go GOPATH=~/go GOCACHE=~/go/pkg go build -buildmode=c-shared -o lib.dll ./lib.go

wlib32:
	pwsh -nop msys.ps1 -mingw32 -c "make lib32"

wlib64:
	pwsh -nop msys.ps1 -ucrt64 -c "make lib64"

testenv:
	MSYSTEM="IMGSDN" python env_make.py

testarg:
	pwsh -nop echoarg.ps1 -mingw32 -c "1112"
