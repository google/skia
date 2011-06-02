# GYP file to build performance testbench.
#
# To build and run on Linux:
#  ./gyp_skia bench.gyp && make
#  out/Debug/bench -repeat 2
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
      'target_name': 'bench',
      'type': 'executable',
      'sources': [
        '../bench/benchmain.cpp',
        
        '../bench/SkBenchmark.h',
        '../bench/SkBenchmark.cpp',
        
        '../bench/BitmapBench.cpp',
        '../bench/DecodeBench.cpp',
        '../bench/FPSBench.cpp',
        '../bench/GradientBench.cpp',
        '../bench/PathBench.cpp',
        '../bench/RectBench.cpp',
        '../bench/RepeatTileBench.cpp',
        '../bench/TextBench.cpp',
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
