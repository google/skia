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
      'SK_SUPPORT_LEGACY_COLORFILTER_PTR',
      'SK_IGNORE_LINEONLY_AA_CONVEX_PATH_OPTS',
      'SK_SUPPORT_LEGACY_MINOR_EFFECT_PTR',
      'SK_SUPPORT_LEGACY_GRADIENT_DITHERING',
      'SK_SUPPORT_LEGACY_DRAWFILTER',
      'SK_SUPPORT_LEGACY_CREATESHADER_PTR',
      'SK_SUPPORT_LEGACY_PATHEFFECT_PTR',
      'SK_SUPPORT_LEGACY_NEW_SURFACE_API',
      'SK_SUPPORT_LEGACY_PICTURE_PTR',
      'SK_SUPPORT_LEGACY_MASKFILTER_PTR',
      'SK_SUPPORT_LEGACY_IMAGEFACTORY',
      'SK_SUPPORT_LEGACY_XFERMODE_PTR',
      'SK_SUPPORT_LEGACY_TYPEFACE_PTR',
    ],
  },
}
