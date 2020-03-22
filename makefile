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


PROJECT = abc
VERSION = 0.3.0
DEBUG = -ggdb
CPPOPTIONS = $(DEBUG) --std=c++17 -Wpedantic
LINKOPTIONS = -l:libstdc++.so.6 -l:libgcc_s.so.1 -l:libpthread.so
SUBDIR_SRC = src
SUBDIR_TEST = test
SUBDIR_OUT = out
SUBDIR_INCLUDE = include
PROG_TEST = $(PROJECT)_test


all: pack

pack: test
	#
	# ---------- Begin packing ----------
	cp -r $(CURDIR)/$(SUBDIR_SRC)/*  $(CURDIR)/$(SUBDIR_OUT)/$(PROJECT)/$(VERSION)/$(SUBDIR_INCLUDE)
	ln --symbolic $(CURDIR)/$(SUBDIR_OUT)/$(PROJECT)/$(VERSION)/$(SUBDIR_INCLUDE) $(CURDIR)/$(SUBDIR_OUT)/$(PROJECT)/$(SUBDIR_INCLUDE)
	# ---------- Done packing ----------
	#

test: build_test
	#
	# ---------- Begin testing ----------
	$(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_TEST)/$(PROG_TEST)
	# ---------- Done testing ----------
	#

build_test: build_product
	#
	# ---------- Begin building tests ----------
	g++ $(CPPOPTIONS) -o $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_TEST)/$(PROG_TEST) $(CURDIR)/$(SUBDIR_TEST)/*.cpp $(LINKOPTIONS)
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
	rm -fdr $(CURDIR)/$(SUBDIR_OUT)
	mkdir $(CURDIR)/$(SUBDIR_OUT)
	mkdir $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_TEST)
	mkdir $(CURDIR)/$(SUBDIR_OUT)/$(PROJECT)
	mkdir $(CURDIR)/$(SUBDIR_OUT)/$(PROJECT)/$(VERSION)
	mkdir $(CURDIR)/$(SUBDIR_OUT)/$(PROJECT)/$(VERSION)/$(SUBDIR_INCLUDE)
	# ---------- Done cleaning ----------
	#

