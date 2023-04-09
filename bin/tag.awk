#!/bin/gawk

# MIT License
# 
# Copyright (c) 2018-2023 Zlatko Michailov 
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


function init() {
	MAX_TAG = 2147483647;
	TAG = "__TAG__";
}


function load_tag() {
	getline < TAG_CONF;
	tag_hi = $2;

	getline < TAG_CONF;
	tag_lo = $2;
}


function save_tag() {
	printf "tag_hi %d\n", tag_hi > TAG_CONF;
	printf "tag_lo %d\n", tag_lo >> TAG_CONF;
}


function inc_tag() {
	if (tag_lo < MAX_TAG) {
		tag_lo++;
	}
	else {
		tag_lo = 0;
		if (tag_hi < MAX_TAG) {
			tag_hi++;
		}
		else {
			tag_hi = 0;
		}
	}
}


function printf_tag() {
	printf "0x";
	if (tag_hi > 0) {
		printf "%x%8.8x", tag_hi, tag_lo;
	}
	else {
		printf "%x", tag_lo;
	}

}


function start() {
	FS = TAG;
}


BEGIN {
	init();
	load_tag();
	start();
}


END {
	save_tag();
}


{
	printf "%s", $1;

	for (i = 2; i <= NF; i++) {
		inc_tag();
		printf_tag();
		printf "%s", $i;
	}

	printf "\n";
}

