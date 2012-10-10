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
        'render_pictures',
        'bench_pictures',
        'pinspect',
        'filter',
      ],
    },
    {
      'target_name': 'skdiff',
      'type': 'executable',
      'sources': [
        '../tools/skdiff_main.cpp',
      ],
      'dependencies': [
        'core.gyp:core',
        'effects.gyp:effects',
        'images.gyp:images',
        'ports.gyp:ports',
        'utils.gyp:utils',
      ],
    },
    {
      'target_name': 'skhello',
      'type': 'executable',
      'sources': [
        '../tools/skhello.cpp',
      ],
      'dependencies': [
        'core.gyp:core',
        'effects.gyp:effects',
        'images.gyp:images',
        'ports.gyp:ports',
        'utils.gyp:utils',
      ],
    },
    {
      'target_name': 'skimage',
      'type': 'executable',
      'sources': [
        '../tools/skimage_main.cpp',
      ],
      'dependencies': [
        'core.gyp:core',
        'effects.gyp:effects',
        'images.gyp:images',
        'ports.gyp:ports',
        'utils.gyp:utils',
      ],
    },
    {
      'target_name': 'render_pictures',
      'type': 'executable',
      'sources': [
        '../tools/render_pictures_main.cpp',
      ],
      'include_dirs': [
        '../src/pipe/utils/',
      ],
      'dependencies': [
        'core.gyp:core',
        'ports.gyp:ports',
        'tools.gyp:picture_renderer',
        'tools.gyp:picture_utils',
      ],
    },
    {
      'target_name': 'bench_pictures',
      'type': 'executable',
      'sources': [
        '../bench/SkBenchLogger.h',
        '../bench/SkBenchLogger.cpp',
        '../bench/TimerData.h',
        '../bench/TimerData.cpp',
        '../tools/bench_pictures_main.cpp',
        '../tools/PictureBenchmark.cpp',
      ],
      'include_dirs': [
        '../bench',
      ],
      'dependencies': [
        'core.gyp:core',
        'effects.gyp:effects',
        'ports.gyp:ports',
        'tools.gyp:picture_utils',
        'tools.gyp:picture_renderer',
        'bench.gyp:bench_timer',
      ],
    },
    {
     'target_name': 'picture_renderer',
     'type': 'static_library',
     'sources': [
        '../tools/PictureRenderer.cpp',
        '../src/pipe/utils/SamplePipeControllers.h',
        '../src/pipe/utils/SamplePipeControllers.cpp',
     ],
     'include_dirs': [
       '../src/pipe/utils/',
       '../src/utils/',
     ],
     'dependencies': [
        'core.gyp:core',
        'effects.gyp:effects',
        'images.gyp:images',
        'tools.gyp:picture_utils',
        'utils.gyp:utils',
     ],
     'export_dependent_settings': [
       'images.gyp:images',
     ],
     'conditions': [
       ['skia_gpu == 1', {
         'dependencies': [
           'gpu.gyp:gr',
           'gpu.gyp:skgr',
         ],
         'export_dependent_settings': [
            'gpu.gyp:gr',
         ],
       }],
    ],
    },
    {
      'target_name': 'picture_utils',
      'type': 'static_library',
      'sources': [
        '../tools/picture_utils.cpp',
        '../tools/picture_utils.h',
      ],
      'dependencies': [
        'core.gyp:core',
      ],
    },
    {
      'target_name': 'pinspect',
      'type': 'executable',
      'sources': [
        '../tools/pinspect.cpp',
      ],
      'dependencies': [
        'core.gyp:core',
        'effects.gyp:effects',
        'images.gyp:images',
        'ports.gyp:ports',
        'utils.gyp:utils',
      ],
    },
    {
      'target_name': 'filter',
      'type': 'executable',
      'include_dirs' : [
        '../src/core',
      ],
      'sources': [
        '../tools/filtermain.cpp',
      ],
      'dependencies': [
        'core.gyp:core',
        'effects.gyp:effects',
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
