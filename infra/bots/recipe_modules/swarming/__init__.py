# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

DEPS = [
  'build/file',
  'build/isolate',
  'build/swarming',
  'build/swarming_client',
  'depot_tools/depot_tools',
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/raw_io',
  'recipe_engine/step',
  'run',
]


# TODO(borenet): Add coverage
DISABLE_STRICT_COVERAGE = True
