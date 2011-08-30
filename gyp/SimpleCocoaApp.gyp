{
  'includes': [
    'common.gypi',
  ],
  'targets': [
    {
      'target_name': 'SimpleCocoaApp',
      'type': 'executable',
      'mac_bundle' : 1,
      'include_dirs' : [
        '../experimental/SimpleCocoaApp/',
      ],
      'sources': [
        '../src/utils/mac/SkEventNotifier.h',
        '../src/utils/mac/SkEventNotifier.mm',
        '../src/utils/mac/skia_mac.mm',
        '../src/utils/mac/SkNSView.h',
        '../src/utils/mac/SkNSView.mm',
        '../src/utils/mac/SkOptionsTableView.h',
        '../src/utils/mac/SkOptionsTableView.mm',
        '../src/utils/mac/SkOSWindow_Mac.mm',
        '../src/utils/mac/SkTextFieldCell.h',
        '../src/utils/mac/SkTextFieldCell.m',
        
        '../experimental/SimpleCocoaApp/SimpleApp-Info.plist',
        '../experimental/SimpleCocoaApp/SimpleApp.h',
        '../experimental/SimpleCocoaApp/SimpleApp.mm',
        
      ],
      'dependencies': [
        'core.gyp:core',
        'opts.gyp:opts',
        'utils.gyp:utils',
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

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
