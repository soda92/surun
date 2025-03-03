all: build

.PHONY: build

build:
	cmake --preset windows
	cmake --build build
	python ../gen_lsp_tdm.py --builddir=cplapplet/build --destdir=cplapplet/build
	pwsh -nop -c "cp build/myapplet.dll build/myapplet.cpl"

install:
	sufilecopy copy build/myapplet.dll C:/windows/system32/

clean:
	pwsh -nop -c "rm -r -Force build"
