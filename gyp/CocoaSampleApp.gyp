
{
  'includes': [
    'common.gypi',
  ],
  'targets': [
    {
      'target_name': 'CocoaSampleApp',
      'type': 'executable',
      'mac_bundle' : 1,
      'include_dirs' : [
        '../src/core', # needed to get SkConcaveToTriangle, maybe this should be moved to include dir?
        '../gm',       # SampleGM.cpp pulls gm.h
        '../include/pipe', # To pull in SkGPipe.h for pipe reader/writer
      ],
      'sources': [
        # gm files needed for SampleGM.cpp
        '../gm/bitmapfilters.cpp',
        '../gm/blurs.cpp',
        '../gm/complexclip.cpp',
        '../gm/filltypes.cpp',
        '../gm/gm.h',
        '../gm/gradients.cpp',
        '../gm/nocolorbleed.cpp',
        '../gm/points.cpp',
        '../gm/poly2poly.cpp',
        '../gm/shadertext.cpp',
        '../gm/shadows.cpp',
        '../gm/shapes.cpp',
        '../gm/tilemodes.cpp',
        '../gm/xfermodes.cpp',

        '../samplecode/ClockFaceView.cpp',
        '../samplecode/OverView.cpp',
        '../samplecode/Sample2PtRadial.cpp',
        '../samplecode/SampleAll.cpp',
        '../samplecode/SampleAnimator.cpp',
        '../samplecode/SampleApp.cpp',
        '../samplecode/SampleArc.cpp',
        '../samplecode/SampleAvoid.cpp',
        '../samplecode/SampleBigBlur.cpp',
        '../samplecode/SampleBigGradient.cpp',
        '../samplecode/SampleBitmapRect.cpp',
        '../samplecode/SampleBlur.cpp',
        '../samplecode/SampleCamera.cpp',
        '../samplecode/SampleCircle.cpp',
        '../samplecode/SampleCode.h',
        '../samplecode/SampleColorFilter.cpp',
        '../samplecode/SampleComplexClip.cpp',
        '../samplecode/SampleConcavePaths.cpp',
        '../samplecode/SampleCull.cpp',
        '../samplecode/SampleDecode.cpp',
        '../samplecode/SampleDegenerateTwoPtRadials.cpp',
        '../samplecode/SampleDither.cpp',
        '../samplecode/SampleDitherBitmap.cpp',
        '../samplecode/SampleDrawBitmap.cpp',
        '../samplecode/SampleDrawLooper.cpp',
        '../samplecode/SampleEffects.cpp',
        '../samplecode/SampleEmboss.cpp',
        '../samplecode/SampleEncode.cpp',
        '../samplecode/SampleExtractAlpha.cpp',
        '../samplecode/SampleFillType.cpp',
        '../samplecode/SampleFilter.cpp',
        '../samplecode/SampleFilter2.cpp',
        '../samplecode/SampleFontCache.cpp',
        '../samplecode/SampleFontScalerTest.cpp',
        '../samplecode/SampleFuzz.cpp',
        '../samplecode/SampleGM.cpp',
        '../samplecode/SampleGradients.cpp',
        '../samplecode/SampleHairline.cpp',
        '../samplecode/SampleImage.cpp',
        '../samplecode/SampleImageDir.cpp',
        '../samplecode/SampleLayerMask.cpp',
        '../samplecode/SampleLayers.cpp',
        '../samplecode/SampleLCD.cpp',
        '../samplecode/SampleLineClipper.cpp',
        '../samplecode/SampleLines.cpp',
        '../samplecode/SampleMeasure.cpp',
        '../samplecode/SampleMipMap.cpp',
        '../samplecode/SampleMovie.cpp',
        '../samplecode/SampleNinePatch.cpp',
        '../samplecode/SampleOvalTest.cpp',
        '../samplecode/SampleOverflow.cpp',
        '../samplecode/SamplePageFlip.cpp',
        '../samplecode/SamplePatch.cpp',
        '../samplecode/SamplePath.cpp',
        '../samplecode/SamplePathClip.cpp',
        '../samplecode/SamplePathEffects.cpp',
        '../samplecode/SamplePicture.cpp',
        '../samplecode/SamplePoints.cpp',
        '../samplecode/SamplePolyToPoly.cpp',
        '../samplecode/SampleAARects.cpp',
        '../samplecode/SampleRegion.cpp',
        '../samplecode/SampleRepeatTile.cpp',
        '../samplecode/SampleShaders.cpp',
        '../samplecode/SampleShaderText.cpp',
        '../samplecode/SampleShapes.cpp',
        '../samplecode/SampleSkLayer.cpp',
        '../samplecode/SampleSlides.cpp',
        '../samplecode/SampleStrokePath.cpp',
        '../samplecode/SampleStrokeText.cpp',
        '../samplecode/SampleTests.cpp',
        '../samplecode/SampleText.cpp',
        '../samplecode/SampleTextAlpha.cpp',
        '../samplecode/SampleTextBox.cpp',
        '../samplecode/SampleTextEffects.cpp',
        '../samplecode/SampleTextOnPath.cpp',
        '../samplecode/SampleTextureDomain.cpp',
        '../samplecode/SampleTiling.cpp',
        '../samplecode/SampleTinyBitmap.cpp',
        '../samplecode/SampleTriangles.cpp',
        '../samplecode/SampleTypeface.cpp',
        '../samplecode/SampleUnitMapper.cpp',
        '../samplecode/SampleVertices.cpp',
        '../samplecode/SampleXfermodes.cpp',
        '../samplecode/SampleXfermodesBlur.cpp',
        '../samplecode/TransitionView.cpp',
        
        # Dependencies for the pipe code in SampleApp
        '../src/pipe/SkGPipeRead.cpp',
        '../src/pipe/SkGPipeWrite.cpp',
        
        # DrawingBoard
        '../experimental/DrawingBoard/SkColorPalette.h',
        '../experimental/DrawingBoard/SkColorPalette.cpp',
        '../experimental/DrawingBoard/SkNetPipeController.h',
        '../experimental/DrawingBoard/SkNetPipeController.cpp',
        '../experimental/DrawingBoard/SampleDrawingClient.cpp',
        '../experimental/DrawingBoard/SampleDrawingServer.cpp',
    
        # Networking
        '../experimental/Networking/SampleNetPipeReader.cpp',
        '../experimental/Networking/SkSockets.cpp',
        '../experimental/Networking/SkSockets.h',
      ],
      'sources!': [
        '../samplecode/SampleSkLayer.cpp', #relies on SkMatrix44 which doesn't compile
        '../samplecode/SampleTests.cpp',   #includes unknown file SkShaderExtras.h
        '../samplecode/SampleWarp.cpp',
        '../samplecode/SampleFontCache.cpp',
      ],
      'dependencies': [
        'core.gyp:core',
        'effects.gyp:effects',
        'images.gyp:images',
        'views.gyp:views',
        'utils.gyp:utils',
        'animator.gyp:animator',
        'xml.gyp:xml',
        'svg.gyp:svg',
        'experimental.gyp:experimental',
        'gpu.gyp:gr',
        'gpu.gyp:skgr',
        'pdf.gyp:pdf',
      ],
      'conditions' : [
       [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris"]', {
         'sources!': [
            '../samplecode/SampleDecode.cpp',
         ],
        }],
        [ 'skia_os == "win"', {
          'sources!': [
            # require UNIX functions
            '../samplecode/SampleEncode.cpp',
            '../samplecode/SamplePageFlip.cpp',
          ],
        }],
        [ 'skia_os == "mac"', {
          'sources!': [
            '../samplecode/SampleDecode.cpp',
          ],
          'sources': [
            '../experimental/CocoaSampleApp/main.m',
            '../experimental/CocoaSampleApp/SampleApp-Info.plist',
            '../experimental/CocoaSampleApp/SampleApp.xib',
            '../experimental/CocoaSampleApp/SampleAppDelegate.h',
            '../experimental/CocoaSampleApp/SampleAppDelegate.mm',
            '../experimental/CocoaSampleApp/SkNSView.h',
            '../experimental/CocoaSampleApp/SkNSView.mm',
            '../experimental/CocoaSampleApp/SkOptionsTableView.h',
            '../experimental/CocoaSampleApp/SkOptionsTableView.mm',
            '../experimental/CocoaSampleApp/SkOSWindow_Mac.h',
            '../experimental/CocoaSampleApp/SkOSWindow_Mac.mm',
            '../experimental/CocoaSampleApp/SkTextFieldCell.h',
            '../experimental/CocoaSampleApp/SkTextFieldCell.m',
            '../experimental/SkEventNotifier.h',
            '../experimental/SkEventNotifier.mm',
          ],
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/Cocoa.framework',
              '$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
              '$(SDKROOT)/System/Library/Frameworks/QuartzCore.framework',
              '$(SDKROOT)/System/Library/Frameworks/OpenGL.framework',
            ],
            'libraries!': [
              '$(SDKROOT)/System/Library/Frameworks/Carbon.framework',
              '$(SDKROOT)/System/Library/Frameworks/AGL.framework',
            ],
          },
          'xcode_settings' : {
            'INFOPLIST_FILE' : '../experimental/CocoaSampleApp/SampleApp-Info.plist',
          },
          'mac_bundle_resources' : [
            '../experimental/CocoaSampleApp/SampleApp.xib',
          ],
        }],
      ],
      'msvs_settings': {
        'VCLinkerTool': {
          'SubSystem': '2',
          'AdditionalDependencies': [
            'd3d9.lib',
          ],
        },
      },
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
