package main

import (
	"os"
)

type Path string

func (pa *Path) Get(arg int, reply *string) error {
	p, err := os.Executable()
	if err != nil {
		return err
	}
	*reply = p
	return nil
}
