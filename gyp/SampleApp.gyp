# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
{
  'targets': [
    {
      'target_name': 'SampleApp',
      'type': 'executable',
      'include_dirs' : [
        '../include/private',
        '../src/core',
        '../src/effects', #needed for BlurMask.h
        '../src/gpu', # needed by SkLua.cpp
        '../src/image',
        '../src/images',
        '../src/lazy',
        '../src/pathops',
        '../gm',       # needed to pull gm.h
        '../samplecode', # To pull SampleApp.h and SampleCode.h
        '../tools/debugger',
      ],
      'includes': [
        'gmslides.gypi',
        'samples.gypi',
      ],
      'sources': [
        '../gm/gm.cpp',
        '../samplecode/SampleApp.cpp',

      ],
      'dependencies': [
        'etc1.gyp:libetc1',
        'flags.gyp:flags',
        'jsoncpp.gyp:jsoncpp',
        'skia_lib.gyp:skia_lib',
        'gputest.gyp:skgputest',
        'tools.gyp:resources',
        'tools.gyp:sk_tool_utils',
        'tools.gyp:timer',
        'tools.gyp:url_data_manager',
        'views.gyp:views',
      ],
      'msvs_settings': {
        'VCLinkerTool': {
          #Allows for creation / output to console.
          #Console (/SUBSYSTEM:CONSOLE)
          'SubSystem': '1',

          #Console app, use main/wmain
          'EntryPointSymbol': 'mainCRTStartup',
        },
      },
      'conditions' : [
        [ 'skia_os == "ios"', {
          'mac_bundle' : 1,
          # TODO: This doesn't build properly yet, but it's getting there.
          'sources': [
            '../src/views/mac/SkEventNotifier.mm',
            '../experimental/iOSSampleApp/SkSampleUIView.mm',
            '../experimental/iOSSampleApp/SkiOSSampleApp-Base.xcconfig',
            '../experimental/iOSSampleApp/SkiOSSampleApp-Debug.xcconfig',
            '../experimental/iOSSampleApp/SkiOSSampleApp-Release.xcconfig',
            '../experimental/iOSSampleApp/iOSSampleApp-Info.plist',
            '../experimental/iOSSampleApp/Shared/SkOptionListController.mm',
            '../experimental/iOSSampleApp/Shared/SkUIRootViewController.mm',
            '../experimental/iOSSampleApp/Shared/SkOptionsTableViewController.mm',
            '../experimental/iOSSampleApp/Shared/SkUIView.mm',
            '../experimental/iOSSampleApp/Shared/SkUIDetailViewController.mm',
            '../experimental/iOSSampleApp/Shared/skia_ios.mm',

            # iPad
            '../experimental/iOSSampleApp/iPad/AppDelegate_iPad.mm',
            '../experimental/iOSSampleApp/iPad/SkUISplitViewController.mm',

            # iPhone
            '../experimental/iOSSampleApp/iPhone/AppDelegate_iPhone.mm',
            '../experimental/iOSSampleApp/iPhone/SkUINavigationController.mm',

            '../src/views/ios/SkOSWindow_iOS.mm',

            '../src/utils/mac/SkCreateCGImageRef.cpp',
            '../experimental/iOSSampleApp/SkiOSSampleApp-Debug.xcconfig',
            '../experimental/iOSSampleApp/SkiOSSampleApp-Release.xcconfig',
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
            'INFOPLIST_FILE' : '../experimental/iOSSampleApp/iOSSampleApp-Info.plist',
          },
          'xcode_config_file': '../experimental/iOSSampleApp/SkiOSSampleApp-Base.xcconfig',
          'mac_bundle_resources' : [
            '../experimental/iOSSampleApp/iPad/MainWindow_iPad.xib',
            '../experimental/iOSSampleApp/iPhone/MainWindow_iPhone.xib',
          ],
        }],
        [ 'skia_os == "android"', {
          'conditions': [
            ['skia_android_framework == 0', {
              'dependencies': [
                'android_deps.gyp:Android_EntryPoint',
                'skia_launcher.gyp:skia_launcher',
              ],
            }],
          ],
          'dependencies!': [
            'experimental.gyp:experimental',
          ],
          'dependencies': [
            'android_output.gyp:android_output',
            'android_deps.gyp:Android_SampleApp',
          ],
        }],
        [ 'skia_gpu == 1', {
          'dependencies': [
            'gputest.gyp:skgputest',
          ],
        }],
        [ 'not skia_pdf', {
          'dependencies!': [ 'pdf.gyp:pdf' ],
          'dependencies': [ 'pdf.gyp:nopdf' ],
        }],
      ],
    },
  ],
}
