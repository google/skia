# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'targets': [
    {
      'target_name': 'CopyVulkanViewerDeps',
      'type': 'none',
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'vulkanviewer.gyp:vulkanviewer',
      ],
      'copies': [
        {
          'destination': '<(android_base)/apps/vulkanviewer/src/main/libs/<(android_arch)',
          'conditions': [
            [ 'skia_shared_lib', {
              'files': [
                '<(SHARED_LIB_DIR)/libskia_android.so',
                '<(SHARED_LIB_DIR)/libvulkanviewer.so',
              ]}, {
              'files': [
                '<(SHARED_LIB_DIR)/libvulkanviewer.so',
              ]}
           ],
          ],
        },
      ],
    },
    {
      'target_name': 'VulkanViewer_APK',
      'type': 'none',
      'dependencies': [ 'CopyVulkanViewerDeps', ],
      'actions': [
        {
          'action_name': 'SkiaVulkanViewer_apk',
          'inputs': [
            '<(android_base)/apps/vulkanviewer/src/main/AndroidManifest.xml',
            '<(android_base)/apps/vulkanviewer/src/main/java/com/skia/vulkanviewer/VulkanViewerActivity.java',
            '<(android_base)/apps/vulkanviewer/src/main/libs/<(android_arch)/libvulkanviewer.so',

          ],
          'conditions': [
            [ 'skia_shared_lib', {
              'inputs': [
                '<(android_base)/apps/vulkanviewer/src/main/libs/<(android_arch)/libskia_android.so',
              ],
            }],
          ],
          'outputs': [
            '../apps/vulkanviewer/build/outputs/apk/',
          ],
          'action': [
            '<(android_base)/apps/gradlew',
            ':vulkanviewer:assemble<(android_variant)<(android_buildtype)',
            '-p<(android_base)/apps/vulkanviewer',
            '-PsuppressNativeBuild',
            '--daemon',
          ],
        },
      ],
    },
  ],
}
