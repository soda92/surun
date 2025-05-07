package main

import (
	"golang.org/x/sys/windows"
	"runtime"
	"syscall"
	"unsafe"
)

func start(arg string) {
	arch := runtime.GOARCH
	var surun syscall.Handle
	if arch == "amd64" {
		surun, _ = syscall.LoadLibrary("SuRun.dll")
	} else {
		surun, _ = syscall.LoadLibrary("SuRun32.dll")
	}
	_WinMain, _ := syscall.GetProcAddress(surun, "_WinMain")

	p2, _ := windows.UTF16PtrFromString(arg)
	p := uintptr(unsafe.Pointer(p2))

	syscall.SyscallN(_WinMain, 0, 0, p, 0)
}
