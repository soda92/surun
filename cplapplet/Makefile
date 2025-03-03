all: build

.PHONY: build

build:
	cmake --preset windows
	cmake --build build
	python ../gen_lsp_tdm.py --builddir=cplapplet/build --destdir=cplapplet/build

clean:
	pwsh -nop -c "rm -r -Force build"
