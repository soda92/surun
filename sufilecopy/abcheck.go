package main

import (
	"log"
)

func GetTargetName(running_name string) string {
	if running_name == "sufilecopy_a" {
		return "sufilecopy_b"
	} else if running_name == "sufilecopy_b" {
		return "sufilecopy_a"
	}
	log.Fatalf("service name %s not recognized", running_name)
	return ""
}
