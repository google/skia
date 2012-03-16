# GYP file to build various tools.
#
# To build on Linux:
#  ./gyp_skia tools.gyp && make tools
#
# Building on other platforms not tested yet.
#
{
  'includes': [
    'apptype_console.gypi',
  ],
  'targets': [
    {
      # Build all executable targets defined below.
      'target_name': 'tools',
      'type': 'none',
      'dependencies': [
        'skdiff',
        'skhello',
        'skimage',
      ],
    },
    {
      'target_name': 'skdiff',
      'type': 'executable',
      'sources': [
        '../src/effects/SkEffects_none.cpp',
        '../tools/skdiff_main.cpp',
      ],
      'dependencies': [
        'core.gyp:core',
        'images.gyp:images',
        'ports.gyp:ports',
        'utils.gyp:utils',
      ],
    },
    {
      'target_name': 'skhello',
      'type': 'executable',
      'sources': [
        '../src/effects/SkEffects_none.cpp',
        '../tools/skhello.cpp',
      ],
      'dependencies': [
        'core.gyp:core',
        'images.gyp:images',
        'ports.gyp:ports',
        'utils.gyp:utils',
      ],
    },
    {
      'target_name': 'skimage',
      'type': 'executable',
      'sources': [
        '../src/effects/SkEffects_none.cpp',
        '../tools/skimage_main.cpp',
      ],
      'dependencies': [
        'core.gyp:core',
        'images.gyp:images',
        'ports.gyp:ports',
        'utils.gyp:utils',
      ],
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
