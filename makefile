
all: pack

pack: test

test: build
	out/abc_test

build: tag
	g++ -ggdb --std=c++17 -Wpedantic -Wl,-l:libstdc++.so.6 -Wl,-l:libgcc_s.so.1 -Wl,-l:libpthread.so -o $(CURDIR)/out/abc_test $(CURDIR)/test/main.cpp

tag:

