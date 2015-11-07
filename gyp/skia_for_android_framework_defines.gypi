# Copyright 2014 Google Inc.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This file is included by chrome's skia/skia_common.gypi, and is intended to
# augment the skia flags that are set there.

{
  'variables': {

    # These flags will be defined in the android framework.
    #
    # If these become 'permanent', they should be moved into common_variables.gypi
    #
    'skia_for_android_framework_defines': [
      'SK_SUPPORT_LEGACY_UNBALANCED_PIXELREF_LOCKCOUNT',
      # Needed until we fix https://bug.skia.org/2440 .
      'SK_SUPPORT_LEGACY_CLIPTOLAYERFLAG',
      'SK_IGNORE_LINEONLY_AA_CONVEX_PATH_OPTS',
      'SK_SUPPORT_LEGACY_GRADIENT_DITHERING',
    ],
  },
}
