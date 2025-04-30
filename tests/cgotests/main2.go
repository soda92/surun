//go:build windows
// +build windows

package main

/*
#include <wchar.h>
#include <stdbool.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>

// demo implementation
wchar_t rdr[] = L"abcαβγ";
const wchar_t * Tolk_DetectScreenReader() {
    return rdr;
}

// demo implementation
bool Tolk_Output(const wchar_t *str, bool interrupt) {
    fflush(stdout);
    int mode = _setmode(_fileno(stdout), _O_U16TEXT);
    int rv = wprintf(L"Tolk_Output: %ls\n", str);
    fflush(stdout);
    _setmode(_fileno(stdout), mode);
    return rv >= 0;
}
*/
import "C"

import (
	"fmt"

	"golang.org/x/sys/windows"
)

func WCharPtrToString(p *C.wchar_t) string {
	return windows.UTF16PtrToString((*uint16)(p))
}

func WCharPtrFromString(s string) (*C.wchar_t, error) {
	p, err := windows.UTF16PtrFromString(s)
	return (*C.wchar_t)(p), err
}

func TolkDetectScreenReader() string {
	return WCharPtrToString(C.Tolk_DetectScreenReader())
}

func TolkOutput(str string, interrupt bool) (bool, error) {
	pstr, err := WCharPtrFromString(str)
	if err != nil {
		return false, err
	}
	ok := C.Tolk_Output(pstr, C.bool(interrupt))
	return bool(ok), nil
}

func main() {
	rdr := TolkDetectScreenReader()
	fmt.Println("TolkDetectScreenReader:", rdr)

	ok, err := TolkOutput("xyzχψω", false)
	if !ok || err != nil {
		fmt.Println(ok, err)
		return
	}
}

/*

Output:

>go run wchar.go
TolkDetectScreenReader: abcαβγ
Tolk_Output: xyzχψω
>

*/
