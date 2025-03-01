//go:build windows

package main

import (
	"fmt"
	"log"
	"os"
	"path/filepath"
	"strings"

	"golang.org/x/sys/windows/svc"
)

func main() {
	inService, err := svc.IsWindowsService()
	if err != nil {
		log.Fatalf("failed to determine if we are running in service: %v", err)
	}
	if inService {
		runService(GetSelfServiceName(), false)
		return
	}

	cmd := strings.ToLower(os.Args[1])
	switch cmd {
	case "-debug":
		runService(GetSelfServiceName(), true)
		return
	case "-test":
		QueryProgramPort()
	case "-deploy":
		Deploy()
	case "-init":
		InitServices()
	case "-uninstall":
		RemoveServices()
	default:
		if len(os.Args) != 3 {
			usage()
			return
		}
		source, _ := filepath.Abs(os.Args[1])
		dest, _ := filepath.Abs(os.Args[2])
		remoteFileCopy(source, dest)
	}
}

func usage() {
	fmt.Printf(`usage: -debug, -test, -deploy, -init or:
		supply two path names as source and destination to copy`)
}
