package main

import (
	"fmt"
	"log"
	"net"
	"net/http"
	"net/rpc"
	"os"
)

type Path string

func (pa *Path) Get(arg *int, reply *string) error {
	p, err := os.Executable()
	if err != nil {
		return err
	}
	*reply = p
	return nil
}

func RunServer() {
	obj := new(Path)
	rpc.Register(obj)
	rpc.HandleHTTP()
	port := QueryAvailablePort()
	l, err := net.Listen("tcp", fmt.Sprintf("127.0.0.1:%d", port))
	if err != nil {
		log.Fatal("listen error:", err)
	}
	go http.Serve(l, nil)
}
