#!/bin/bash
# Copyright 2020 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
################################################################################

set -ex

git clone https://github.com/google/oss-fuzz.git --depth 1
docker pull gcr.io/oss-fuzz-base/cifuzz-base:latest

docker build -t gcr.io/oss-fuzz-base/build_fuzzers oss-fuzz/infra/cifuzz/actions/build_fuzzers
docker build -t gcr.io/oss-fuzz-base/run_fuzzers oss-fuzz/infra/cifuzz/actions/run_fuzzers

# TODO(metzman): Make these not required.
export GITHUB_EVENT_NAME="push"
export GITHUB_REPOSITORY=$REPO_NAME

export DRY_RUN=0
export CI=true
export SANITIZER='address'
export GITHUB_SHA=$COMMIT_SHA

export WORKDIR='/tmp/cifuzz'
mkdir $WORKDIR
export GITHUB_WORKSPACE=$WORKDIR
echo "OSS_FUZZ_PROJECT_NAME $OSS_FUZZ_PROJECT_NAME"

docker run --name build_fuzzers --rm -ti -e MANUAL_SRC_PATH -e OSS_FUZZ_PROJECT_NAME -e GITHUB_WORKSPACE \
    -e GITHUB_REPOSITORY -e GITHUB_EVENT_NAME -e DRY_RUN -e CI -e SANITIZER -e GITHUB_SHA \
    -v $MANUAL_SRC_PATH:$MANUAL_SRC_PATH -v /var/run/docker.sock:/var/run/docker.sock -v $WORKDIR:$WORKDIR \
    gcr.io/oss-fuzz-base/build_fuzzers

docker run --name run_fuzzers --rm -ti -e OSS_FUZZ_PROJECT_NAME -e GITHUB_WORKSPACE -e GITHUB_REPOSITORY \
    -e GITHUB_EVENT_NAME -e DRY_RUN -e CI -e SANITIZER -e GITHUB_SHA -v /var/run/docker.sock:/var/run/docker.sock \
    -v $WORKDIR:$WORKDIR gcr.io/oss-fuzz-base/run_fuzzers