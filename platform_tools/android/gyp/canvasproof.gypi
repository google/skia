# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'targets': [
    {
      'target_name': 'canvasproof',
      'type': 'shared_library',
      'dependencies': [ 'skia_lib.gyp:skia_lib', ],
      'sources': [
        '../apps/canvasproof/src/main/jni/JavaInputStream.cpp',
        '../apps/canvasproof/src/main/jni/JavaInputStream.h',
        '../apps/canvasproof/src/main/jni/org_skia_canvasproof_GaneshPictureRenderer.cpp',
        '../apps/canvasproof/src/main/jni/org_skia_canvasproof_GaneshPictureRenderer.h',
        '../apps/canvasproof/src/main/jni/org_skia_canvasproof_CreateSkiaPicture.cpp',
        '../apps/canvasproof/src/main/jni/org_skia_canvasproof_CreateSkiaPicture.h  ',
      ],
    },
    {
      'target_name': 'CopyCanvasProofDeps',
      'type': 'none',
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'canvasproof',
      ],
      'copies': [
        {
          'destination': '../apps/canvasproof/src/main/libs/<(android_arch)',
          'conditions': [
            [ 'skia_shared_lib', {
              'files': [
                '<(SHARED_LIB_DIR)/libskia_android.so',
                '<(SHARED_LIB_DIR)/libcanvasproof.so',
              ]}, {
              'files': [
                '<(SHARED_LIB_DIR)/libcanvasproof.so',
              ]}
           ],
          ],
        },
      ],
    },
    {
      'target_name': 'CanvasProof_APK',
      'type': 'none',
      'dependencies': [ 'CopyCanvasProofDeps', ],
      'actions': [
        {
          'action_name': 'SkiaCanvasProof_apk',
          'inputs': [
            '../apps/canvasproof/src/main/assets/skps',
            '../apps/canvasproof/src/main/AndroidManifest.xml',
            '../apps/canvasproof/src/main/java/org/skia/canvasproof/CreateSkiaPicture.java',
            '../apps/canvasproof/src/main/java/org/skia/canvasproof/CanvasProofActivity.java',
            '../apps/canvasproof/src/main/java/org/skia/canvasproof/GaneshPictureRenderer.java',
            '../apps/canvasproof/src/main/java/org/skia/canvasproof/HwuiPictureView.java',
            '<(android_base)/apps/canvasproof/src/main/libs/<(android_arch)/libcanvasproof.so',
            '<(android_base)/apps/canvasproof/src/main/libs/<(android_arch)/libskia_android.so',

          ],
          'outputs': [
            '../apps/canvasproof/build',
          ],
          'action': [
            '<(android_base)/apps/gradlew',
            ':canvasproof:assemble<(android_variant)Debug',
            '-p<(android_base)/apps/canvasproof',
            '-PsuppressNativeBuild',
          ],
        },
      ],
    },
  ],
}
