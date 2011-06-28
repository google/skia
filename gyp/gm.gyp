# GYP file to build the "gm" (golden master) executable.
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
        '../gm/lcdtext.cpp',
        '../gm/nocolorbleed.cpp',
        '../gm/pathfill.cpp',
        '../gm/points.cpp',
        '../gm/poly2poly.cpp',
        '../gm/shadows.cpp',
        '../gm/shapes.cpp',
        '../gm/strokerects.cpp',
        '../gm/strokes.cpp',
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
        'pdf.gyp:pdf',
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
