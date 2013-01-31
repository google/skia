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
        'bench_pictures',
        'filter',
        'pinspect',
        'render_pdfs',
        'render_pictures',
        'skdiff',
        'skhello',
        'skimage',
      ],
    },
    {
      'target_name': 'skdiff',
      'type': 'executable',
      'sources': [
        '../tools/skdiff.cpp',
        '../tools/skdiff.h',
        '../tools/skdiff_html.cpp',
        '../tools/skdiff_html.h',
        '../tools/skdiff_main.cpp',
        '../tools/skdiff_utils.cpp',
        '../tools/skdiff_utils.h',
      ],
      'dependencies': [
        'skia_base_libs.gyp:skia_base_libs',
        'effects.gyp:effects',
        'images.gyp:images',
      ],
    },
    {
      'target_name': 'skimagediff',
      'type': 'executable',
      'sources': [
        '../tools/skdiff.cpp',
        '../tools/skdiff.h',
        '../tools/skdiff_html.cpp',
        '../tools/skdiff_html.h',
        '../tools/skdiff_image.cpp',
        '../tools/skdiff_utils.cpp',
        '../tools/skdiff_utils.h',
      ],
      'dependencies': [
        'skia_base_libs.gyp:skia_base_libs',
        'effects.gyp:effects',
        'images.gyp:images',
      ],
    },
    {
      'target_name': 'skhello',
      'type': 'executable',
      'sources': [
        '../tools/skhello.cpp',
      ],
      'dependencies': [
        'skia_base_libs.gyp:skia_base_libs',
        'effects.gyp:effects',
        'images.gyp:images',
      ],
    },
    {
      'target_name': 'skimage',
      'type': 'executable',
      'sources': [
        '../tools/skimage_main.cpp',
      ],
      'dependencies': [
        'skia_base_libs.gyp:skia_base_libs',
        'effects.gyp:effects',
        'images.gyp:images',
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
        'skia_base_libs.gyp:skia_base_libs',
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
        'skia_base_libs.gyp:skia_base_libs',
        'effects.gyp:effects',
        'tools.gyp:picture_utils',
        'tools.gyp:picture_renderer',
        'bench.gyp:bench_timer',
      ],
    },
    {
      'target_name': 'picture_renderer',
      'type': 'static_library',
      'sources': [
        '../tools/PictureRenderer.h',
        '../tools/PictureRenderer.cpp',
        '../tools/CopyTilesRenderer.h',
        '../tools/CopyTilesRenderer.cpp',
        '../src/pipe/utils/SamplePipeControllers.h',
        '../src/pipe/utils/SamplePipeControllers.cpp',
      ],
      'include_dirs': [
        '../src/core/',
        '../src/pipe/utils/',
        '../src/utils/',
      ],
      'dependencies': [
        'skia_base_libs.gyp:skia_base_libs',
        'effects.gyp:effects',
        'images.gyp:images',
        'tools.gyp:picture_utils',
      ],
      'export_dependent_settings': [
        'images.gyp:images',
      ],
    },
    {
      'target_name': 'render_pdfs',
      'type': 'executable',
      'sources': [
        '../tools/render_pdfs_main.cpp',
        '../tools/PdfRenderer.cpp',
        '../tools/PdfRenderer.h',
      ],
      'include_dirs': [
        '../src/pipe/utils/',
        '../src/utils/',
      ],
      'dependencies': [
        'core.gyp:core',
        'effects.gyp:effects',
        'images.gyp:images',
        'pdf.gyp:pdf',
        'ports.gyp:ports',
        'tools.gyp:picture_utils',
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
        'skia_base_libs.gyp:skia_base_libs',
      ],
    },
    {
      'target_name': 'pinspect',
      'type': 'executable',
      'sources': [
        '../tools/pinspect.cpp',
      ],
      'dependencies': [
        'skia_base_libs.gyp:skia_base_libs',
        'effects.gyp:effects',
        'images.gyp:images',
      ],
    },
    {
      'target_name': 'filter',
      'type': 'executable',
      'include_dirs' : [
        '../src/core',
        '../debugger',
      ],
      'sources': [
        '../tools/filtermain.cpp',
        '../tools/path_utils.h',
        '../tools/path_utils.cpp',
        '../debugger/SkDrawCommand.h',
        '../debugger/SkDrawCommand.cpp',
        '../debugger/SkDebugCanvas.h',
        '../debugger/SkDebugCanvas.cpp',
        '../debugger/SkObjectParser.h',
        '../debugger/SkObjectParser.cpp',
      ],
      'dependencies': [
        'skia_base_libs.gyp:skia_base_libs',
        'effects.gyp:effects',
        'images.gyp:images',
        'tools.gyp:picture_utils',
      ],
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
