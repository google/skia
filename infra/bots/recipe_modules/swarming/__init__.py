# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# TODO(borenet): This module was copied from build.git and heavily modified to
# remove dependencies on other modules in build.git.  It belongs in a different
# repo. Remove this once it has been moved.


DEPS = [
  'isolate',
  'recipe_engine/context',
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/raw_io',
  'recipe_engine/step',
  'swarming_client',
]

from recipe_engine.recipe_api import Property

PROPERTIES = {
  'show_shards_in_collect_step': Property(default=False, kind=bool),
  'show_isolated_out_in_collect_step': Property(default=True, kind=bool),
}


# TODO(phajdan.jr): provide coverage (http://crbug.com/693058).
DISABLE_STRICT_COVERAGE = True
