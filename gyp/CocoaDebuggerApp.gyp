{
  'includes': [
    'target_defaults.gypi',
  ],
  'targets': [
    {
      'target_name': 'CocoaDebuggerApp',
      'type': 'executable',
      'mac_bundle' : 1,
      
      'include_dirs' : [
        '../include/pipe',
        '../experimental/CocoaDebugger',
        '../experimental/SimpleCocoaApp',
      ],
      'sources': [
        '../experimental/CocoaDebugger/SkCommandListView.cpp',
        '../experimental/CocoaDebugger/SkContentView.cpp',
        '../experimental/CocoaDebugger/SkDebugDumper.cpp',
        '../experimental/CocoaDebugger/SkDumpCanvasM.cpp',
        '../experimental/CocoaDebugger/SkInfoPanelView.cpp',
        '../src/pipe/SkGPipeRead.cpp',
      ],
      'dependencies': [
        'core.gyp:core',
        'effects.gyp:effects',
        'opts.gyp:opts',
        'utils.gyp:utils',
        'views.gyp:views',
        'xml.gyp:xml',
      ],
      'conditions' : [
        # Only supports Mac currently
        ['OS == "mac"', {
          'sources': [
            '../experimental/CocoaDebugger/CocoaDebugger-Info.plist',
            '../experimental/CocoaDebugger/CocoaDebugger_Prefix.pch',
            '../experimental/CocoaDebugger/CocoaDebuggerAppDelegate.mm',
            '../experimental/CocoaDebugger/main.m',
            '../experimental/CocoaDebugger/SkDebugger.mm',
            '../experimental/CocoaDebugger/SkMenuController.mm',
            '../experimental/SimpleCocoaApp/SkNSView.mm',
            '../experimental/SimpleCocoaApp/SkNSWindow.mm',
            '../include/utils/mac/SkCGUtils.h',
            '../src/utils/mac/SkCreateCGImageRef.cpp',
          ],
          'link_settings': {
            'libraries': [
            '$(SDKROOT)/System/Library/Frameworks/Cocoa.framework',
            '$(SDKROOT)/System/Library/Frameworks/AppKit.framework',
            '$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
            '$(SDKROOT)/System/Library/Frameworks/OpenGL.framework',
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
            'INFOPLIST_FILE' : '../experimental/CocoaDebugger/CocoaDebugger-Info.plist',
          },
          'mac_bundle_resources' : [
            '../experimental/CocoaDebugger/English.lproj/InfoPlist.strings',
            '../experimental/CocoaDebugger/English.lproj/MainMenu.xib',
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
