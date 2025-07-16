#!/bin/bash
# Copyright 2025 Google LLC
#
# Build a tag for the Docker image based on the current datetime, user, and
# repo state.

set -e

REL=$(dirname "$BASH_SOURCE")
DATETIME=`date -u "+%Y-%m-%dT%H_%M_%SZ"`
HASH=`git rev-parse HEAD`

# Determine repo state.
GITSTATE=`${REL}/gitstate.sh`
REPO_STATE=clean
if [ "$GITSTATE" = "dirty" ]; then
  REPO_STATE=dirty
fi
echo "${DATETIME}-${USER}-${HASH:0:7}-${REPO_STATE}"
