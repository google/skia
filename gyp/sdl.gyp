# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# A simple gyp file to generate SDL for internal purposes
{
  'variables': {
    'base_dir%': '../third_party/libsdl',
    'src_dir%': '../third_party/externals/sdl',
    'skia_warnings_as_errors': 0,
  },
  'includes': [
    '../third_party/libsdl/sdl.gypi',
  ],
}
