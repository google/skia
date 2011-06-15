# Gyp config to build libpng as needed for Skia.
{
  'targets': [
    {
      'target_name': 'libpng',
      'type': 'static_library',
      'dependencies': [
        'zlib.gyp:zlib',
      ],
      'include_dirs': [
        '../third_party/libpng',
      ],
      'sources': [
        '../third_party/libpng/png.h',
        '../third_party/libpng/pngconf.h',
        '../third_party/libpng/pngdebug.h',
        '../third_party/libpng/pnginfo.h',
        '../third_party/libpng/pnglibconf.h',
        '../third_party/libpng/pngpriv.h',
        '../third_party/libpng/pngstruct.h',

        '../third_party/libpng/png.c',
        '../third_party/libpng/pngerror.c',
        '../third_party/libpng/pngget.c',
        '../third_party/libpng/pngmem.c',
        '../third_party/libpng/pngpread.c',
        '../third_party/libpng/pngread.c',
        '../third_party/libpng/pngrio.c',
        '../third_party/libpng/pngrtran.c',
        '../third_party/libpng/pngrutil.c',
        '../third_party/libpng/pngset.c',
        '../third_party/libpng/pngtrans.c',
        '../third_party/libpng/pngwio.c',
        '../third_party/libpng/pngwrite.c',
        '../third_party/libpng/pngwtran.c',
        '../third_party/libpng/pngwutil.c',
      ],
      'all_dependent_settings': {
        'include_dirs': [
          '../third_party/libpng',
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
