
{
  'includes': [
    'common.gypi',
  ],
  'targets': [
    {
      'target_name': 'SampleApp',
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
        '../samplecode/SampleAll.cpp',
        '../samplecode/SampleAnimator.cpp',
        '../samplecode/SampleApp.cpp',
        '../samplecode/SampleArc.cpp',
        '../samplecode/SampleAvoid.cpp',
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
        
        # Dependencies for the pipe code in SampleApp
        '../src/pipe/SkGPipeRead.cpp',
        '../src/pipe/SkGPipeWrite.cpp',
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
        }],
        [ 'skia_os == "ios"', {
          # TODO: This doesn't build properly yet, but it's getting there.
          'sources!': [
            '../samplecode/SampleDecode.cpp',
          ],
          'sources': [
            '../experimental/iOSSampleApp/SkIOSNotifier.mm',
            '../experimental/iOSSampleApp/SkTime_iOS.mm',
            '../experimental/iOSSampleApp/SkUIDetailViewController.mm',
            '../experimental/iOSSampleApp/SkUIRootViewController.mm',
            '../experimental/iOSSampleApp/SkUIView_shell.mm',

            '../experimental/iOSSampleApp/iOSSampleApp_Prefix.pch',
            '../experimental/iOSSampleApp/Shared/main.m',
            '../experimental/iOSSampleApp/iPad/AppDelegate_iPad.mm',
            '../experimental/iOSSampleApp/iPad/SkUISplitViewController.mm',
            '../experimental/iOSSampleApp/iPhone/AppDelegate_iPhone.mm',
            '../experimental/iOSSampleApp/iPhone/SkUINavigationController.mm',

            '../src/utils/ios/SkOSWindow_iOS.mm',
            '../src/utils/ios/SkImageDecoder_iOS.mm',
            '../src/utils/ios/SkStream_NSData.mm',
            '../src/utils/ios/SkOSFile_iOS.mm',

            '../src/utils/mac/SkCreateCGImageRef.cpp',
            '../experimental/iOSSampleApp/SkiOSSampleApp-Debug.xcconfig',
            '../experimental/iOSSampleApp/SkiOSSampleApp-Release.xcconfig',
          ],
          'include_dirs' : [
            '../experimental/iOSSampleApp',
            '../experimental/iOSSampleApp/iPad',
            '../experimental/iOSSampleApp/iPhone',
            '../include/utils/ios',
            '../../gpu/include',
          ],
          'xcode_config_file': '../experimental/iOSSampleApp/SkiOSSampleApp-Base.xcconfig',
          'mac_bundle_resources' : [
            '../experimental/iOSSampleApp/iPad/MainWindow_iPad.xib',
            '../experimental/iOSSampleApp/iPhone/MainWindow_iPhone.xib',
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
