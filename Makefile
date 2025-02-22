all: build

configure:
	cmake --preset windows-only

build: configure
	cmake --build build
	python gen_lsp_tdm.py

