# target_defaults used for executable targets that generate a console app
{
  'target_defaults': {
    'msvs_settings': {
      'VCLinkerTool': {
        #Allows for creation / output to console.
        #Console (/SUBSYSTEM:CONSOLE)
        'SubSystem': '1',

        #Console app, use main/wmain
        'EntryPointSymbol': 'mainCRTStartup',
      },
    },
    'conditions': [
      [ 'skia_os == "android"', {
        'dependencies': [
          'android_deps.gyp:Android_EntryPoint',
          'android_system.gyp:skia_launcher',
        ],
      }],
      [ 'skia_os == "nacl"', {
        'dependencies': [
          'nacl.gyp:nacl_interface',
        ],
      }],
      ['skia_os == "ios"', {
        'target_conditions': [
          ['_type == "executable"', {
            'mac_bundle' : 1,
          }],
        ],
        'include_dirs' : [
          '../experimental/iOSSampleApp/Shared',
          '../include/views',
          '../include/xml',
          '../include/utils/mac',
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
        ],
        'dependencies': [
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
        },
        'xcode_config_file': '../experimental/iOSSampleApp/SkiOSSampleApp-Base.xcconfig',
        'mac_bundle_resources' : [
          '../experimental/SimpleiOSApp/iPad/MainWindow_iPad.xib',
          '../experimental/SimpleiOSApp/iPhone/MainWindow_iPhone.xib',
        ],
        'xcode_settings' : {
          'INFOPLIST_FILE' : '../experimental/SimpleiOSApp/tool-Info.plist',
        },
      }],
    ],
  },
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
