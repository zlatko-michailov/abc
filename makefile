#!/usr/bin/make

# MIT License
#
# Copyright (c) 2018-2022 Zlatko Michailov
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
VERSION = 1.16.0

CPP = g++
CPP_OPT_DEBUG = -ggdb
CPP_OPT_STD = --std=c++11
CPP_OPT_WARN = -Wall -Wextra -Wpedantic -Wno-array-bounds
CPP_OPT_FILE_OFFSET_BITS = -D_FILE_OFFSET_BITS=64
CPP_OPTIONS = $(CPP_OPT_DEBUG) $(CPP_OPT_STD) $(CPP_OPT_WARN) $(CPP_OPT_FILE_OFFSET_BITS)
CPP_LINK_OPTIONS = -lstdc++ -lpthread

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
SAMPLE_PICAR_4WD = picar_4wd

PROG_TEST = $(PROJECT)_test

UNAME = $(shell uname)
DEPS_BUILD_SAMPLES_COMMON = build_sample_$(SAMPLE_BASIC) build_sample_$(SAMPLE_VMEM) build_sample_$(SAMPLE_TICTACTOE) build_sample_$(SAMPLE_CONNECT4)
ifeq "$(UNAME)" "Linux"
	DEPS_BUILD_SAMPLES = $(DEPS_BUILD_SAMPLES_COMMON)  build_sample_$(SAMPLE_PICAR_4WD)
else
	DEPS_BUILD_SAMPLES = $(DEPS_BUILD_SAMPLES_COMMON)
endif


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
	ln -s $(VERSION)/$(SUBDIR_INCLUDE) $(CURDIR)/$(SUBDIR_OUT)/$(PROJECT)/$(SUBDIR_INCLUDE)
	ln -s $(VERSION)/$(SUBDIR_BIN) $(CURDIR)/$(SUBDIR_OUT)/$(PROJECT)/$(SUBDIR_BIN)
	cd $(CURDIR)/$(SUBDIR_OUT); zip -ry9 $(PROJECT)_$(VERSION).zip $(PROJECT); cd $(CURDIR)
	# ---------- Done packing ----------
	#

build_samples: build_product build_samples_begin $(DEPS_BUILD_SAMPLES)
	#
	# ---------- Done building samples ----------
	#

build_samples_begin:
	#
	# ---------- Begin building samples ----------

build_sample_$(SAMPLE_BASIC): build_product
	#
	# ---------- Begin building basic ----------
	mkdir $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_BASIC)
	$(CPP) $(CPP_OPTIONS) -o $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_BASIC)/$(SAMPLE_BASIC) $(CURDIR)/$(SUBDIR_SAMPLES)/$(SAMPLE_BASIC)/*.cpp $(CPP_LINK_OPTIONS)
	mkdir $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_BASIC)/$(SUBDIR_RESOURCES)
	cp $(CURDIR)/$(SUBDIR_SAMPLES)/$(SAMPLE_BASIC)/$(SUBDIR_RESOURCES)/* $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_BASIC)/$(SUBDIR_RESOURCES)
	# ---------- Done building basic ----------

build_sample_$(SAMPLE_VMEM): build_product
	#
	# ---------- Begin building vmem ----------
	mkdir $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_VMEM)
	$(CPP) $(CPP_OPTIONS) -o $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_VMEM)/$(SAMPLE_VMEM) $(CURDIR)/$(SUBDIR_SAMPLES)/$(SAMPLE_VMEM)/*.cpp $(CPP_LINK_OPTIONS)
	# ---------- Done building vmem ----------

build_sample_$(SAMPLE_TICTACTOE): build_product
	#
	# ---------- Begin building tictactoe ----------
	mkdir $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_TICTACTOE)
	$(CPP) $(CPP_OPTIONS) -o $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_TICTACTOE)/$(SAMPLE_TICTACTOE) $(CURDIR)/$(SUBDIR_SAMPLES)/$(SAMPLE_TICTACTOE)/*.cpp $(CPP_LINK_OPTIONS)
	mkdir $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_TICTACTOE)/$(SUBDIR_RESOURCES)
	cp $(CURDIR)/$(SUBDIR_SAMPLES)/$(SAMPLE_TICTACTOE)/$(SUBDIR_RESOURCES)/* $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_TICTACTOE)/$(SUBDIR_RESOURCES)
	# ---------- Done building tictactoe ----------

build_sample_$(SAMPLE_CONNECT4): build_product
	#
	# ---------- Begin building connect4 ----------
	mkdir $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_CONNECT4)
	$(CPP) $(CPP_OPTIONS) -o $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_CONNECT4)/$(SAMPLE_CONNECT4) $(CURDIR)/$(SUBDIR_SAMPLES)/$(SAMPLE_CONNECT4)/*.cpp $(CPP_LINK_OPTIONS)
	mkdir $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_CONNECT4)/$(SUBDIR_RESOURCES)
	cp $(CURDIR)/$(SUBDIR_SAMPLES)/$(SAMPLE_CONNECT4)/$(SUBDIR_RESOURCES)/* $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_CONNECT4)/$(SUBDIR_RESOURCES)
	# ---------- Done building connect4 ----------

build_sample_$(SAMPLE_PICAR_4WD): build_product
	#
	# ---------- Begin building picar_4wd ----------
	mkdir $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_PICAR_4WD)
	$(CPP) $(CPP_OPTIONS) -o $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_PICAR_4WD)/$(SAMPLE_PICAR_4WD) $(CURDIR)/$(SUBDIR_SAMPLES)/$(SAMPLE_PICAR_4WD)/*.cpp $(CPP_LINK_OPTIONS)
	mkdir $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_PICAR_4WD)/$(SUBDIR_RESOURCES)
	cp $(CURDIR)/$(SUBDIR_SAMPLES)/$(SAMPLE_PICAR_4WD)/$(SUBDIR_RESOURCES)/* $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_SAMPLES)/$(SAMPLE_PICAR_4WD)/$(SUBDIR_RESOURCES)
	# ---------- Done building picar_4wd ----------

build_test: build_product
	#
	# ---------- Begin building tests ----------
	$(CPP) $(CPP_OPTIONS) -o $(CURDIR)/$(SUBDIR_OUT)/$(SUBDIR_TEST)/$(PROG_TEST) $(CURDIR)/$(SUBDIR_TEST)/*.cpp $(CPP_LINK_OPTIONS)
	# ---------- Done building tests ----------
	#

build_product: clean
	#
	# ---------- Begin building product ----------
	# This section should remain blank.
	# ---------- Done building product ----------
	#

clean: vars
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

vars:
	#
	# ---------- Begin vars ----------
	# CPP = $(CPP)
	# CPP_OPT_DEBUG = $(CPP_OPT_DEBUG)
	# CPP_OPT_STD = $(CPP_OPT_STD)
	# CPP_OPT_WARN = $(CPP_OPT_WARN)
	# CPP_OPT_FILE_OFFSET_BITS = $(CPP_OPT_FILE_OFFSET_BITS)
	# CPP_OPTIONS = $(CPP_OPTIONS)
	# CPP_LINK_OPTIONS = $(CPP_LINK_OPTIONS)
	# ---------- Done vars ----------
	#

