package main

import "github.com/google/uuid"

type Project struct {
	file         string
	name         string
	dependencies []Project
}

type Solution struct {
	id       uuid.UUID
	projects []Project
}
