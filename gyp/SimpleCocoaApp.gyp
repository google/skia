# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'targets': [
    {
      'target_name': 'SimpleCocoaApp',
      'type': 'executable',
      'mac_bundle' : 1,
      'include_dirs' : [
        '../experimental/SimpleCocoaApp/',
      ],
      'sources': [
        '../experimental/SimpleCocoaApp/SimpleApp-Info.plist',
        '../experimental/SimpleCocoaApp/SimpleApp.h',
        '../experimental/SimpleCocoaApp/SimpleApp.mm',

      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'views.gyp:views',
        'xml.gyp:xml',
      ],
      'conditions' : [
        # Only supports Mac currently
        [ 'skia_os == "mac"', {
          'sources': [
            '../include/utils/mac/SkCGUtils.h',
            '../src/utils/mac/SkCreateCGImageRef.cpp',
          ],
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/OpenGL.framework',
            ],
          },
          'xcode_settings' : {
            'INFOPLIST_FILE' : '../experimental/SimpleCocoaApp/SimpleApp-Info.plist',
          },
          'mac_bundle_resources' : [
            '../experimental/SimpleCocoaApp/SimpleApp.xib',
          ],
        }],
      ],
    },
  ],
}
