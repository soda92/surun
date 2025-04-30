package main

// #include <wchar.h>
import "C"
import (
	"fmt"

	"golang.org/x/sys/windows"
)

//export MyFunction
func MyFunction(p *C.wchar_t) {
	fmt.Print(windows.UTF16PtrToString((*uint16)(p)))
}

//export MyFunction2
func MyFunction2(p int) {
	fmt.Printf("%d\n", p)
}

func main() {}
