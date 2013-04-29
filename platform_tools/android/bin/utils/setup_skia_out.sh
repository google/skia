#!/bin/bash
#

UTIL_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ -z "$SKIA_OUT" ]
then

    if [ ! -f .android_config ]
    then
        echo "Unable to find the .android_config file"
        exit 1;
    fi

    export SKIA_OUT=$(cat .android_config)
    
    if [ ! -d ${SKIA_OUT} ]
    then
        echo "The contents of .android_config are invalid"
        exit 1;
    fi
fi