# Copyright 2025 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from . import api as _api

DEPS = [
  'recipe_engine/cipd',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/step',
  'vars',
]

API = _api.SkiaXCodeApi
