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
        '../src/images',
        '../src/lazy',
        '../gm',       # needed to pull gm.h
        '../samplecode', # To pull SampleApp.h and SampleCode.h
        '../src/utils/debugger',
        '../tools',
        '../experimental',
      ],
      'includes': [
        'gmslides.gypi',
      ],
      'sources': [
        '../gm/gm.cpp',
        '../samplecode/GMSampleView.h',
        '../samplecode/GMSampleView.cpp',
        '../samplecode/ClockFaceView.cpp',
        '../samplecode/OverView.cpp',
        '../samplecode/OverView.h',
        '../samplecode/PerlinPatch.cpp',
        '../samplecode/Sample2PtRadial.cpp',
        '../samplecode/SampleAAClip.cpp',
        '../samplecode/SampleAARects.cpp',
        '../samplecode/SampleAARectModes.cpp',
        '../samplecode/SampleAll.cpp',
        '../samplecode/SampleAnimatedText.cpp',
        '../samplecode/SampleAnimator.cpp',
        '../samplecode/SampleAnimBlur.cpp',
        '../samplecode/SampleApp.cpp',
        '../samplecode/SampleArc.cpp',
        '../samplecode/SampleAtlas.cpp',
        '../samplecode/SampleBigBlur.cpp',
        '../samplecode/SampleBigGradient.cpp',
        '../samplecode/SampleBitmapRect.cpp',
        '../samplecode/SampleBlur.cpp',
        '../samplecode/SampleCamera.cpp',
        '../samplecode/SampleChart.cpp',
        '../samplecode/SampleCircle.cpp',
        '../samplecode/SampleClip.cpp',
        '../samplecode/SampleClipDrawMatch.cpp',
        '../samplecode/SampleClock.cpp',
        '../samplecode/SampleCode.h',
        '../samplecode/SampleColorFilter.cpp',
        '../samplecode/SampleComplexClip.cpp',
        '../samplecode/SampleConcavePaths.cpp',
        '../samplecode/SampleDegenerateTwoPtRadials.cpp',
        '../samplecode/SampleDither.cpp',
        '../samplecode/SampleDitherBitmap.cpp',
        '../samplecode/SampleEffects.cpp',
        '../samplecode/SampleEmboss.cpp',
        '../samplecode/SampleFatBits.cpp',
        '../samplecode/SampleFillType.cpp',
        '../samplecode/SampleFilter.cpp',
        '../samplecode/SampleFilter2.cpp',
        '../samplecode/SampleFilterQuality.cpp',
        '../samplecode/SampleFilterFuzz.cpp',
        '../samplecode/SampleFontCache.cpp',
        '../samplecode/SampleFontScalerTest.cpp',
        '../samplecode/SampleFuzz.cpp',
        '../samplecode/SampleGradients.cpp',
        '../samplecode/SampleHairCurves.cpp',
        '../samplecode/SampleHairline.cpp',
        '../samplecode/SampleHairModes.cpp',
        '../samplecode/SampleHT.cpp',
        '../samplecode/SampleIdentityScale.cpp',
        '../samplecode/SampleLayerMask.cpp',
        '../samplecode/SampleLayers.cpp',
        '../samplecode/SampleLCD.cpp',
        '../samplecode/SampleLighting.cpp',
        '../samplecode/SampleLines.cpp',
        '../samplecode/SampleLua.cpp',
        '../samplecode/SampleManyRects.cpp',
        '../samplecode/SampleMeasure.cpp',
        '../samplecode/SamplePatch.cpp',
        '../samplecode/SamplePath.cpp',
        '../samplecode/SamplePathClip.cpp',
        '../samplecode/SamplePathFuzz.cpp',
        '../samplecode/SamplePathEffects.cpp',
        '../samplecode/SamplePictFile.cpp',
        '../samplecode/SamplePoints.cpp',
        '../samplecode/SamplePolyToPoly.cpp',
        '../samplecode/SampleQuadStroker.cpp',
        '../samplecode/SampleRectanizer.cpp',
        '../samplecode/SampleRegion.cpp',
        '../samplecode/SampleRepeatTile.cpp',
        '../samplecode/SampleShaders.cpp',
        '../samplecode/SampleShaderText.cpp',
        '../samplecode/SampleShip.cpp',
        '../samplecode/SampleSkLayer.cpp',
        '../samplecode/SampleSlides.cpp',
        '../samplecode/SampleStringArt.cpp',
        '../samplecode/SampleStrokePath.cpp',
        '../samplecode/SampleSubpixelTranslate.cpp',
        '../samplecode/SampleText.cpp',
        '../samplecode/SampleTextAlpha.cpp',
        '../samplecode/SampleTextBox.cpp',
        '../samplecode/SampleTextOnPath.cpp',
        '../samplecode/SampleTextureDomain.cpp',
        '../samplecode/SampleTiling.cpp',
        '../samplecode/SampleTinyBitmap.cpp',
        '../samplecode/SampleUnpremul.cpp',
        '../samplecode/SampleVertices.cpp',
        '../samplecode/SampleXfermodesBlur.cpp',
        '../samplecode/SampleXfer.cpp',

        # DrawingBoard
        #'../experimental/DrawingBoard/SkColorPalette.h',
        #'../experimental/DrawingBoard/SkColorPalette.cpp',
        #'../experimental/DrawingBoard/SkNetPipeController.h',
        #'../experimental/DrawingBoard/SkNetPipeController.cpp',
        #'../experimental/DrawingBoard/SampleDrawingClient.cpp',
        #'../experimental/DrawingBoard/SampleDrawingServer.cpp',

        # Networking
        #'../experimental/Networking/SampleNetPipeReader.cpp',
        #'../experimental/Networking/SkSockets.cpp',
        #'../experimental/Networking/SkSockets.h',

        # PerlinNoise2
        '../experimental/SkPerlinNoiseShader2/SkPerlinNoiseShader2.cpp',
        '../experimental/SkPerlinNoiseShader2/SkPerlinNoiseShader2.h',

        # Lua
        '../src/utils/SkLuaCanvas.cpp',
        '../src/utils/SkLua.cpp',
      ],
      'sources!': [
        '../samplecode/SampleSkLayer.cpp', #relies on SkMatrix44 which doesn't compile
        '../samplecode/SampleFontCache.cpp',
      ],
      'dependencies': [
        'animator.gyp:animator',
        'etc1.gyp:libetc1',
        'experimental.gyp:experimental',
        'flags.gyp:flags',
        'lua.gyp:lua',
        'pdf.gyp:pdf',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:resources',
        'tools.gyp:sk_tool_utils',
        'tools.gyp:timer',
        'views.gyp:views',
        'views_animated.gyp:views_animated',
        'xml.gyp:xml',
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
          'sources!': [
            '../samplecode/SampleAnimator.cpp',
          ],
          'conditions': [
            ['skia_android_framework == 0', {
              'dependencies': [
                'android_deps.gyp:Android_EntryPoint',
                'skia_launcher.gyp:skia_launcher',
              ],
            }],
          ],
          'dependencies!': [
            'animator.gyp:animator',
            'experimental.gyp:experimental',
          ],
          'dependencies': [
            'android_output.gyp:android_output',
            'android_deps.gyp:Android_SampleApp',
          ],
        }],
        [ 'skia_os == "chromeos"', {
          'sources!': [
            '../samplecode/SampleLighting.cpp',  #doesn't compile due to gpu dependencies
          ],
          'include_dirs' : [
            '../include/gpu',
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
