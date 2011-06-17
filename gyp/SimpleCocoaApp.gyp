{
  'includes': [
    'target_defaults.gypi',
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
        '../experimental/SimpleCocoaApp/main.m',
        '../experimental/SimpleCocoaApp/SampleWindow.mm',
        '../experimental/SimpleCocoaApp/SimpleCocoaApp-Info.plist',
        '../experimental/SimpleCocoaApp/SimpleCocoaApp_Prefix.pch',
        '../experimental/SimpleCocoaApp/SimpleCocoaAppDelegate.mm',
        '../experimental/SimpleCocoaApp/SkNSView.mm',
        '../experimental/SimpleCocoaApp/SkNSWindow.mm',
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
        [ 'OS == "mac"', {
          'sources': [
            '../include/utils/mac/SkCGUtils.h',
            '../src/utils/mac/SkCreateCGImageRef.cpp',
          ],
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/Cocoa.framework',
              '$(SDKROOT)/System/Library/Frameworks/AppKit.framework',
            ],
            'libraries!': [
              # Currently skia mac apps rely on Carbon and AGL for UI. Future
              # apps should use Cocoa instead and dependencies on Carbon and AGL
              # should eventually be removed
              '$(SDKROOT)/System/Library/Frameworks/Carbon.framework',
              '$(SDKROOT)/System/Library/Frameworks/AGL.framework',
            ],
          },
          'xcode_settings' : {
            'INFOPLIST_FILE' : '../experimental/SimpleCocoaApp/SimpleCocoaApp-Info.plist',
          },
          'mac_bundle_resources' : [
            '../experimental/SimpleCocoaApp/English.lproj/InfoPlist.strings',
            '../experimental/SimpleCocoaApp/English.lproj/MainMenu.xib',
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
