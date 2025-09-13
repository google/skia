#!/bin/bash
# Copyright 2025 Google LLC
#
# Build a tag for the Docker image based on the current git revision.
# Stage Manager cares about the format of this when determining which
# version to deploy or not.

set -e

HASH=`git rev-parse HEAD`

echo "git-${HASH}"
