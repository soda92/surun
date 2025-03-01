package main

import (
	"log"
	"net/rpc"
	"os"
)

type Path string

func (pa *Path) Get() string {
	p, err := os.Executable()
	if err != nil {
		log.Fatal("err")
	}
	return p
}

func Server() {
	obj := new(Path)
	rpc.Register(obj)
	rpc.HandleHTTP()
	l, err := net.Listen("tcp", ":%d")
	if err != nil {
		log.Fatal("listen error:", err)
	}
	go http.Serve(l, nil)
}
