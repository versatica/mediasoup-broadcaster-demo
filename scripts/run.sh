#!/usr/bin/env bash

set -e

PROJECT_PWD=${PWD}

current_dir_name=${PROJECT_PWD##*/}
if [ "${current_dir_name}" != "mediasoup-broadcaster-demo" ] && [ "${current_dir_name}" != "v3-mediasoup-broadcaster-demo" ] ; then
	echo ">>> [ERROR] $(basename $0) must be called from root directory" >&2
	exit 1
fi

if [ "$1" == "build" ]; then
	echo ">>> building..."
	cmake --build build
elif [ "$1" == "rebuild" ]; then
	echo ">>> rebuilding..."
	rm -rf build/
	cmake . -Bbuild
	cmake --build build
fi

# Run.
./build/broadcaster $@
