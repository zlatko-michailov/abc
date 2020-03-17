function init() {
	MAX_TAG = 2147483647;
	TAG = "__TAG__";
##	TAG_CONF = "tag.conf";
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

