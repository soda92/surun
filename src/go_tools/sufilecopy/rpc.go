package main

import (
	"errors"
	"fmt"
	"log"
	"net"
	"net/http"
	"net/rpc"
	"os"
	"path/filepath"
	"strings"

	"golang.org/x/sys/windows/svc"
)

type APIVersion string

func (pa *APIVersion) Get(arg int, reply *string) error {
	*reply = API_VER
	return nil
}

type Service string

func (s *Service) StartService(service_name string, result_service *string) error {
	err := startService(service_name)
	return err
}

func (s *Service) SwitchService(service_name string, result_service *string) error {
	var base_name string
	if service_name == NAME_A {
		base_name = NAME_B
	} else if service_name == NAME_B {
		base_name = NAME_A
	} else {
		log.Fatal("incorrect executable name")
	}
	err := startService(service_name)
	var errstr string
	errstr = fmt.Sprintf("%s", err)

	err = controlService(base_name, svc.Stop, svc.Stopped)
	errstr = fmt.Sprintf("%s %s", errstr, err)
	return errors.New(errstr)
}

func (s *Service) StopService(service_name string, result *string) error {
	err := controlService(service_name, svc.Stop, svc.Stopped)
	return err
}

type Exe string

func (e *Exe) GetName(_ int, name *string) error {
	p, err := os.Executable()
	if err != nil {
		return err
	}
	name2 := filepath.Base(p)
	ext := filepath.Ext(p)
	base := strings.TrimSuffix(name2, ext)
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

	var base string
	err = client.Call("Exe.GetName", 0, &base)
	if err != nil {
		log.Print(err)
	}

	var target_name string
	if base == NAME_A {
		target_name = NAME_B
	} else if base == NAME_B {
		target_name = NAME_A
	} else {
		log.Fatal("incorrect executable name")
	}

	var reply string
	err = client.Call("Service.SwitchService", target_name, &reply)
	if err != nil {
		log.Print(err)
	}
	client.Close()
	return err
}

func RemoteStopService(service_name string) error {
	port := QueryProgramPort()
	client, err := rpc.DialHTTP("tcp", fmt.Sprintf("127.0.0.1:%d", port))
	if err != nil {
		log.Printf("connect to port %d failed: %v", port, err)
		return err
	}

	var reply string
	err = client.Call("Service.StopService", service_name, &reply)
	if err != nil {
		log.Print(err)
	}
	return err
}

func RunServer() {
	api := new(APIVersion)
	rpc.Register(api)

	exe := new(Exe)
	rpc.Register(exe)

	tool := new(Tool)
	rpc.Register(tool)

	service := new(Service)
	rpc.Register(service)

	rpc.HandleHTTP()
	port := QueryAvailablePort()
	l, err := net.Listen("tcp", fmt.Sprintf("127.0.0.1:%d", port))
	if err != nil {
		log.Fatal("listen error:", err)
	}
	go http.Serve(l, nil)
}
