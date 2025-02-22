all: build

configure:
	cmake --preset windows-only

build: configure
	cmake --build build
	pwsh -c "cp build/compile_commands.json ."

