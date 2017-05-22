# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# TODO(borenet): This module was copied from build.git and heavily modified to
# remove dependencies on other modules in build.git.  It belongs in a different
# repo. Remove this once it has been moved.


from recipe_engine import recipe_test_api

class IsolateTestApi(recipe_test_api.RecipeTestApi):
  def output_json(self, targets=None, missing=None):
    """Mocked output of 'find_isolated_tests' and 'isolate_tests' steps.

    Deterministically synthesizes json.output test data for the given targets.
    If |targets| is None, will emit test data with some dummy targets instead,
    emulating find_isolated_tests.py finding some files.

    If |missing| is given it's a subset of |targets| that wasn't isolated in
    'isolate_tests' due to some error.
    """
    missing = missing or ()
    if targets is None:
      targets = ['dummy_target_1', 'dummy_target_2']
    return self.m.json.output({
      target: None if target in missing else '[dummy hash for %s]' % target
      for target in targets
    })
