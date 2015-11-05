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
      [ 'android_buildtype == "Release"', {
        'android_apk_suffix': "release.apk",
      }, {
        'android_apk_suffix': "debug.apk",
      }],
    ],
  },
  'includes' : [ 'canvasproof.gypi', ],
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
          'destination': '<(android_base)/apps/sample_app/src/main/libs/<(android_arch)',
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
      'actions': [
        {
          'action_name': 'SampleApp_apk',
          'inputs': [
            '<(android_base)/apps/sample_app/src/main/AndroidManifest.xml',
            '<(android_base)/apps/sample_app/src/main/jni/com_skia_SkiaSampleRenderer.h',
            '<(android_base)/apps/sample_app/src/main/jni/com_skia_SkiaSampleRenderer.cpp',
            '<(android_base)/apps/sample_app/src/main/java/com/skia/SkiaSampleActivity.java',
            '<(android_base)/apps/sample_app/src/main/java/com/skia/SkiaSampleRenderer.java',
            '<(android_base)/apps/sample_app/src/main/java/com/skia/SkiaSampleView.java',
            '<(android_base)/apps/sample_app/src/main/libs/<(android_arch)/libSampleApp.so',
          ],
          'conditions': [
            [ 'skia_shared_lib', {
              'inputs': [
                '<(android_base)/apps/sample_app/src/main/libs/<(android_arch)/libskia_android.so',
              ],
            }],
          ],
          'outputs': [
            '<(android_base)/apps/sample_app/build/outputs/apk/sample_app-<(android_variant)-<(android_apk_suffix)',
          ],
          'action': [
            '<(android_base)/apps/gradlew',
            ':sample_app:assemble<(android_variant)<(android_buildtype)',
            '-p<(android_base)/apps/sample_app',
            '-PsuppressNativeBuild',
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
          'destination': '<(android_base)/apps/visualbench/src/main/libs/<(android_arch)',
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
      'actions': [
        {
          'action_name': 'SkiaVisualBench_apk',
          'inputs': [
            '<(android_base)/apps/visualbench/src/main/AndroidManifest.xml',
            '<(android_base)/apps/visualbench/src/main/java/com/skia/VisualBenchActivity.java',
            '<(android_base)/apps/visualbench/src/main/libs/<(android_arch)/libvisualbench.so',
          ],
          'conditions': [
            [ 'skia_shared_lib', {
              'inputs': [
                '<(android_base)/apps/visualbench/src/main/libs/<(android_arch)/libskia_android.so',
              ],
            }],
          ],
          'outputs': [
            '<(android_base)/apps/visualbench/build/outputs/apk/visualbench-<(android_variant)-<(android_apk_suffix)',
          ],
          'action': [
            '<(android_base)/apps/gradlew',
            ':visualbench:assemble<(android_variant)<(android_buildtype)',
            '-p<(android_base)/apps/visualbench',
            '-PsuppressNativeBuild',
          ],
        },
      ],
    },
    {
      'target_name': 'VisualBenchTest_APK',
      'type': 'none',
      'dependencies': [
        'VisualBench_APK',
      ],
      'actions': [
        {
          'action_name': 'SkiaVisualBench_apk',
          'inputs': [
            '<(android_base)/apps/visualbench/src/main/AndroidManifest.xml',
            '<(android_base)/apps/visualbench/src/main/java/com/skia/VisualBenchActivity.java',
            '<(android_base)/apps/visualbench/src/main/java/com/skia/VisualBenchTestActivity.java',
            '<(android_base)/apps/visualbench/src/main/libs/<(android_arch)/libvisualbench.so',
          ],
          'conditions': [
            [ 'skia_shared_lib', {
              'inputs': [
                '<(android_base)/apps/visualbench/src/main/libs/<(android_arch)/libskia_android.so',
              ],
            }],
          ],
          'outputs': [
            '<(android_base)/apps/visualbench/build/outputs/apk/visualbench-<(android_variant)-debug-androidTest-unaligned.apk',
          ],
          'action': [
            '<(android_base)/apps/gradlew',
            ':visualbench:assemble<(android_variant)DebugAndroidTest',
            '-p<(android_base)/apps/visualbench',
            '-PsuppressNativeBuild',
          ],
        },
      ],
    },
    # TODO all of this duplicated code can be removed when SDL becomes the default
    # Currently, to use this you have to override skia_use_sdl
    {
      'target_name': 'CopyVisualBenchSDLDeps',
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
          'destination': '<(android_base)/apps/visualbenchsdl/src/main/libs/<(android_arch)',
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
      'target_name': 'VisualBenchSDL_APK',
      'type': 'none',
      'dependencies': [
        'CopyVisualBenchSDLDeps',
      ],
      'actions': [
        {
          'action_name': 'SkiaVisualBenchSDL_apk',
          'inputs': [
            '<(android_base)/apps/visualbenchsdl/src/main/AndroidManifest.xml',
            '<(android_base)/apps/visualbenchsdl/src/main/java/org/libsdl/app/SDLActivity.java',
            '<(android_base)/apps/visualbenchsdl/src/main/java/com/skia/VisualBenchActivity.java',
            '<(android_base)/apps/visualbenchsdl/src/main/libs/<(android_arch)/libvisualbench.so',
          ],
          'conditions': [
            [ 'skia_shared_lib', {
              'inputs': [
                '<(android_base)/apps/visualbenchsdl/src/main/libs/<(android_arch)/libskia_android.so',
              ],
            }],
          ],
          'outputs': [
            '<(android_base)/apps/visualbenchsdl/build/outputs/apk/visualbench-<(android_variant)-<(android_apk_suffix)',
          ],
          'action': [
            '<(android_base)/apps/gradlew',
            ':visualbenchsdl:assemble<(android_variant)<(android_buildtype)',
            '-p<(android_base)/apps/visualbenchsdl',
            '-PsuppressNativeBuild',
          ],
        },
      ],
    },
  ],
}
