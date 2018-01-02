#!/bin/bash

set -x -e

rm -rf platform_tools/android/apps/skqpapp/src/main/assets/gmkb
# go get -u go.skia.org/infra/golden/go/search
go run tools/skqp/make_gmkb.go ~/Downloads/meta.json \
       platform_tools/android/apps/skqpapp/src/main/assets/gmkb
