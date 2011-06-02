# GYP file to build the "gm" (golden master) executable.
#
# To build and run on Linux:
#  ./gyp_skia gm.gyp && make
#  out/Debug/gm -r ../gm/base-linux
#
# Building on other platforms not tested yet.
#
{
  'includes': [
    'apptype_console.gypi',
    'target_defaults.gypi',
  ],
  'targets': [
    {
      'target_name': 'gm',
      'type': 'executable',
      'sources': [
        '../gm/bitmapfilters.cpp',
        '../gm/blurs.cpp',
        '../gm/filltypes.cpp',
        '../gm/gradients.cpp',
        '../gm/nocolorbleed.cpp',
        '../gm/pathfill.cpp',
        '../gm/points.cpp',
        '../gm/poly2poly.cpp',
        '../gm/shadows.cpp',
        '../gm/shapes.cpp',
        '../gm/strokerects.cpp',
        '../gm/tilemodes.cpp',
        '../gm/xfermodes.cpp',
        '../gm/shadertext.cpp',
        '../gm/complexclip.cpp',
        '../gm/gmmain.cpp',
      ],
      'dependencies': [
        'core.gyp:core',
        'effects.gyp:effects',
        'gpu.gyp:gr',
        'gpu.gyp:skgr',
        'images.gyp:images',
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
