package main

import (
	"fmt"
	"log"
	"net"
	"net/http"
	"net/rpc"
	"os"
	"path/filepath"

	"golang.org/x/sys/windows/svc"
)

type Service string

func (s *Service) SwitchAB(_ int, result_service *string) error {
	p, err := os.Executable()
	if err != nil {
		return err
	}
	base := filepath.Base(p)
	var target_name string
	if base == NAME_A {
		target_name = NAME_B
	} else if base == NAME_B {
		target_name = NAME_A
	} else {
		log.Fatal("incorrect executable name")
	}
	service_name := base
	err = controlService(service_name, svc.Stop, svc.Stopped)
	if err != nil {
		return err
	}
	startService(target_name)
	return nil
}

type Exe string

func (e *Exe) GetName(_ int, name *string) error {
	p, err := os.Executable()
	if err != nil {
		return err
	}
	base := filepath.Base(p)
	*name = base
	return nil
}

func determineRunningService() (string, error) {
	port := QueryProgramPort()
	client, err := rpc.DialHTTP("tcp", fmt.Sprintf("127.0.0.1:%d", port))
	if err != nil {
		log.Printf("connect to port %d failed: %v", port, err)
		return "", err
	}

	var reply string
	err = client.Call("Exe.GetName", 0, &reply)
	if err != nil {
		log.Print(err)
	}
	return reply, err
}

type Tool string

func (t *Tool) CopyFile(arg CopyArg, bytes_copied *int) error {
	b, err := copy(arg.Source, arg.Destination)
	if err != nil {
		return err
	}
	*bytes_copied = int(b)
	return nil
}

func remoteFileCopy(src, dst string) error {
	port := QueryProgramPort()
	client, err := rpc.DialHTTP("tcp", fmt.Sprintf("127.0.0.1:%d", port))
	if err != nil {
		log.Printf("connect to port %d failed: %v", port, err)
		return err
	}

	var bytes_copied int
	arg := CopyArg{src, dst}
	err = client.Call("Tool.CopyFile", arg, &bytes_copied)
	if err != nil {
		log.Print(err)
	}
	return err
}

func remoteSwitchService() error {
	port := QueryProgramPort()
	client, err := rpc.DialHTTP("tcp", fmt.Sprintf("127.0.0.1:%d", port))
	if err != nil {
		log.Printf("connect to port %d failed: %v", port, err)
		return err
	}

	var reply string
	err = client.Call("Service.SwitchAB", 0, &reply)
	if err != nil {
		log.Print(err)
	}
	return err
}

func RunServer() {
	obj := new(APIVersion)
	rpc.Register(obj)
	rpc.HandleHTTP()
	port := QueryAvailablePort()
	l, err := net.Listen("tcp", fmt.Sprintf("127.0.0.1:%d", port))
	if err != nil {
		log.Fatal("listen error:", err)
	}
	go http.Serve(l, nil)
}
