# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from . import api as _api

DEPS = [
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/raw_io',
  'recipe_engine/step',
  'run',
  'vars',
]

API = _api.InfraApi
