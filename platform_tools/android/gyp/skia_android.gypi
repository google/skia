# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This GYP file stores the dependencies necessary to build Skia on the Android
# platform. The OS doesn't provide many stable libraries as part of the
# distribution so we have to build a few of them ourselves.
#
{
  'variables': {
    'conditions': [
      [ 'skia_arch_type == "arm" and arm_version != 7', {
        'android_arch%': "armeabi",
        'android_variant%': "arm",
      }],
      [ 'skia_arch_type == "arm" and arm_version == 7', {
        'android_arch%': "armeabi-v7a",
        'android_variant%': "arm",
      }],
      [ 'skia_arch_type == "arm64"', {
        'android_arch%': "arm64-v8a",
        'android_variant%': "arm64",
      }],
      [ 'skia_arch_type == "x86"', {
        'android_arch%': "x86",
        'android_variant%': "x86",
      }],
      [ 'skia_arch_type == "x86_64"', {
        'android_arch%': "x86_64",
        'android_variant%': "x86_64",
      }],
      [ 'skia_arch_type == "mips32"', {
        'android_arch%': "mips",
        'android_variant%': "mips",
      }],
      [ 'skia_arch_type == "mips64"', {
        'android_arch%': "mips64",
        'android_variant%': "mips64",
      }],
      [ 'android_buildtype == "Debug"', {
        'android_apk_suffix': "debug.apk",
      }, {
        # This also accounts for Release_Developer BUILDTYPE
        'android_buildtype': "Release", 
        'android_apk_suffix': "release.apk",
      }],
    ],
  },
  'includes' : [
      'canvasproof.gypi',
      'viewer.gypi',
  ],
}
