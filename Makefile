all: build build32

configure:
	cmake --preset windows-only

build: configure
	cmake --build build
	python gen_lsp_tdm.py
	pwsh -c "cp build/SuRunExt/SuRunExt.dll ReleaseUx64"
	pwsh -c "cp build/SuRun.exe ReleaseUx64"

configure32:
	cmake --preset windows-only-32
build32: configure32
	cmake --build build32
	pwsh -c "cp build32/SuRunExt/SuRunExt.dll ReleaseUx64/SuRunExt32.dll"
	pwsh -c "cp build32/SuRun.exe ReleaseUx64/SuRun32.bin"
