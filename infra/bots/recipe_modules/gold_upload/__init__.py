# Copyright 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

PYTHON_VERSION_COMPATIBILITY = "PY2+3"

DEPS = [
  'recipe_engine/json',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/python',
  'recipe_engine/properties',
  'recipe_engine/step',
  'recipe_engine/time',
  'flavor',
  'gsutil',
  'run',
  'vars',
]
