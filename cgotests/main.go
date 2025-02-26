package main

import (
	"fmt"
	"unicode/utf16"
	"unsafe"
)

/*
#include <stdlib.h>
#include <wchar.h>

void process_wchar_string(wchar_t* wstr) {
	if (wstr) {
		wprintf(L"Received wchar_t string from Go: %ls\n", wstr);
	}
}
*/
import "C"

func main1() {
	goString := "Hello, 世界!"
	utf16Slice := utf16.Encode([]rune(goString))

	cString := (*C.wchar_t)(C.malloc(C.size_t((len(utf16Slice)+1)*2))) // +1 for null terminator, *2 for size of wchar_t
	if cString == nil {
		panic("Failed to allocate memory")
	}
	defer C.free(unsafe.Pointer(cString))

	for i, r := range utf16Slice {
		*(*C.wchar_t)(unsafe.Pointer(uintptr(unsafe.Pointer(cString)) + uintptr(i*2))) = C.wchar_t(r)
	}
	*(*C.wchar_t)(unsafe.Pointer(uintptr(unsafe.Pointer(cString)) + uintptr(len(utf16Slice)*2))) = 0 // Null terminate

	C.process_wchar_string(cString)

	fmt.Println("C function call complete.")
}