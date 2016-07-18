  # Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
{
  'conditions' : [
    [ 'skia_os == "ios"', {
      'targets': [
        {
          'target_name': 'iOSShell',
          'type': 'executable',
          'mac_bundle' : 1,
          'includes': [
            'bench.gypi',
            'dm.gypi',
          ],
          'dependencies': [
            'tools.gyp:crash_handler',
            'tools.gyp:timer',
            'views.gyp:views',
            'xml.gyp:xml',
          ],
          'sources': [
            '../tests/skia_test.cpp',
            '../tools/iOSShell.cpp',
            '../src/views/mac/SkEventNotifier.mm',
            '../experimental/iOSSampleApp/SkiOSSampleApp-Base.xcconfig',
            '../experimental/iOSSampleApp/SkiOSSampleApp-Debug.xcconfig',
            '../experimental/iOSSampleApp/SkiOSSampleApp-Release.xcconfig',
            '../experimental/iOSShell/iOSShell-Info.plist',
            '../experimental/iOSSampleApp/Shared/SkUIRootViewController.mm',
            '../experimental/iOSSampleApp/Shared/SkUIView.mm',
            '../experimental/iOSSampleApp/Shared/skia_ios.mm',

            # iPad
            '../experimental/iOSSampleApp/iPad/AppDelegate_iPad.mm',
            '../experimental/iOSSampleApp/iPad/SkUISplitViewController.mm',
            '../experimental/iOSSampleApp/iPad/MainWindow_iPad.xib',

            # iPhone
            '../experimental/iOSSampleApp/iPhone/AppDelegate_iPhone.mm',
            '../experimental/iOSSampleApp/iPhone/SkUINavigationController.mm',
            '../experimental/iOSSampleApp/iPhone/MainWindow_iPhone.xib',

            '../src/views/ios/SkOSWindow_iOS.mm',
            '../src/utils/mac/SkCreateCGImageRef.cpp',
          ],
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/CoreFoundation.framework',
              '$(SDKROOT)/System/Library/Frameworks/CoreGraphics.framework',
              '$(SDKROOT)/System/Library/Frameworks/CoreText.framework',
              '$(SDKROOT)/System/Library/Frameworks/UIKit.framework',
              '$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
              '$(SDKROOT)/System/Library/Frameworks/QuartzCore.framework',
              '$(SDKROOT)/System/Library/Frameworks/OpenGLES.framework',
              '$(SDKROOT)/System/Library/Frameworks/ImageIO.framework',
              '$(SDKROOT)/System/Library/Frameworks/MobileCoreServices.framework',
            ],
          },
          'include_dirs' : [
            '../experimental/iOSSampleApp',
            '../experimental/iOSSampleApp/iPad',
            '../experimental/iOSSampleApp/iPhone',
            '../experimental/iOSSampleApp/Shared',
            '../include/utils/ios',
            '../src/views/mac',
          ],
          'xcode_settings' : {
            'INFOPLIST_FILE' : '../experimental/iOSShell/iOSShell-Info.plist',
          },
          'xcode_config_file': '../experimental/iOSSampleApp/SkiOSSampleApp-Base.xcconfig',
          'mac_bundle_resources' : [
            '../experimental/iOSSampleApp/iPad/MainWindow_iPad.xib',
            '../experimental/iOSSampleApp/iPhone/MainWindow_iPhone.xib',
          ],
          'conditions' : [
            [ 'skia_gpu == 1', {
              'dependencies': [
                'gputest.gyp:skgputest',
              ],
            }],
          ],
        },
      ],
    }],
  ]
}
