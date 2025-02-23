package main

import (
	"fmt"
	"os"
	"strings"
)

func check(e error) {
	if e != nil {
		panic(e)
	}
}

func ParseSolution(file string) Solution {
	dat, err := os.ReadFile(file)
	check(err)
	content := string(dat)
	lines := strings.Split(content, "\n")
	fmt.Println(lines[2])
	var ret Solution
	return ret
}
