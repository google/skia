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
      'SK_SUPPORT_LEGACY_SETCONFIG_INFO',
      'SK_SUPPORT_LEGACY_SETCONFIG',
      'SK_SUPPORT_LEGACY_IMAGEDECODER_CONFIG',
      'SK_SUPPORT_LEGACY_DEVICE_VIRTUAL_ISOPAQUE',
      'SK_SUPPORT_LEGACY_BITMAP_CONFIG',
      # Needed until we fix skbug.com/2440.
      'SK_SUPPORT_LEGACY_CLIPTOLAYERFLAG',
      # Transitional, for deprecated SkCanvas::SaveFlags methods.
      'SK_ATTR_DEPRECATED=SK_NOTHING_ARG1',
      'SK_SUPPORT_LEGACY_SHADER_LOCALMATRIX',
      'SK_SUPPORT_LEGACY_COMPUTE_CONFIG_SIZE',
    ],
  },
}
