# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# Building test for running CanvasState

# HOW TO USE:
# This target is not included in normal Skia builds. In order to build it,
# you need to run gyp_skia on this file. This target also requires the
# variable skia_pic to be used during building:
#
# GYP_DEFINES=skia_pic=1 ./gyp_skia gyp/canvas_state_lib.gyp
# ninja -C out/Debug canvas_state_lib
#
# This will create the shared library libcanvas_state_lib.so. That can
# be passed to tests to test passing an SkCanvas between versions of
# Skia. See tests/CanvasStateTest.cpp for more info.
{
  'targets' : [
    {
      'target_name' : 'canvas_state_lib',
      'type' : 'shared_library',
      # FIXME: Is there a way to ensure that -fPIC was used for skia_lib?
      'dependencies' : [ 'skia_lib.gyp:skia_lib'],
      'sources' : [
        '../tests/CanvasStateHelpers.cpp',
      ],
      'cflags' : [
        '-fPIC',
      ],
    },
    {
        # Dummy 'most' target, since gyp_skia sets 'most' to be the default.
        'target_name' : 'most',
        'type'        : 'none',
        'dependencies' : [
          'canvas_state_lib',
        ],
    }
  ],
}
