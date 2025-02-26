package main

// #cgo LDFLAGS: -Lbuild -lSuRun
// #include "main.h"
import "C"

func main(){
	C._WinMain(nil, nil, nil, 0);
}