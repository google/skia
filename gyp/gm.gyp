# GYP file to build the "gm" (golden master) executable.
{
  'includes': [
    'apptype_console.gypi',
    'common.gypi',
  ],
  'targets': [
    {
      'target_name': 'gm',
      'type': 'executable',
      'sources': [
        '../gm/aarectmodes.cpp',
        '../gm/bitmapfilters.cpp',
        '../gm/bitmapscroll.cpp',
        '../gm/blurs.cpp',
        '../gm/complexclip.cpp',
        '../gm/complexclip2.cpp',
        '../gm/emptypath.cpp',
        '../gm/filltypes.cpp',
        '../gm/filltypespersp.cpp',
        '../gm/gmmain.cpp',
        '../gm/gradients.cpp',
        '../gm/hairmodes.cpp',
        '../gm/lcdtext.cpp',
        '../gm/ninepatchstretch.cpp',
        '../gm/nocolorbleed.cpp',
        '../gm/pathfill.cpp',
        '../gm/points.cpp',
        '../gm/poly2poly.cpp',
        '../gm/shadertext.cpp',
        '../gm/shadows.cpp',
        '../gm/shapes.cpp',
        '../gm/strokerects.cpp',
        '../gm/strokes.cpp',
        '../gm/texdata.cpp',
        '../gm/tilemodes.cpp',
        '../gm/tinybitmap.cpp',
        '../gm/xfermodes.cpp',
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
      #mac does not like empty dependency.
      'conditions': [
        [ 'skia_os == "win"', {
          'dependencies': [
            'xps.gyp:xps',
          ],
        }],
      ],  
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
