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
      }],
      [ 'skia_arch_type == "arm" and arm_version == 7', {
        'android_arch%': "armeabi-v7a",
      }],
      [ 'skia_arch_type == "arm64"', {
        'android_arch%': "arm64-v8a",
      }],
      [ 'skia_arch_type == "x86"', {
        'android_arch%': "x86",
      }],
      [ 'skia_arch_type == "x86_64"', {
        'android_arch%': "x86_64",
      }],
      [ 'skia_arch_type == "mips" and skia_arch_width == 32', {
        'android_arch%': "mips",
      }],
      [ 'skia_arch_type == "mips" and skia_arch_width == 64', {
        'android_arch%': "mips64",
      }],
    ],
  },
  'targets': [
    {
      'target_name': 'CopySampleAppDeps',
      'type': 'none',
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'SampleApp.gyp:SampleApp',
      ],
      'copies': [
        # Copy all shared libraries into the Android app's libs folder.  Note
        # that this copy requires us to build SkiaAndroidApp after those
        # libraries, so that they exist by the time it occurs.  If there are no
        # libraries to copy, this will cause an error in Make, but the app will
        # still build.
        {
          'destination': '<(PRODUCT_DIR)/android/SampleApp/libs/<(android_arch)',
          'conditions': [
            [ 'skia_shared_lib', {
              'files': [
                '<(SHARED_LIB_DIR)/libSampleApp.so',
                '<(SHARED_LIB_DIR)/libskia_android.so',
              ]}, {
              'files': [
                '<(SHARED_LIB_DIR)/libSampleApp.so',
             ]}
           ],
          ],
        },
      ],
    },
    {
      'target_name': 'SampleApp_APK',
      'type': 'none',
      'dependencies': [
        'CopySampleAppDeps',
      ],
      'variables': {
         'ANDROID_SDK_ROOT': '<!(echo $ANDROID_SDK_ROOT)',
         # the ninja generator treats PRODUCT_DIR as a relative path to the
         # gyp directory but android ant build wants a path relative to the
         # build.xml file so we do that adjustment here.
         'ANDROID_OUT': '../../<(PRODUCT_DIR)/android/SampleApp'
       },
      'actions': [
        {
          'action_name': 'SampleApp_apk',
          'inputs': [
            '<(android_base)/app/AndroidManifest.xml',
            '<(android_base)/app/build.xml',
            '<(android_base)/app/project.properties',
            '<(android_base)/app/jni/com_skia_SkiaSampleRenderer.h',
            '<(android_base)/app/jni/com_skia_SkiaSampleRenderer.cpp',
            '<(android_base)/app/src/com/skia/SkiaSampleActivity.java',
            '<(android_base)/app/src/com/skia/SkiaSampleRenderer.java',
            '<(android_base)/app/src/com/skia/SkiaSampleView.java',
          ],
          'outputs': [
            '<(PRODUCT_DIR)/../android/SampleApp/bin/SampleApp.apk',
          ],
          'action': [
            'ant',
            '-quiet',
            '-f',
            '<(android_base)/app/build.xml',
            '-Dout.dir=<(ANDROID_OUT)/bin',
            '-Dgen.absolute.dir=<(ANDROID_OUT)/gen',
            '-Dnative.libs.absolute.dir=<(ANDROID_OUT)/libs',
            '-Dout.final.file=<(ANDROID_OUT)/bin/SampleApp.apk',
            '-Dsdk.dir=<(ANDROID_SDK_ROOT)',
            'debug',
          ],
        },
      ],
    },
    {
      'target_name': 'CopyVisualBenchDeps',
      'type': 'none',
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'visualbench.gyp:visualbench',
      ],

      'copies': [
        # Copy all shared libraries into the Android app's libs folder.  Note
        # that this copy requires us to build SkiaAndroidApp after those
        # libraries, so that they exist by the time it occurs.  If there are no
        # libraries to copy, this will cause an error in Make, but the app will
        # still build.
        {
          'destination': '<(PRODUCT_DIR)/android/VisualBench/libs/<(android_arch)',
          'conditions': [
            [ 'skia_shared_lib', {
              'files': [
                '<(SHARED_LIB_DIR)/libskia_android.so',
                '<(SHARED_LIB_DIR)/libvisualbench.so',
              ]}, {
              'files': [
                '<(SHARED_LIB_DIR)/libvisualbench.so',
             ]}
           ],
          ],
        },
      ],
    },
    {
      'target_name': 'VisualBench_APK',
      'type': 'none',
      'dependencies': [
        'CopyVisualBenchDeps',
      ],
      'variables': {
         'ANDROID_SDK_ROOT': '<!(echo $ANDROID_SDK_ROOT)',
         # the ninja generator treats PRODUCT_DIR as a relative path to the
         # gyp directory but android ant build wants a path relative to the
         # build.xml file so we do that adjustment here.
         'ANDROID_OUT': '../../<(PRODUCT_DIR)/android/VisualBench/'
       },
      'actions': [
        {
          'action_name': 'SkiaVisualBench_apk',
          'inputs': [
            '<(android_base)/visualbench/AndroidManifest.xml',
            '<(android_base)/visualbench/build.xml',
            '<(android_base)/visualbench/project.properties',
            '<(android_base)/visualbench/src/com/skia/VisualBenchActivity.java',
          ],
          'outputs': [
            '<(PRODUCT_DIR)/../android/VisualBench/bin/VisualBench.apk',
          ],
          'action': [
            'ant',
            '-quiet',
            '-f',
            '<(android_base)/visualbench/build.xml',
            '-Dout.dir=<(ANDROID_OUT)/bin',
            '-Dgen.absolute.dir=<(ANDROID_OUT)/gen',
            '-Dnative.libs.absolute.dir=<(ANDROID_OUT)/libs',
            '-Dout.final.file=<(ANDROID_OUT)/bin/VisualBench.apk',
            '-Dsdk.dir=<(ANDROID_SDK_ROOT)',
            'debug',
          ],
        },
      ],
    },
  ],
}
