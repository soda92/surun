package main

import (
	"bufio"
	"fmt"
	"os"
	"strings"
)

func readline() string {
	reader := bufio.NewReader(os.Stdin)
	text, _ := reader.ReadString('\n')
	text = strings.TrimSuffix(text, "\n")
	text = strings.TrimSuffix(text, "\r")
	return text
}

func main() {
	args := strings.Join(os.Args, " ")

	fmt.Print("press enter to continue")
	readline()
	start(args)
}
