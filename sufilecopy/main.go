package main

import "time"

func main() {
	RunServer()
	time.Sleep(1 * time.Second)
	QueryProgramPort()
}
