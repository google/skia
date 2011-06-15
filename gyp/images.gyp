{
  'includes': [
    'target_defaults.gypi',
  ],
  'targets': [
    {
      'target_name': 'images',
      'type': 'static_library',
      'include_dirs': [
        '../include/config',
        '../include/core',
        '../include/images',
      ],
      'sources': [
        '../include/images/SkFlipPixelRef.h',
        '../include/images/SkImageDecoder.h',
        '../include/images/SkImageEncoder.h',
        '../include/images/SkImageRef.h',
        '../include/images/SkImageRef_GlobalPool.h',
        '../include/images/SkJpegUtility.h',
        '../include/images/SkMovie.h',
        '../include/images/SkPageFlipper.h',

        '../src/images/bmpdecoderhelper.cpp',
        '../src/images/bmpdecoderhelper.h',
        '../src/images/SkBitmap_RLEPixels.h',
        '../src/images/SkCreateRLEPixelRef.cpp',
        '../src/images/SkFDStream.cpp',
        '../src/images/SkFlipPixelRef.cpp',
        '../src/images/SkImageDecoder.cpp',
        '../src/images/SkImageDecoder_Factory.cpp',
        '../src/images/SkImageDecoder_libbmp.cpp',
        '../src/images/SkImageDecoder_libgif.cpp',
        '../src/images/SkImageDecoder_libico.cpp',
        '../src/images/SkImageDecoder_libjpeg.cpp',
        '../src/images/SkImageDecoder_libpng.cpp',
        '../src/images/SkImageDecoder_wbmp.cpp',
        '../src/images/SkImageEncoder.cpp',
        '../src/images/SkImageEncoder_Factory.cpp',
        '../src/images/SkImageRef.cpp',
        '../src/images/SkImageRefPool.cpp',
        '../src/images/SkImageRefPool.h',
        '../src/images/SkImageRef_GlobalPool.cpp',
        '../src/images/SkJpegUtility.cpp',
        '../src/images/SkMovie.cpp',
        '../src/images/SkMovie_gif.cpp',
        '../src/images/SkPageFlipper.cpp',
        '../src/images/SkScaledBitmapSampler.cpp',
        '../src/images/SkScaledBitmapSampler.h',
      ],
      'conditions': [
        [ 'OS == "win"', {
          'sources!': [
            '../include/images/SkJpegUtility.h',

            '../src/images/SkFDStream.cpp',
            '../src/images/SkImageDecoder_libgif.cpp',
            '../src/images/SkImageDecoder_libjpeg.cpp',
            '../src/images/SkImageDecoder_libpng.cpp',
            '../src/images/SkImageDecoder_libpvjpeg.c',
            '../src/images/SkJpegUtility.cpp',
            '../src/images/SkMovie_gif.cpp',
          ],
        }],
        [ 'OS == "mac"', {
          'sources!': [
            '../include/images/SkJpegUtility.h',

            '../src/images/SkImageDecoder_libgif.cpp',
            '../src/images/SkImageDecoder_libjpeg.cpp',
            '../src/images/SkImageDecoder_libpng.cpp',
            '../src/images/SkImageDecoder_libpvjpeg.c',
            '../src/images/SkJpegUtility.cpp',
            '../src/images/SkMovie_gif.cpp',
          ],
        }],
        [ 'OS == "linux" or OS == "freebsd" or OS == "openbsd" or OS == "solaris"', {
          'sources!': [
            '../include/images/SkJpegUtility.h',

            '../src/images/SkImageDecoder_libjpeg.cpp',
            '../src/images/SkImageDecoder_libgif.cpp',
            '../src/images/SkImageDecoder_libpvjpeg.c',
            '../src/images/SkJpegUtility.cpp',
            '../src/images/SkMovie_gif.cpp',
          ],
          # libpng stuff:
          # Any targets that depend on this target should link in libpng and
          # our code that calls it.
          # See http://code.google.com/p/gyp/wiki/InputFormatReference#Dependent_Settings
          'dependencies' : [
            'libpng.gyp:libpng',
          ],
          'link_settings': {
            'sources': [
              '../src/images/SkImageDecoder_libpng.cpp',
            ],
          },
          # end libpng stuff
        }],

      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../include/images',
        ],
      },
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
