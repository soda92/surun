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

func GetSelfServiceName() string {
	p, _ := os.Executable()
	name := filepath.Base(p)
	ext := filepath.Ext(p)
	base := strings.TrimSuffix(name, ext)
	return base
}

func main() {
	inService, err := svc.IsWindowsService()
	if err != nil {
		log.Fatalf("failed to determine if we are running in service: %v", err)
	}
	if inService {
		runService(GetSelfServiceName(), false)
		return
	}
	if len(os.Args) < 2 {
		usage()
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
	case "copy":
		if len(os.Args) != 4 {
			usage()
			return
		}
		source, err := filepath.Abs(os.Args[2])
		if err != nil {
			log.Fatal(err)
		}
		dest, err := filepath.Abs(os.Args[3])
		if err != nil {
			log.Fatal(err)
		}
		stat, err := os.Stat(dest)
		if err != nil {
			log.Fatal(err)
		}
		if stat.IsDir() {
			dest += "/" + filepath.Base(source)
		}
		err = remoteFileCopy(source, dest)
		if err != nil {
			log.Fatal(err)
		}
		log.Print("copy succeeded")
	default:
		usage()
		return
	}
}

func usage() {
	fmt.Printf(`usage: -debug, -test, -deploy, -init or "copy [A] [B]"`)
}
