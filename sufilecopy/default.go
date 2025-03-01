package main

func DefaultPort() int {
	return 12534
}

func PortQueryMax() int {
	return DefaultPort() + 100
}
