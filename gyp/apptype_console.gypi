# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
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
        'conditions': [
          ['skia_android_framework == 0', {
            'dependencies': [
              'android_deps.gyp:Android_EntryPoint',
              'skia_launcher.gyp:skia_launcher',
            ],
          }],
        ],
        'dependencies': [
          'android_output.gyp:android_output',
        ],
      }],
      ['skia_os == "ios"', {
        'target_conditions': [
          ['_type == "executable"', {
            'mac_bundle' : 1,
            'sources': [
              '../src/views/ios/SkOSWindow_iOS.mm',
              '../src/views/mac/SkEventNotifier.mm',
              '../experimental/iOSSampleApp/iPad/AppDelegate_iPad.mm',
              '../experimental/iOSSampleApp/iPhone/AppDelegate_iPhone.mm',
              '../experimental/iOSSampleApp/Shared/SkUIView.mm',
              '../experimental/iOSSampleApp/Shared/skia_ios.mm',
              '../experimental/SimpleiOSApp/SimpleApp.mm',
            ],
            'include_dirs' : [
              '../experimental/iOSSampleApp/Shared',
              '../include/views',
              '../include/utils/mac',
              '../src/views/mac',
            ],
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
        'dependencies': [
          'views.gyp:views',
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
      }],
    ],
  },
}
