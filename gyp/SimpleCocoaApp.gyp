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
        '../src/views/mac/SkEventNotifier.h',
        '../src/views/mac/SkEventNotifier.mm',
        '../src/views/mac/skia_mac.mm',
        '../src/views/mac/SkNSView.h',
        '../src/views/mac/SkNSView.mm',
        '../src/views/mac/SkOptionsTableView.h',
        '../src/views/mac/SkOptionsTableView.mm',
        '../src/views/mac/SkOSWindow_Mac.mm',
        '../src/views/mac/SkTextFieldCell.h',
        '../src/views/mac/SkTextFieldCell.m',

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
