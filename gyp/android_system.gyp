# This GYP file stores the dependencies necessary to build Skia on the Android
# platform. The OS doesn't provide many stable libraries as part of the
# distribution so we have to build a few of them ourselves.  
#
# NOTE: We tried adding the gyp file to the android/ directory at the root of 
# the Skia repo, but that resulted in the generated makefiles being created
# outside of the out directory. We may be able to move the bulk of this gyp
# to the /android directory and put a simple shim here, but that has yet to be
# tested. 

{
  # Define the location of the required Android sources, allowing for override
  # in GYP_DEFINES.
  #
  # These sources are necessary because they must be built using the Android
  # toolchain and they are not expected to be present on the host OS.
  #
  'variables': {
    'android_repo%': '../../android',
  },
  'android_repo%': '<(android_repo)',

  'includes': [
    'common.gypi',
  ],

  'targets': [
    {
      'target_name': 'ft2',
      'type': 'static_library',
      'sources': [
        '<(android_repo)/third_party/externals/freetype/src/base/ftbbox.c',
        '<(android_repo)/third_party/externals/freetype/src/base/ftbitmap.c',
        '<(android_repo)/third_party/externals/freetype/src/base/ftglyph.c',
        '<(android_repo)/third_party/externals/freetype/src/base/ftlcdfil.c',
        '<(android_repo)/third_party/externals/freetype/src/base/ftstroke.c',
        '<(android_repo)/third_party/externals/freetype/src/base/ftxf86.c',
        '<(android_repo)/third_party/externals/freetype/src/base/ftbase.c',
        '<(android_repo)/third_party/externals/freetype/src/base/ftsystem.c',
        '<(android_repo)/third_party/externals/freetype/src/base/ftinit.c',
        '<(android_repo)/third_party/externals/freetype/src/base/ftgasp.c',
#        '<(android_repo)/third_party/externals/freetype/src/base/ftfstype.c',
        '<(android_repo)/third_party/externals/freetype/src/raster/raster.c',
        '<(android_repo)/third_party/externals/freetype/src/sfnt/sfnt.c',
        '<(android_repo)/third_party/externals/freetype/src/smooth/smooth.c',
        '<(android_repo)/third_party/externals/freetype/src/autofit/autofit.c',
        '<(android_repo)/third_party/externals/freetype/src/truetype/truetype.c',
        '<(android_repo)/third_party/externals/freetype/src/cff/cff.c',
        '<(android_repo)/third_party/externals/freetype/src/psnames/psnames.c',
        '<(android_repo)/third_party/externals/freetype/src/pshinter/pshinter.c',
      ],
      'include_dirs': [
        '<(android_repo)/third_party/externals/freetype/builds',
        '<(android_repo)/third_party/externals/freetype/include',
      ],
      'cflags': [
        '-W',
        '-Wall',
        '-fPIC',
        '-DPIC',
        '-DDARWIN_NO_CARBON',
        '-DFT2_BUILD_LIBRARY',
        '-O2',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(android_repo)/third_party/externals/freetype/include',  # For ft2build.h
        ],
      }
    },
    {
      'target_name': 'expat',
      'type': 'static_library',
      'sources': [
        '<(android_repo)/third_party/externals/expat/lib/xmlparse.c',
        '<(android_repo)/third_party/externals/expat/lib/xmlrole.c',
        '<(android_repo)/third_party/externals/expat/lib/xmltok.c',
      ],
      'include_dirs': [
        '<(android_repo)/third_party/externals/expat',
        '<(android_repo)/third_party/externals/expat/lib',
      ],
      'cflags': [
        '-Wall',
        '-Wmissing-prototypes',
        '-Wstrict-prototypes',
        '-fexceptions',
        '-DHAVE_EXPAT_CONFIG_H',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(android_repo)/third_party/externals/expat/lib',  # For expat.h
        ],
      }
    },
    {
      'target_name': 'gif',
      'type': 'static_library',
      'sources': [
        '<(android_repo)/third_party/externals/gif/dgif_lib.c',
        '<(android_repo)/third_party/externals/gif/gifalloc.c',
        '<(android_repo)/third_party/externals/gif/gif_err.c',
      ],
      'include_dirs': [
        '<(android_repo)/third_party/externals/gif',
      ],
      'cflags': [
        '-Wno-format',
        '-DHAVE_CONFIG_H',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(android_repo)/third_party/externals/gif',
        ],
      }
    },
    {
      'target_name': 'png',
      'type': 'static_library',
      'sources': [
        '<(android_repo)/third_party/externals/png/png.c',
        '<(android_repo)/third_party/externals/png/pngerror.c',
        '<(android_repo)/third_party/externals/png/pnggccrd.c',
        '<(android_repo)/third_party/externals/png/pngget.c',
        '<(android_repo)/third_party/externals/png/pngmem.c',
        '<(android_repo)/third_party/externals/png/pngpread.c',
        '<(android_repo)/third_party/externals/png/pngread.c',
        '<(android_repo)/third_party/externals/png/pngrio.c',
        '<(android_repo)/third_party/externals/png/pngrtran.c',
        '<(android_repo)/third_party/externals/png/pngrutil.c',
        '<(android_repo)/third_party/externals/png/pngset.c',
        '<(android_repo)/third_party/externals/png/pngtrans.c',
        '<(android_repo)/third_party/externals/png/pngvcrd.c',
        '<(android_repo)/third_party/externals/png/pngwio.c',
        '<(android_repo)/third_party/externals/png/pngwrite.c',
        '<(android_repo)/third_party/externals/png/pngwtran.c',
        '<(android_repo)/third_party/externals/png/pngwutil.c',
      ],
      'include_dirs': [
        '<(android_repo)/third_party/externals/png',
      ],
      'cflags': [
        '-fvisibility=hidden',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(android_repo)/third_party/externals/png',
        ],
      }
    },
  ]
}