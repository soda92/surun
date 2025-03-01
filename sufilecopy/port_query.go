package main

import (
	"fmt"
	"log"
	"net/rpc"
	"os"
)

func QueryAvailablePort() int {
	port := DefaultPort
	portEnd := PortQueryMax

	for {
		if port == portEnd {
			log.Fatal("no available port")
		}
		_, err := rpc.DialHTTP("tcp", fmt.Sprintf("127.0.0.1:%d", port))
		if err != nil {
			return port
		} else {
			port += 1
		}

	}
}

func QueryProgramPort() int {
	port := DefaultPort
	portEnd := PortQueryMax

	for {
		if port == portEnd {
			log.Fatal("server not found")
		}
		client, err := rpc.DialHTTP("tcp", fmt.Sprintf("127.0.0.1:%d", port))
		if err != nil {
			port += 1
			log.Printf("connect to port %d failed: %v", port, err)
			continue
		}

		var reply string
		err = client.Call("Path.Get", 0, &reply)
		if err != nil {
			port += 1
			log.Print(err)
			continue
		}
		p, _ := os.Executable()
		if reply != p {
			port += 1
			log.Printf("found path: %s, expected: %s", reply, p)
			continue
		}
		fmt.Printf("found port: %d\n", port)
		return port
	}
}
