#!/bin/bash


show_usage_and_exit() {
	echo
	echo tag.h \[ --reset \| --no-reset \] \[ --commit \| --no-commit \] -c \| --conf path/to/conf/file
	echo tag.h -? \| -h \| --help
	echo
	echo  \ \ \ \ --reset\ \ \ \ \ \ - resets the repo to the last commit, which reverts any uncommited changes. 
	echo  \ \ \ \ --no-reset\ \ \ - \(default\) proceeds with any uncommited changes.
	echo  \ \ \ \ --commit\ \ \ \ \ - commits the tagged files.
	echo  \ \ \ \ --no-commit\ \ - \(default\) leaves the tagged files uncommitted.
	echo  \ \ \ \ -? -h --help - shows this info
	echo
	echo \ \ \ \ Note: tag.* files are not subject to tagging.
	echo

	exit 1
}


init_globals() {
	TRUE=0
	FALSE=1
	UNDEFINED=2

	TAG_DIR=$( dirname $1 )
	TAG_AWK=$TAG_DIR/tag.awk
	TAG_SENTINEL=__TAG__
}


init_options() {
	OPT_RESET=$UNDEFINED
	OPT_COMMIT=$UNDEFINED
	OPT_CONF=

	while test $# -gt 0
	do
		case $1 in
			--reset)
				if test $OPT_RESET -eq $UNDEFINED
				then
					OPT_RESET=$TRUE
				else
					show_usage_and_exit
				fi
			;;
			--no-reset)
				if test $OPT_RESET -eq $UNDEFINED
				then
					OPT_RESET=$FALSE
				else
					show_usage_and_exit
				fi
			;;
			--commit)
				if test $OPT_COMMIT -eq $UNDEFINED
				then
					OPT_COMMIT=$TRUE
				else
					show_usage_and_exit
				fi
			;;
			--no-commit)
				if test $OPT_COMMIT -eq $UNDEFINED
				then
					OPT_COMMIT=$FALSE
				else
					show_usage_and_exit
				fi
			;;
			-c|--conf)
				if test -z $OPT_CONF
				then
					OPT_CONF=$2
					shift
				else
					show_usage_and_exit
				fi
			;;
			-?|-h|--help)
				show_usage_and_exit
			;;
			*)
				echo Unexpected parameter: $1
				exit 1
			;;
		esac
		shift
	done

	if test $OPT_RESET -eq $UNDEFINED
	then
		OPT_RESET=$FALSE
	fi

	if test $OPT_COMMIT -eq $UNDEFINED
	then
		OPT_COMMIT=$FALSE
	fi

	if test -z $OPT_CONF
	then
		echo Missing required parameter: --conf path/to/conf/file
		show_usage_and_exit
	fi
}


init_temp() {
	TEMP_DIR=/var/tmp/abc
	TEMP_FILE=$TEMP_DIR/tag.src
	mkdir --parents $TEMP_DIR
}


get_changed_files() {
	BEGIN_COMMIT=$( gawk '/^commit[ \t]+.*$/ { print $2 }' $OPT_CONF )
	END_COMMIT=$( git l --pretty=%h HEAD~1..HEAD | gawk '{ print $2 }' )

	# Get the files that were changed from the commit in the conf file through the last commit.
	FILES=$( git diff --name-status $BEGIN_COMMIT..$END_COMMIT | gawk '/^[AM][ \t]+.*$/ { print $2 }' )
}


tag_changed_files() {
	get_changed_files

	for FILE in $FILES
	do
		# Skip files named tag.*.
		echo $( basename $FILE ) | grep -E -q "^tag[\.$]"
		if test $? -eq 0
		then
			continue
		fi

		# Skip files that don't contain the tag sentinel.
		grep -E -q "$TAG_SENTINEL" $FILE
		if test $? -ne 0
		then
			continue
		fi

		# Create a temp copy of the subject file, so that we can overwrite it.
		cp -f -T $FILE $TEMP_FILE
		gawk -f $TAG_AWK -v TAG_CONF=$OPT_CONF $TEMP_FILE > $FILE
	done

	# Update the commit in the conf file.
	# Create a temp copy of the conf file, so that we can overwrite it.
	cp -f -T $OPT_CONF $TEMP_FILE
	gawk '! /^commit[ \t]+.*$/ { print $0 }' $TEMP_FILE > $OPT_CONF
	echo commit $END_COMMIT >> $OPT_CONF
}


main() {
	init_globals $0
	init_options $*
	init_temp

	if test $OPT_RESET -eq $TRUE
	then
		git reset --hard HEAD
	fi

	tag_changed_files

	if test $OPT_COMMIT -eq $TRUE
	then
		git add -- .
		git commit -m "Tagging"
	fi
}

main $*
exit 0
