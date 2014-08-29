# Copyright 2014 Google Inc.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This file is included by chrome's skia/skia_common.gypi, and is intended to
# augment the skia flags that are set there.

{
  'variables': {

    # These flags will be defined in chromium
    #
    # If these become 'permanent', they should be moved into skia_common.gypi
    #
    'skia_for_chromium_defines': [
      'SK_IGNORE_PROPER_FRACTIONAL_SCALING',
      'SK_SUPPORT_LEGACY_PICTURE_CLONE',
      'SK_SUPPORT_LEGACY_GETDEVICE',
      'SK_IGNORE_ETC1_SUPPORT',
      'SK_IGNORE_GPU_DITHER',
      'SK_SUPPORT_LEGACY_IMAGECACHE_NAME',
      'SK_LEGACY_PICTURE_SIZE_API',
    ],
  },
}
