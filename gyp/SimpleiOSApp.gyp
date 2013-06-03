{
  'conditions' : [
    [ 'skia_os != "ios"', {
      'error': '<!(set GYP_DEFINES=\"skia_os=\'ios\'\")'
    }],
  ],
  'targets': [
    {
      'target_name': 'SimpleiOSApp',
      'type': 'executable',
      'mac_bundle' : 1,
      'include_dirs' : [
        '../experimental/iOSSampleApp/Shared',
      ],
      'sources': [
        '../src/views/ios/SkOSWindow_iOS.mm',
        '../src/views/mac/SkEventNotifier.h',
        '../src/views/mac/SkEventNotifier.mm',
        '../experimental/iOSSampleApp/iPad/AppDelegate_iPad.h',
        '../experimental/iOSSampleApp/iPad/AppDelegate_iPad.mm',
        '../experimental/iOSSampleApp/iPhone/AppDelegate_iPhone.h',
        '../experimental/iOSSampleApp/iPhone/AppDelegate_iPhone.mm',
        '../experimental/iOSSampleApp/Shared/SkUIView.h',
        '../experimental/iOSSampleApp/Shared/SkUIView.mm',
        '../experimental/iOSSampleApp/Shared/skia_ios.mm',
        '../experimental/SimpleiOSApp/SimpleApp.h',
        '../experimental/SimpleiOSApp/SimpleApp.mm',
        '../experimental/SimpleiOSApp/SimpleiOSApp-Info.plist',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'views.gyp:views',
        'xml.gyp:xml',
      ],
      'link_settings': {
        'libraries': [
          '$(SDKROOT)/System/Library/Frameworks/CoreGraphics.framework',
          '$(SDKROOT)/System/Library/Frameworks/CoreText.framework',
          '$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
          '$(SDKROOT)/System/Library/Frameworks/ImageIO.framework',
          '$(SDKROOT)/System/Library/Frameworks/MobileCoreServices.framework',
          '$(SDKROOT)/System/Library/Frameworks/UIKit.framework',
        ],
        'libraries!': [
          #remove mac dependencies
          '$(SDKROOT)/System/Library/Frameworks/Cocoa.framework',
          '$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
          '$(SDKROOT)/System/Library/Frameworks/QuartzCore.framework',
          '$(SDKROOT)/System/Library/Frameworks/OpenGL.framework',
          '$(SDKROOT)/System/Library/Frameworks/ApplicationServices.framework',
        ],
      },
      'xcode_settings' : {
        'INFOPLIST_FILE' : '../experimental/SimpleiOSApp/SimpleiOSApp-Info.plist',
      },
      'xcode_config_file': '../experimental/iOSSampleApp/SkiOSSampleApp-Base.xcconfig',
      'mac_bundle_resources' : [
        '../experimental/SimpleiOSApp/iPad/MainWindow_iPad.xib',
        '../experimental/SimpleiOSApp/iPhone/MainWindow_iPhone.xib',
      ],
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
