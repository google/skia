#!/bin/bash
# Copyright 2025 Google LLC
#
# Build a tag for the Docker image based on the current datetime, user, and
# git revision.

set -e

DATETIME=`date -u "+%Y-%m-%dT%H_%M_%SZ"`
HASH=`git rev-parse HEAD`

echo "${DATETIME}-${USER}-${HASH:0:7}"
