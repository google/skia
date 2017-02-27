# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

DEPS = [
  'build/file',
  'depot_tools/gsutil',
  'recipe_engine/path',
  'recipe_engine/step',
  'run',
]


# TODO(borenet): Add coverage
DISABLE_STRICT_COVERAGE = True
