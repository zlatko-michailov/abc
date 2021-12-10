#!/usr/bin/make

# MIT License
# 
# Copyright (c) 2018-2021 Zlatko Michailov 
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
VERSION = 1.14.0
DEBUG = -ggdb
CPPOPTIONS = $(DEBUG) --std=c++11 -Wpedantic -D_FILE_OFFSET_BITS=64
LINKOPTIONS = -lstdc++ -lpthread
SUBDIR_SRC = src
SUBDIR_TEST = test
SUBDIR_OUT = out
SUBDIR_INCLUDE = include
SUBDIR_BIN = bin
SUBDIR_SAMPLES = samples
SUBDIR_RESOURCES = resources
SAMPLE_BASIC = basic
SAMPLE_VMEM = vmem
SAMPLE_TICTACTOE = tictactoe
SAMPLE_CONNECT4 = connect4
PROG_TEST = $(PROJECT)_test


all: test
	#
	# ---------- Done all ----------
	#
	#

test: pack
	#
	# ---------- Begin testing ----------
	$(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_TEST)/$(PROG_TEST)
	# ---------- Done testing ----------
	#

pack: build_product build_test build_samples
	#
	# ---------- Begin packing ----------
	cp $(CURDIR)/LICENSE  $(CURDIR)/$(SUBDIR_OUT)/$(PROJECT)/$(VERSION)
	cp $(CURDIR)/README.md  $(CURDIR)/$(SUBDIR_OUT)/$(PROJECT)/$(VERSION)
	cp -r $(CURDIR)/$(SUBDIR_SRC)/*  $(CURDIR)/$(SUBDIR_OUT)/$(PROJECT)/$(VERSION)/$(SUBDIR_INCLUDE)
	cp -r $(CURDIR)/$(SUBDIR_BIN)/*  $(CURDIR)/$(SUBDIR_OUT)/$(PROJECT)/$(VERSION)/$(SUBDIR_BIN)
	cp -r $(CURDIR)/$(SUBDIR_SAMPLES)/*  $(CURDIR)/$(SUBDIR_OUT)/$(PROJECT)/$(VERSION)/$(SUBDIR_SAMPLES)
	ln --symbolic $(VERSION)/$(SUBDIR_INCLUDE) $(CURDIR)/$(SUBDIR_OUT)/$(PROJECT)/$(SUBDIR_INCLUDE)
	ln --symbolic $(VERSION)/$(SUBDIR_BIN) $(CURDIR)/$(SUBDIR_OUT)/$(PROJECT)/$(SUBDIR_BIN)
	cd $(CURDIR)/$(SUBDIR_OUT); zip -ry9 $(PROJECT)_$(VERSION).zip $(PROJECT); cd $(CURDIR)
	# ---------- Done packing ----------
	#

build_samples: build_product
	#
	# ---------- Begin building samples ----------
	#
	# ---------- Begin building basic ----------
	mkdir $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_BASIC)
	g++ $(CPPOPTIONS) -o $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_BASIC)/$(SAMPLE_BASIC) $(CURDIR)/$(SUBDIR_SAMPLES)/$(SAMPLE_BASIC)/*.cpp $(LINKOPTIONS)
	mkdir $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_BASIC)/$(SUBDIR_RESOURCES)
	cp $(CURDIR)/$(SUBDIR_SAMPLES)/$(SAMPLE_BASIC)/$(SUBDIR_RESOURCES)/* $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_BASIC)/$(SUBDIR_RESOURCES)
	# ---------- Done building basic ----------
	#
	# ---------- Begin building vmem ----------
	mkdir $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_VMEM)
	g++ $(CPPOPTIONS) -o $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_VMEM)/$(SAMPLE_VMEM) $(CURDIR)/$(SUBDIR_SAMPLES)/$(SAMPLE_VMEM)/*.cpp $(LINKOPTIONS)
	# ---------- Done building vmem ----------
	#
	# ---------- Begin building tictactoe ----------
	mkdir $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_TICTACTOE)
	g++ $(CPPOPTIONS) -o $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_TICTACTOE)/$(SAMPLE_TICTACTOE) $(CURDIR)/$(SUBDIR_SAMPLES)/$(SAMPLE_TICTACTOE)/*.cpp $(LINKOPTIONS)
	mkdir $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_TICTACTOE)/$(SUBDIR_RESOURCES)
	cp $(CURDIR)/$(SUBDIR_SAMPLES)/$(SAMPLE_TICTACTOE)/$(SUBDIR_RESOURCES)/* $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_TICTACTOE)/$(SUBDIR_RESOURCES)
	# ---------- Done building tictactoe ----------
	#
	# ---------- Begin building connect4 ----------
	mkdir $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_CONNECT4)
	g++ $(CPPOPTIONS) -o $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_CONNECT4)/$(SAMPLE_CONNECT4) $(CURDIR)/$(SUBDIR_SAMPLES)/$(SAMPLE_CONNECT4)/*.cpp $(LINKOPTIONS)
	mkdir $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_CONNECT4)/$(SUBDIR_RESOURCES)
	cp $(CURDIR)/$(SUBDIR_SAMPLES)/$(SAMPLE_CONNECT4)/$(SUBDIR_RESOURCES)/* $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_CONNECT4)/$(SUBDIR_RESOURCES)
	# ---------- Done building tictactoe ----------
	#
	# ---------- Done building connect4 ----------
	#

build_test: build_product
	#
	# ---------- Begin building tests ----------
	g++ $(CPPOPTIONS) -o $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_TEST)/$(PROG_TEST) $(CURDIR)/$(SUBDIR_TEST)/*.cpp $(LINKOPTIONS)
	# ---------- Done building tests ----------
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
	mkdir $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)
	mkdir $(CURDIR)/$(SUBDIR_OUT)/$(PROJECT)
	mkdir $(CURDIR)/$(SUBDIR_OUT)/$(PROJECT)/$(VERSION)
	mkdir $(CURDIR)/$(SUBDIR_OUT)/$(PROJECT)/$(VERSION)/$(SUBDIR_INCLUDE)
	mkdir $(CURDIR)/$(SUBDIR_OUT)/$(PROJECT)/$(VERSION)/$(SUBDIR_BIN)
	mkdir $(CURDIR)/$(SUBDIR_OUT)/$(PROJECT)/$(VERSION)/$(SUBDIR_SAMPLES)
	# ---------- Done cleaning ----------
	#

