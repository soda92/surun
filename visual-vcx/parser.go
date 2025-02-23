package main

import (
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
	current  string
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
	p.current = p.contents[p.lineno]
	return p.contents[p.lineno]
}

func (p *parser) SkipWhiteLines() {
	for {
		if strings.TrimSpace(p.GetLine()) == "\ufeff" {
			p.Advance()
		} else if strings.TrimSpace(p.GetLine()) == "" {
			p.Advance()
		} else {
			break
		}
	}
}

func (p *parser) Advance() {
	p.lineno += 1
}

func (p *parser) ParseSolution() Solution {
	var ret Solution

	p.SkipWhiteLines()
	if p.GetLine()[:len("Microsoft")] == "Microsoft" {
		p.Advance()
	}
	if p.GetLine()[:2] == "# " {
		p.Advance()
	}

	for {
		line := p.GetLine()
		if line[:len("Project")] == "Project" {
			a, sid := p.ParseProject()
			ret.id = sid
			ret.projects = append(ret.projects, a)
			p.Advance()
		} else {
			break
		}
	}
	ret.projects = LinkDeps(ret.projects)
	return ret
}

func LinkDeps(p []Project) []Project {
	for _, v := range p {
		v.dependencies = append(v.dependencies, nil)
	}
	return p
}

func GetString(s string) string {
	s = strings.TrimSpace(s)
	s = strings.TrimLeft(s, "\"")
	s = strings.TrimRight(s, "\"")
	return s
}

func TrimBrace(s string) string {
	s = strings.TrimSpace(s)
	s = strings.TrimLeft(s, "{")
	s = strings.TrimRight(s, "}")
	return s
}

func (p *parser) ParseProject() (Project, uuid.UUID) {
	var ret Project
	var sid uuid.UUID

	line := p.GetLine()
	rest := line[len("Project(\"{"):]
	sid = uuid.MustParse(rest[:36])
	arr := strings.Split(strings.Split(rest, "=")[1], ",")
	ret.name = GetString(arr[0])
	ret.file = GetString(arr[1])
	ret.id = uuid.MustParse(TrimBrace(GetString(arr[2])))
	p.Advance()
	ret.depend_uuids = p.ParseDepends()
	p.Advance() // endproject
	return ret, sid
}

func (p *parser) ParseDepends() []uuid.UUID {
	var ret []uuid.UUID
	for {
		line := p.GetLine()
		line = strings.TrimSpace(line)
		if line == "EndProjectSection" {
			break
		}
		p.Advance()
	}
	p.Advance()
	return ret
}
