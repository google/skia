# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This GYP file stores the dependencies necessary to build Skia on the Android
# platform. The OS doesn't provide many stable libraries as part of the
# distribution so we have to build a few of them ourselves.
#
# NOTE: We tried adding the gyp file to the android/ directory at the root of
# the Skia repo, but that resulted in the generated makefiles being created
# outside of the out directory. We may be able to move the bulk of this gyp
# to the /android directory and put a simple shim here, but that has yet to be
# tested.

{
  'variables': {
    'skia_warnings_as_errors': 0,
  },
  'targets': [
    {
      'target_name': 'native_app_glue',
      'type': 'static_library',
      'direct_dependent_settings': {
        'include_dirs': [
          '../third_party/native_app_glue',
        ],
      },
      'sources': [
        '../third_party/native_app_glue/android_native_app_glue.c',
        '../third_party/native_app_glue/android_native_app_glue.h',
      ],
      'cflags': [
        '-w',
      ],
    },
    {
      'target_name': 'cpu_features',
      'type': 'static_library',
      'direct_dependent_settings': {
        'include_dirs': [
          '../third_party/cpufeatures',
        ],
      },
      'sources': [
        '../third_party/cpufeatures/cpu-features.c',
        '../third_party/cpufeatures/cpu-features.h',
      ],
      'cflags': [
        '-w',
      ],
    },
    {
      'target_name': 'ashmem',
      'type': 'static_library',
      'sources': [
        '../third_party/ashmem/cutils/ashmem.h',
        '../third_party/ashmem/cutils/ashmem-dev.c'
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../third_party/ashmem',
        ]
      },
    },
    {
      # This target is a dependency for all console-type Skia applications which
      # will run on Android.  Since Android requires us to load native code in
      # shared libraries, we need a common entry point to wrap around main().
      # Here we also change the type of all would-be executables to be shared
      # libraries.  The alternative would be to introduce a condition in every
      # executable target which changes to a shared library if the target OS is
      # Android.  This is nicer because the switch is in one place.
      'target_name': 'Android_EntryPoint',
      'type': 'static_library',
      'direct_dependent_settings': {
        'target_conditions': [
          # '_type' is an 'automatic variable' which is defined for any
          # target which defines a key-value pair with 'type' as the key (so,
          # all of them).  Conditionals inside 'target_conditions' are evaluated
          # *after* all other definitions and conditionals are evaluated, so
          # we're guaranteed that '_type' will be defined when we get here.
          # For more info, see:
          # - http://code.google.com/p/gyp/wiki/InputFormatReference#Variables
          # - http://codereview.appspot.com/6353065/
          ['_type == "executable"', {
            'type': 'shared_library',
          }],
        ],
      },
    },
    {
      # This target is a dependency for Skia Sample application which runs on
      # Android.  Since Android requires us to load native code in shared
      # libraries, we need a common entry point to wrap around main(). Here
      # we also change the type of all would-be executables to be shared
      # libraries.  The alternative would be to introduce a condition in every
      # executable target which changes to a shared library if the target OS is
      # Android.  This is nicer because the switch is in one place.
      'target_name': 'Android_SampleApp',
      'type': 'static_library',
      'direct_dependent_settings': {
        'target_conditions': [
          # '_type' is an 'automatic variable' which is defined for any
          # target which defines a key-value pair with 'type' as the key (so,
          # all of them).  Conditionals inside 'target_conditions' are evaluated
          # *after* all other definitions and conditionals are evaluated, so
          # we're guaranteed that '_type' will be defined when we get here.
          # For more info, see:
          # - http://code.google.com/p/gyp/wiki/InputFormatReference#Variables
          # - http://codereview.appspot.com/6353065/
          ['_type == "executable"', {
            'type': 'shared_library',
          }],
        ],
        'cflags': [
          '-Wno-unused-private-field',
        ],
        'sources': [
          '../apps/sample_app/src/main/jni/com_skia_SkiaSampleRenderer.cpp',
        ],
      },
    },
  ]
}
