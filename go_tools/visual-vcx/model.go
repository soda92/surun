package main

import "github.com/google/uuid"

type Project struct {
	id           uuid.UUID
	file         string
	name         string
	dependencies []*Project
	depend_uuids []uuid.UUID
}

type Solution struct {
	id       uuid.UUID
	projects []Project
}
