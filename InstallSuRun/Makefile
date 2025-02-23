all: build

build:
	cmake --preset windows-only
	cmake --build ../build-i
	cmake --build ../build-i
	python ../merge_cc.py -B build -I build-i

debug:
	cmake --preset windows-debug
	cmake --build ../build-ig
	cmake --build ../build-ig
	python ../merge_cc.py -B build-debug -I build-ig
