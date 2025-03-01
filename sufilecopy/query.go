package main

import (
	"fmt"
	"log"
	"net/rpc"
)

func QueryAvailablePort() int {
	port := 12534
	portEnd := port + 100

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
	port := 12534
	portEnd := port + 100
	found := false

	for {
		if port == portEnd {
			log.Fatal("server not found")
		}
		client, err := rpc.DialHTTP("tcp", fmt.Sprintf("127.0.0.1:%d", port))
		if err != nil {
			// log.Fatal("dialing:", err)
			port += 1
			continue
		}

		
	}

}
