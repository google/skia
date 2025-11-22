# Copyright 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from . import api as _api

DEPS = [
  'recipe_engine/json',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'recipe_engine/step',
  'recipe_engine/time',
  'flavor',
  'gsutil',
  'run',
  'vars',
]

API = _api.GoldUploadApi
