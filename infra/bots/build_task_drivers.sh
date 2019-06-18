#!/bin/bash
# Copyright 2019 Google, LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -x -e

export GOCACHE="$(pwd)/cache/go_cache"
export GOPATH="$(pwd)/cache/gopath"
export GOROOT="$(pwd)/go/go"

cd skia

# Build task drivers from the infra repo.
export GOBIN="${1}"
go install -v go.skia.org/infra/infra/bots/task_drivers/update_go_deps
