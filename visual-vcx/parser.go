package main

import (
	"fmt"
	"os"
	"strings"

	"github.com/google/uuid"
)

func check(e error) {
	if e != nil {
		panic(e)
	}
}

type parser struct {
	file     string
	lineno   int
	contents []string
}

func NewParser(file string) parser {
	var p parser
	dat, err := os.ReadFile(file)
	check(err)
	content := string(dat)
	lines := strings.Split(content, "\n")
	p.file = file
	p.contents = lines
	p.lineno = 0
	return p
}

func (p *parser) GetLine() string {
	return p.contents[p.lineno]
}

func (p *parser) Advance() {
	p.lineno += 1
}

func (p *parser) ParseSolution() Solution {
	var ret Solution
	if p.GetLine() == "" {
		p.Advance()
	}

	if p.GetLine()[:len("Microsoft")] == "Microsoft" {
		p.Advance()
	}
	if p.GetLine()[:2] == "# " {
		p.Advance()
	}

	if p.GetLine()[:len("Project")] == "Project" {
		a, sid := p.ParseProject()
		ret.id = sid
		ret.projects = append(ret.projects, a)
	}
	return ret
}

func (p *parser) ParseProject() (Project, uuid.UUID) {
	var ret Project
	var id uuid.UUID
	return ret, id
}
