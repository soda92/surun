package main

const API_VER = "v1"
type APIVersion string

func (pa *APIVersion) Get(arg int, reply *string) error {
	*reply = API_VER
	return nil
}
