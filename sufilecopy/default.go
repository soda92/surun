package main

const DefaultPort = 12534
const PortQueryMax = DefaultPort + 100

const (
	NAME_A = "sufilecopy_a"
	NAME_B = "sufilecopy_b"
)
const API_VER = "v1"

type APIVersion string

func (pa *APIVersion) Get(arg int, reply *string) error {
	*reply = API_VER
	return nil
}
