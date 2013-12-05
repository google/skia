#
{
  'variables': {
    #manually set sample_pdf_file_viewer to 1 to have the PdfViewer in SampleApp
    'sample_pdf_file_viewer%': 0,
  },
  'targets': [
    {
      'target_name': 'SampleApp',
      'type': 'executable',
      'mac_bundle' : 1,
      'include_dirs' : [
        '../src/core',
        '../src/effects', #needed for BlurMask.h
        '../src/images',
        '../src/lazy',
        '../gm',       # needed to pull gm.h
        '../samplecode', # To pull SampleApp.h and SampleCode.h
        '../src/pipe/utils', # For TiledPipeController
        '../src/utils/debugger',
      ],
      'includes': [
        'gmslides.gypi',
      ],
      'sources': [
        '../src/utils/debugger/SkDrawCommand.h',
        '../src/utils/debugger/SkDrawCommand.cpp',
        '../src/utils/debugger/SkDebugCanvas.h',
        '../src/utils/debugger/SkDebugCanvas.cpp',
        '../src/utils/debugger/SkObjectParser.h',
        '../src/utils/debugger/SkObjectParser.cpp',

        '../gm/gm.cpp',
        '../gm/gm.h',

        '../samplecode/GMSampleView.h',
        '../samplecode/ClockFaceView.cpp',
        '../samplecode/OverView.cpp',
        '../samplecode/OverView.h',
        '../samplecode/Sample2PtRadial.cpp',
        '../samplecode/SampleAAClip.cpp',
        '../samplecode/SampleAARects.cpp',
        '../samplecode/SampleAARectModes.cpp',
        '../samplecode/SampleAll.cpp',
        '../samplecode/SampleAnimator.cpp',
        '../samplecode/SampleAnimBlur.cpp',
        '../samplecode/SampleApp.cpp',
        '../samplecode/SampleArc.cpp',
        '../samplecode/SampleAvoid.cpp',
        '../samplecode/SampleBigBlur.cpp',
        '../samplecode/SampleBigGradient.cpp',
        '../samplecode/SampleBitmapRect.cpp',
        '../samplecode/SampleBlur.cpp',
        '../samplecode/SampleCamera.cpp',
        '../samplecode/SampleChart.cpp',
        '../samplecode/SampleCircle.cpp',
        '../samplecode/SampleClock.cpp',
        '../samplecode/SampleClip.cpp',
        '../samplecode/SampleCode.h',
        '../samplecode/SampleColorFilter.cpp',
        '../samplecode/SampleComplexClip.cpp',
        '../samplecode/SampleConcavePaths.cpp',
        '../samplecode/SampleCull.cpp',
        '../samplecode/SampleDegenerateTwoPtRadials.cpp',
        '../samplecode/SampleDither.cpp',
        '../samplecode/SampleDitherBitmap.cpp',
        '../samplecode/SampleEffects.cpp',
        '../samplecode/SampleEmboss.cpp',
        '../samplecode/SampleEmptyPath.cpp',
        '../samplecode/SampleEncode.cpp',
        '../samplecode/SampleFatBits.cpp',
        '../samplecode/SampleFillType.cpp',
        '../samplecode/SampleFilter.cpp',
        '../samplecode/SampleFilter2.cpp',
        '../samplecode/SampleFilterFuzz.cpp',
        '../samplecode/SampleFontCache.cpp',
        '../samplecode/SampleFontScalerTest.cpp',
        '../samplecode/SampleFuzz.cpp',
        '../samplecode/SampleGradients.cpp',
        '../samplecode/SampleHairCurves.cpp',
        '../samplecode/SampleHairline.cpp',
        '../samplecode/SampleHairModes.cpp',
        '../samplecode/SampleLayerMask.cpp',
        '../samplecode/SampleLayers.cpp',
        '../samplecode/SampleLCD.cpp',
        '../samplecode/SampleLines.cpp',
        '../samplecode/SampleLua.cpp',
        '../samplecode/SampleManyRects.cpp',
        '../samplecode/SampleMeasure.cpp',
        '../samplecode/SampleMipMap.cpp',
        '../samplecode/SampleMovie.cpp',
        '../samplecode/SampleOvalTest.cpp',
        '../samplecode/SampleOverflow.cpp',
        '../samplecode/SamplePatch.cpp',
        '../samplecode/SamplePath.cpp',
        '../samplecode/SamplePathClip.cpp',
        '../samplecode/SamplePathUtils.cpp',
        '../samplecode/SamplePathEffects.cpp',
        '../samplecode/SamplePicture.cpp',
        '../samplecode/SamplePictFile.cpp',
        '../samplecode/SamplePoints.cpp',
        '../samplecode/SamplePolyToPoly.cpp',
        '../samplecode/SampleRegion.cpp',
        '../samplecode/SampleRepeatTile.cpp',
        '../samplecode/SampleRotateCircles.cpp',
        '../samplecode/SampleShaders.cpp',
        '../samplecode/SampleShaderText.cpp',
        '../samplecode/SampleSkLayer.cpp',
        '../samplecode/SampleSlides.cpp',
        '../samplecode/SampleStringArt.cpp',
        '../samplecode/SampleStrokePath.cpp',
        '../samplecode/SampleTests.cpp',
        '../samplecode/SampleText.cpp',
        '../samplecode/SampleTextAlpha.cpp',
        '../samplecode/SampleTextBox.cpp',
        '../samplecode/SampleTextOnPath.cpp',
        '../samplecode/SampleTextureDomain.cpp',
        '../samplecode/SampleTiling.cpp',
        '../samplecode/SampleTinyBitmap.cpp',
        '../samplecode/SampleUnitMapper.cpp',
        '../samplecode/SampleUnpremul.cpp',
        '../samplecode/SampleVertices.cpp',
        '../samplecode/SampleXfermodesBlur.cpp',
        '../samplecode/TransitionView.cpp',
        '../samplecode/TransitionView.h',

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

        # TiledPipeController
        '../src/pipe/utils/SamplePipeControllers.h',
        '../src/pipe/utils/SamplePipeControllers.cpp',

        # Lua
        '../src/utils/SkLuaCanvas.cpp',
        '../src/utils/SkLua.cpp',
      ],
      'sources!': [
        '../samplecode/SampleSkLayer.cpp', #relies on SkMatrix44 which doesn't compile
        '../samplecode/SampleTests.cpp',   #includes unknown file SkShaderExtras.h
        '../samplecode/SampleWarp.cpp',
        '../samplecode/SampleFontCache.cpp',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'views.gyp:views',
        'animator.gyp:animator',
        'xml.gyp:xml',
        'experimental.gyp:experimental',
        'pdf.gyp:pdf',
        'views_animated.gyp:views_animated',
        'lua.gyp:lua',
      ],
     'conditions' : [
       [ 'sample_pdf_file_viewer == 1', {
         'defines': [
           'SAMPLE_PDF_FILE_VIEWER',
         ],
         'dependencies': [
           'pdfviewer_lib.gyp:pdfviewer_lib',
         ],
         'include_dirs' : [
           '../experimental/PdfViewer/inc',
         ],
         'sources': [
           '../samplecode/SamplePdfFileViewer.cpp',
         ]
       }],
        [ 'skia_os == "win"', {
          'sources!': [
            # require UNIX functions
            '../samplecode/SampleEncode.cpp',
          ],
        }],
        [ 'skia_os == "mac"', {
          'sources': [
            # Sample App specific files
            '../src/views/mac/SampleApp-Info.plist',
            '../src/views/mac/SampleAppDelegate.h',
            '../src/views/mac/SampleAppDelegate.mm',
            '../src/views/mac/SkSampleNSView.h',
            '../src/views/mac/SkSampleNSView.mm',

            # Mac files
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
          ],
          'libraries': [
            '$(SDKROOT)/System/Library/Frameworks/QuartzCore.framework',
            '$(SDKROOT)/System/Library/Frameworks/OpenGL.framework',
          ],
          'xcode_settings' : {
            'INFOPLIST_FILE' : '../src/views/mac/SampleApp-Info.plist',
          },
          'mac_bundle_resources' : [
            '../src/views/mac/SampleApp.xib',
          ],
        }],
        [ 'skia_os == "ios"', {
          # TODO: This doesn't build properly yet, but it's getting there.
          'sources!': [
            '../samplecode/SampleDecode.cpp',
          ],
          'sources': [
            '../src/views/mac/SkEventNotifier.h',
            '../src/views/mac/SkEventNotifier.mm',
            '../experimental/iOSSampleApp/SkSampleUIView.h',
            '../experimental/iOSSampleApp/SkSampleUIView.mm',
            '../experimental/iOSSampleApp/SkiOSSampleApp-Base.xcconfig',
            '../experimental/iOSSampleApp/SkiOSSampleApp-Debug.xcconfig',
            '../experimental/iOSSampleApp/SkiOSSampleApp-Release.xcconfig',
            '../experimental/iOSSampleApp/iOSSampleApp-Info.plist',
            '../experimental/iOSSampleApp/Shared/SkOptionListController.h',
            '../experimental/iOSSampleApp/Shared/SkOptionListController.mm',
            '../experimental/iOSSampleApp/Shared/SkUIRootViewController.h',
            '../experimental/iOSSampleApp/Shared/SkUIRootViewController.mm',
            '../experimental/iOSSampleApp/Shared/SkOptionsTableViewController.h',
            '../experimental/iOSSampleApp/Shared/SkOptionsTableViewController.mm',
            '../experimental/iOSSampleApp/Shared/SkUIView.h',
            '../experimental/iOSSampleApp/Shared/SkUIView.mm',
            '../experimental/iOSSampleApp/Shared/SkUIDetailViewController.h',
            '../experimental/iOSSampleApp/Shared/SkUIDetailViewController.mm',
            '../experimental/iOSSampleApp/Shared/skia_ios.mm',

            # iPad
            '../experimental/iOSSampleApp/iPad/AppDelegate_iPad.h',
            '../experimental/iOSSampleApp/iPad/AppDelegate_iPad.mm',
            '../experimental/iOSSampleApp/iPad/SkUISplitViewController.h',
            '../experimental/iOSSampleApp/iPad/SkUISplitViewController.mm',
            '../experimental/iOSSampleApp/iPad/MainWindow_iPad.xib',

            # iPhone
            '../experimental/iOSSampleApp/iPhone/AppDelegate_iPhone.h',
            '../experimental/iOSSampleApp/iPhone/AppDelegate_iPhone.mm',
            '../experimental/iOSSampleApp/iPhone/SkUINavigationController.h',
            '../experimental/iOSSampleApp/iPhone/SkUINavigationController.mm',
            '../experimental/iOSSampleApp/iPhone/MainWindow_iPhone.xib',

            '../src/views/ios/SkOSWindow_iOS.mm',
            '../src/utils/ios/SkImageDecoder_iOS.mm',
            '../src/utils/ios/SkStream_NSData.mm',
            '../src/utils/ios/SkOSFile_iOS.mm',

            '../include/utils/mac/SkCGUtils.h',
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
            '../include/utils/ios',
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
            '../samplecode/SampleUnitMapper.cpp',
          ],
          'dependencies!': [
            'animator.gyp:animator',
            'experimental.gyp:experimental',
          ],
          'dependencies': [
            'android_deps.gyp:Android_SampleApp',
          ],
        }],
        [ 'skia_gpu == 1', {
          'dependencies': [
            'gputest.gyp:skgputest',
          ],
        }],
        [ 'skia_os == "nacl"', {
          'sources': [
            '../../nacl/src/nacl_sample.cpp',
          ],
        }],
      ],
      'msvs_settings': {
        'VCLinkerTool': {
          'SubSystem': '2',
        },
      },
    },
  ],
}
