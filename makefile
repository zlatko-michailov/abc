#!/usr/bin/make

# MIT License
# 
# Copyright (c) 2018-2020 Zlatko Michailov 
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.


VERSION = 0.1
DEBUG = -ggdb
CPPOPTIONS = $(DEBUG) --std=c++17 -Wpedantic
LINKOPTIONS = -l:libstdc++.so.6 -l:libgcc_s.so.1 -l:libpthread.so

all: pack

pack: test
	#
	# ---------- Begin packing ----------
	cp -r $(CURDIR)/src/*  $(CURDIR)/out/abc/inc
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

build_product: clean
	#
	# ---------- Begin building product ----------
	# This section should remain blank.
	# ---------- Done building product ----------
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

