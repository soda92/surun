all: surun build32 bi

configure-surun:
	cmake --preset windows-only

surun: configure-surun
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

configure32:
	cmake --preset windows-only-32
build32: configure32
	cmake --build build32
	pwsh -nop -c "cp build32/SuRunExt/SuRunExt32.dll ReleaseUx64/SuRunExt32.dll"
	pwsh -nop -c "cp build32/SuRun32.dll ."
	GOARCH=386 go build -o SuRun32.bin .
	pwsh -nop -c "mv SuRun32.bin ReleaseUx64/"

bi: #build installer
	cd InstallSuRUn && cmake --preset windows-only
	cmake --build build-i
	python merge_cc.py

clean:
	pwsh -nop -c "rm -r build"
	pwsh -nop -c "rm -r build32"
	pwsh -nop -c "rm -r build-i"
	pwsh -nop -c "rm -r build-debug"

debug:
	cmake --preset windows-debug
	cmake --build build-debug
	python gen_lsp_tdm.py -B build-debug
	python merge_cc.py -B build-debug
	pwsh -nop -c "cp build-debug/SuRun.dll ."
	go build -gcflags="-N -l" -o SuRunD.exe .
