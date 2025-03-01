package main

import (
	"fmt"
	"log"
	"net"
	"net/http"
	"net/rpc"
)

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
