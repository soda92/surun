package main

import (
	"log"
	"strings"
)

func GetTargetName(running_name string) string {
	if strings.Contains(running_name, ".exe") {
		running_name = running_name[:len(running_name)-4]
	}
	if running_name == "sufilecopy_a" {
		return "sufilecopy_b"
	} else if running_name == "sufilecopy_b" {
		return "sufilecopy_a"
	}
	log.Fatalf("service name %s not recognized", running_name)
	return ""
}
