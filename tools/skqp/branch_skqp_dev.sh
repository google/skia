#! /bin/sh
# Copyright 2018 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
set -xe
set -- platform_tools/android/apps/skqp/src/main/assets/files.checksum       \
       platform_tools/android/apps/skqp/src/main/assets/skqp/rendertests.txt \
       platform_tools/android/apps/skqp/src/main/assets/skqp/unittests.txt
for arg; do [ -f "$arg" ] || exit 1; done
MSG="$(printf 'Cut SkQP %s\n\nNo-Try: true' "$(date +%Y-%m-%d)")"
git merge -s ours origin/skqp/dev -m "$MSG"
git add "$@"
git commit --amend --reuse-message=HEAD
