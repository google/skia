#!/bin/bash
# Copyright 2025 Google LLC
#
# Build a tag for the Docker image based on the current datetime, user, and
# git revision.

set -e

echo "git-${HASH}"
