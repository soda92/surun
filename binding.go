package main

// #cgo LDFLAGS: -Lbuild -lSuRun
// #define UNICODE
// #define _UNICODE
// #include "main.h"
// #include <wchar.h>
import "C"

import(
	"golang.org/x/sys/windows"
)

func WCharPtrToString(p *C.wchar_t) string {
	return windows.UTF16PtrToString((*uint16)(p))
}

func WCharPtrFromString(s string) (*C.wchar_t, error) {
	p, err := windows.UTF16PtrFromString(s)
	return (*C.wchar_t)(p), err
}

func start(arg string) {
	p, _ := WCharPtrFromString(arg);

	C._WinMain(nil, nil, p, 0);
}