package main

import (
	"os"
	"path/filepath"
	"strings"
)

func GetSelfServiceName() string {
	p, _ := os.Executable()
	name := filepath.Base(p)
	ext := filepath.Ext(p)
	base := strings.TrimSuffix(name, ext)
	return base
}
