
VERSION = 0.1
PRODUCTLIB = abc-$(VERSION)
DEBUG = -ggdb
CPPOPTIONS = $(DEBUG) --std=c++17 -Wpedantic
LINKOPTIONS = -l:libstdc++.so.6 -l:libgcc_s.so.1 -l:libpthread.so

all: pack

pack: test
	#
	# ---------- Begin packing ----------
	cp $(CURDIR)/src/*.h     $(CURDIR)/out/abc/inc
	cp $(CURDIR)/out/obj/*.a $(CURDIR)/out/abc/lib
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
	g++ $(CPPOPTIONS) -o $(CURDIR)/out/test/abc_test $(CURDIR)/test/*.cpp $(LINKOPTIONS) -L$(CURDIR)/out/obj -l$(PRODUCTLIB)
	# ----------Done building tests ----------
	#

build_product: tag
	#
	# ---------- Begin building product ----------
	g++ -c $(CPPOPTIONS) $(CURDIR)/src/*.cpp
	mv $(CURDIR)/*.o $(CURDIR)/out/obj
	ar rcs $(CURDIR)/out/obj/lib$(PRODUCTLIB).a $(CURDIR)/out/obj/*.o
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
	mkdir $(CURDIR)/out/obj
	mkdir $(CURDIR)/out/test
	mkdir $(CURDIR)/out/abc
	mkdir $(CURDIR)/out/abc/inc
	mkdir $(CURDIR)/out/abc/lib
	# ---------- Done cleaning ----------
	#

