# GYP for libpoppler, a PDF rendering library.
#
# !!! WARNING !!! Poppler is GPL software, and should not be used in anything
# except testing code. Or the lawyercats won't be happy.
#
# libpoppler should be statically linked (doesn't have DLL exports),
# but libpopper-cpp can be dynamically linked.

{
  'variables': {
    'skia_warnings_as_errors': 0,
  },
  'targets': [
    {
      'target_name': 'libpoppler-gpl',
      'type': 'static_library',
      'include_dirs' : [
        '../third_party/externals/poppler',
        '../third_party/externals/poppler/goo',
      ],
      'sources': [
        '../third_party/externals/poppler/fofi/FoFiBase.cc',
        '../third_party/externals/poppler/fofi/FoFiEncodings.cc',
        '../third_party/externals/poppler/fofi/FoFiIdentifier.cc',
        '../third_party/externals/poppler/fofi/FoFiTrueType.cc',
        '../third_party/externals/poppler/fofi/FoFiType1.cc',
        '../third_party/externals/poppler/fofi/FoFiType1C.cc',
        '../third_party/externals/poppler/goo/FixedPoint.cc',
        '../third_party/externals/poppler/goo/gfile.cc',
        '../third_party/externals/poppler/goo/gmem.cc',
        '../third_party/externals/poppler/goo/gmempp.cc',
        '../third_party/externals/poppler/goo/GooHash.cc',
        '../third_party/externals/poppler/goo/GooList.cc',
        '../third_party/externals/poppler/goo/GooString.cc',
        '../third_party/externals/poppler/goo/GooTimer.cc',
        '../third_party/externals/poppler/goo/grandom.cc',
        '../third_party/externals/poppler/goo/gstrtod.cc',
        '../third_party/externals/poppler/goo/ImgWriter.cc',
        '../third_party/externals/poppler/goo/JpegWriter.cc',
        '../third_party/externals/poppler/goo/PNGWriter.cc',
        '../third_party/externals/poppler/goo/TiffWriter.cc',
        '../third_party/externals/poppler/poppler/Annot.cc',
        '../third_party/externals/poppler/poppler/Array.cc',
        '../third_party/externals/poppler/poppler/BuiltinFont.cc',
        '../third_party/externals/poppler/poppler/BuiltinFontTables.cc',
        '../third_party/externals/poppler/poppler/CachedFile.cc',
        '../third_party/externals/poppler/poppler/Catalog.cc',
        '../third_party/externals/poppler/poppler/CharCodeToUnicode.cc',
        '../third_party/externals/poppler/poppler/CMap.cc',
        '../third_party/externals/poppler/poppler/DateInfo.cc',
        '../third_party/externals/poppler/poppler/Decrypt.cc',
        '../third_party/externals/poppler/poppler/Dict.cc',
        '../third_party/externals/poppler/poppler/Error.cc',
        '../third_party/externals/poppler/poppler/FileSpec.cc',
        '../third_party/externals/poppler/poppler/FontEncodingTables.cc',
        '../third_party/externals/poppler/poppler/FontInfo.cc',
        '../third_party/externals/poppler/poppler/Form.cc',
        '../third_party/externals/poppler/poppler/Function.cc',
        '../third_party/externals/poppler/poppler/Gfx.cc',
        '../third_party/externals/poppler/poppler/GfxFont.cc',
        '../third_party/externals/poppler/poppler/GfxState.cc',
        '../third_party/externals/poppler/poppler/GlobalParams.cc',
        '../third_party/externals/poppler/poppler/Hints.cc',
        '../third_party/externals/poppler/poppler/JArithmeticDecoder.cc',
        '../third_party/externals/poppler/poppler/JBIG2Stream.cc',
        '../third_party/externals/poppler/poppler/JPXStream.cc',
        '../third_party/externals/poppler/poppler/Lexer.cc',
        '../third_party/externals/poppler/poppler/Linearization.cc',
        '../third_party/externals/poppler/poppler/Link.cc',
        '../third_party/externals/poppler/poppler/LocalPDFDocBuilder.cc',
        '../third_party/externals/poppler/poppler/Movie.cc',
        '../third_party/externals/poppler/poppler/NameToCharCode.cc',
        '../third_party/externals/poppler/poppler/Object.cc',
        '../third_party/externals/poppler/poppler/OptionalContent.cc',
        '../third_party/externals/poppler/poppler/Outline.cc',
        '../third_party/externals/poppler/poppler/OutputDev.cc',
        '../third_party/externals/poppler/poppler/Page.cc',
        '../third_party/externals/poppler/poppler/PageLabelInfo.cc',
        '../third_party/externals/poppler/poppler/PageTransition.cc',
        '../third_party/externals/poppler/poppler/Parser.cc',
        '../third_party/externals/poppler/poppler/PDFDoc.cc',
        '../third_party/externals/poppler/poppler/PDFDocEncoding.cc',
        '../third_party/externals/poppler/poppler/PDFDocFactory.cc',
        '../third_party/externals/poppler/poppler/PopplerCache.cc',
        '../third_party/externals/poppler/poppler/PreScanOutputDev.cc',
        '../third_party/externals/poppler/poppler/ProfileData.cc',
        '../third_party/externals/poppler/poppler/PSOutputDev.cc',
        '../third_party/externals/poppler/poppler/PSTokenizer.cc',
        '../third_party/externals/poppler/poppler/Rendition.cc',
        '../third_party/externals/poppler/poppler/SecurityHandler.cc',
        '../third_party/externals/poppler/poppler/Sound.cc',
        '../third_party/externals/poppler/poppler/SplashOutputDev.cc',
        '../third_party/externals/poppler/poppler/StdinCachedFile.cc',
        '../third_party/externals/poppler/poppler/StdinPDFDocBuilder.cc',
        '../third_party/externals/poppler/poppler/Stream.cc',
        '../third_party/externals/poppler/poppler/strtok_r.cpp',
        '../third_party/externals/poppler/poppler/TextOutputDev.cc',
        '../third_party/externals/poppler/poppler/UnicodeMap.cc',
        '../third_party/externals/poppler/poppler/UnicodeTypeTable.cc',
        '../third_party/externals/poppler/poppler/UTF.cc',
        '../third_party/externals/poppler/poppler/ViewerPreferences.cc',
        '../third_party/externals/poppler/poppler/XpdfPluginAPI.cc',
        '../third_party/externals/poppler/poppler/XRef.cc',
        '../third_party/externals/poppler/splash/Splash.cc',
        '../third_party/externals/poppler/splash/SplashBitmap.cc',
        '../third_party/externals/poppler/splash/SplashClip.cc',
        '../third_party/externals/poppler/splash/SplashFont.cc',
        '../third_party/externals/poppler/splash/SplashFontEngine.cc',
        '../third_party/externals/poppler/splash/SplashFontFile.cc',
        '../third_party/externals/poppler/splash/SplashFontFileID.cc',
        '../third_party/externals/poppler/splash/SplashFTFont.cc',
        '../third_party/externals/poppler/splash/SplashFTFontEngine.cc',
        '../third_party/externals/poppler/splash/SplashFTFontFile.cc',
        '../third_party/externals/poppler/splash/SplashPath.cc',
        '../third_party/externals/poppler/splash/SplashPattern.cc',
        '../third_party/externals/poppler/splash/SplashScreen.cc',
        '../third_party/externals/poppler/splash/SplashState.cc',
        '../third_party/externals/poppler/splash/SplashT1Font.cc',
        '../third_party/externals/poppler/splash/SplashT1FontEngine.cc',
        '../third_party/externals/poppler/splash/SplashT1FontFile.cc',
        '../third_party/externals/poppler/splash/SplashXPath.cc',
        '../third_party/externals/poppler/splash/SplashXPathScanner.cc',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../third_party/externals/poppler/poppler',
        ],
      },
      'cflags': [
        '-w'
      ],
      'cflags_cc!': [
        '-fno-rtti'
      ],
      'conditions': [
        ['skia_os == "linux"', {
          'include_dirs': [
            '../third_party/poppler/config/linux',
            '/usr/include/freetype2',
          ],
          'cflags': [
            '-fPIC',
          ],
        }],
        ['skia_os in ["mac", "win"]', {
          'dependencies': [
            'freetype.gyp:freetype_poppler',
            'fontconfig.gyp:fontconfig',
          ],
        }],
        ['skia_os == "mac"', {
          'include_dirs': [
            '../third_party/poppler/config/mac',
          ],
        }],
        ['skia_os == "win"', {
          'include_dirs': [
            '../third_party/poppler/config/windows',
          ],
        }],
      ],
    },

    {
      'target_name': 'libpoppler-cpp-gpl',
      'dependencies': [
        'libpoppler-gpl',
      ],
      'type': 'shared_library',
      'include_dirs' : [
        '../third_party/externals/poppler/cpp',
        '../third_party/externals/poppler',
        '../third_party/externals/poppler/poppler',
        '../third_party/externals/poppler/goo',

        '../third_party/poppler/config',
      ],
      'sources': [
        '../third_party/externals/poppler/cpp/PNMWriter.cc',
        '../third_party/externals/poppler/cpp/poppler-document.cpp',
        '../third_party/externals/poppler/cpp/poppler-embedded-file.cpp',
        '../third_party/externals/poppler/cpp/poppler-font.cpp',
        '../third_party/externals/poppler/cpp/poppler-global.cpp',
        '../third_party/externals/poppler/cpp/poppler-image.cpp',
        '../third_party/externals/poppler/cpp/poppler-page.cpp',
        '../third_party/externals/poppler/cpp/poppler-page-renderer.cpp',
        '../third_party/externals/poppler/cpp/poppler-page-transition.cpp',
        '../third_party/externals/poppler/cpp/poppler-private.cpp',
        '../third_party/externals/poppler/cpp/poppler-toc.cpp',
      ],
      'defines': [
        'poppler_cpp_EXPORTS',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../third_party/externals/poppler/cpp',
        ],
      },
      'conditions': [
        ['skia_os == "linux"', {
          'include_dirs': [
            '../third_party/poppler/config/linux',
          ],
          'cflags': [
            '-fPIC',
          ],
        }],
        ['skia_os == "mac"', {
          'include_dirs': [
            '../third_party/poppler/config/mac',
          ],
          'libraries':[
            '$(SDKROOT)/usr/lib/libiconv.dylib',
            '$(SDKROOT)/usr/lib/libexpat.dylib',
          ],
          'xcode_settings': {
            'DYLIB_INSTALL_NAME_BASE': '@executable_path',
            'OTHER_CPLUSPLUSFLAGS!': [
              # poppler doesn't do gcc-style exports
              '-fvisibility=hidden',
            ],
          },
        }],
        ['skia_os == "win"', {
          'dependencies': [
            'iconv.gyp:iconv',
          ],
          'include_dirs': [
            '../third_party/poppler/config/windows',
          ],
        }],
      ],
    },
  ],
}
