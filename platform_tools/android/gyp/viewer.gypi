# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'targets': [
    {
      'target_name': 'CopyViewerDeps',
      'type': 'none',
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'viewer.gyp:viewer',
      ],
      'copies': [
        {
          'destination': '<(android_base)/apps/viewer/src/main/libs/<(android_arch)',
          'conditions': [
            [ 'skia_shared_lib', {
              'files': [
                '<(SHARED_LIB_DIR)/libskia_android.so',
                '<(SHARED_LIB_DIR)/libviewer.so',
              ]}, {
              'files': [
                '<(SHARED_LIB_DIR)/libviewer.so',
              ]}
           ],
          ],
        },
      ],
    },
    {
      'target_name': 'Viewer_APK',
      'type': 'none',
      'dependencies': [ 'CopyViewerDeps', ],
      'actions': [
        {
          'action_name': 'SkiaViewer_apk',
          'inputs': [
            '<(android_base)/apps/viewer/src/main/AndroidManifest.xml',
            '<(android_base)/apps/viewer/src/main/java/org/skia/viewer/ViewerActivity.java',
            '<(android_base)/apps/viewer/src/main/libs/<(android_arch)/libviewer.so',

          ],
          'conditions': [
            [ 'skia_shared_lib', {
              'inputs': [
                '<(android_base)/apps/viewer/src/main/libs/<(android_arch)/libskia_android.so',
              ],
            }],
          ],
          'outputs': [
            '../apps/viewer/build/outputs/apk/',
          ],
          'action': [
            '<(android_base)/apps/gradlew',
            ':viewer:assemble<(android_variant)<(android_buildtype)',
            '-p<(android_base)/apps/viewer',
            '-PsuppressNativeBuild',
            '--daemon',
          ],
        },
      ],
    },
  ],
}
