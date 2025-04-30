all: build
.PHONY: build clean

build:
	cmake --preset windows
	cmake --build build
	pwsh -nop -c "cp ../godylib/lib.dll build/"
	python ../gen_lsp_tdm.py --builddir=c_link_go/build --destdir=c_link_go/build

clean:
	pwsh -nop -c "rm -r -Force build"

run:
	.\build\test.exe