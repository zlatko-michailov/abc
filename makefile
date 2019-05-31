
VERSION = 0.1
DEBUG = -ggdb
CPPOPTIONS = $(DEBUG) --std=c++17 -Wpedantic
LINKOPTIONS = -l:libstdc++.so.6 -l:libgcc_s.so.1 -l:libpthread.so

all: pack

pack: test
	#
	# ---------- Begin packing ----------
	cp $(CURDIR)/src/*.h     $(CURDIR)/out/abc/inc
	# ---------- Done packing ----------
	#

test: build_test
	#
	# ---------- Begin testing ----------
	$(CURDIR)/out/test/abc_test
	# ---------- Done testing ----------
	#

build_test: build_product
	#
	# ---------- Begin building tests ----------
	g++ $(CPPOPTIONS) -o $(CURDIR)/out/test/abc_test $(CURDIR)/test/*.cpp $(LINKOPTIONS)
	# ----------Done building tests ----------
	#

build_product: tag
	#
	# ---------- Begin building product ----------
	# This section should remain blank.
	# ---------- Done building product ----------
	#

tag: clean
	#
	# ---------- Begin tagging ----------
	# TODO: tag
	# ---------- Done tagging ----------
	#

clean:
	#
	# ---------- Begin cleaning ----------
	rm -fdr $(CURDIR)/out
	mkdir $(CURDIR)/out
	mkdir $(CURDIR)/out/test
	mkdir $(CURDIR)/out/abc
	mkdir $(CURDIR)/out/abc/inc
	# ---------- Done cleaning ----------
	#

