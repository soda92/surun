all: surun build32 bi
msys: msys-surun msys-32 msys-installer

surun:
	cmake --preset windows-only
	cmake --build build
	python gen_lsp_tdm.py
	pwsh -nop -c "cp build/SuRunExt/SuRunExt.dll ReleaseUx64/"
	pwsh -nop -c "cp build/SuRun.dll ."
	go build .

msys-surun:
	pwsh -nop msys.ps1 -ucrt64 -c "make _msys_surun"

_msys_surun:
	cmake --preset msys2
	cmake --build build
	cp build/SuRunExt/SuRunExt.dll ReleaseUx64/
	cp build/SuRun.dll .
	go build .

build32:
	cmake --preset windows-only-32
	cmake --build build32
	pwsh -nop -c "cp build32/SuRunExt/SuRunExt32.dll ReleaseUx64/SuRunExt32.dll"
	pwsh -nop -c "cp build32/SuRun32.dll ."
	GOARCH=386 go build -o SuRun32.bin .
	pwsh -nop -c "mv SuRun32.bin ReleaseUx64/"

msys-32:
	pwsh -nop msys.ps1 -mingw32 -c "make _msys_32"

_msys_32:
	cmake --preset msys2_32
	cmake --build build-msys-32
	cp build32/SuRunExt/SuRunExt32.dll ReleaseUx64/SuRunExt32.dll
	cp build32/SuRun32.dll .
	GOARCH=386 go build -o SuRun32.bin .
	mv SuRun32.bin ReleaseUx64/

bi: #build installer
	cd InstallSuRun && cmake --preset windows-only
	cmake --build build-i
	python merge_cc.py

msys-installer:
	pwsh -nop msys.ps1 -ucrt64 -c "make _msys_installer"

_msys_installer:
	cd InstallSuRun && cmake --preset msys2
	cmake --build build-msys-installer

clean:
	pwsh -nop -c "rm -r build"
	pwsh -nop -c "rm -r build32"
	pwsh -nop -c "rm -r build-i"
	pwsh -nop -c "rm -r build-debug"

clean-msys:
	pwsh -nop msys.ps1 -ucrt64 -c "make _clean_msys"

_clean_msys:
	rm -r build-msys* || true

debug:
	cmake --preset windows-debug
	cmake --build build-debug
	python gen_lsp_tdm.py -B build-debug
	python merge_cc.py -B build-debug
	pwsh -nop -c "cp build-debug/SuRun.dll ."
	go build -gcflags="-N -l" -o SuRunD.exe .
