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
        '../bench/BenchTimer.h',
        '../bench/BenchTimer.cpp',
        '../bench/BenchSysTimer_mach.h',
        '../bench/BenchSysTimer_mach.cpp',
        '../bench/BenchSysTimer_posix.h',
        '../bench/BenchSysTimer_posix.cpp',
        '../bench/BenchSysTimer_windows.h',
        '../bench/BenchSysTimer_windows.cpp',
        '../bench/BenchGpuTimer_gl.h',
        '../bench/BenchGpuTimer_gl.cpp',
        '../bench/BenchGpuTimer_none.h',
        '../bench/BenchGpuTimer_none.cpp',
        
        '../bench/SkBenchmark.h',
        '../bench/SkBenchmark.cpp',
        
        '../bench/BitmapBench.cpp',
        '../bench/DecodeBench.cpp',
        '../bench/FPSBench.cpp',
        '../bench/GradientBench.cpp',
        '../bench/MatrixBench.cpp',
        '../bench/PathBench.cpp',
        '../bench/RectBench.cpp',
        '../bench/RepeatTileBench.cpp',
        '../bench/ScalarBench.cpp',
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
      'conditions': [
        [ 'OS != "mac"', {
          'sources!': [
            '../bench/BenchSysTimer_mach.h',
            '../bench/BenchSysTimer_mach.cpp',
          ],
        }],
        [ 'OS not in ["linux", "freebsd", "openbsd", "solaris"]', {
          'sources!': [
            '../bench/BenchSysTimer_posix.h',
            '../bench/BenchSysTimer_posix.cpp',
          ],
        },{
          'link_settings': {
            'libraries': [
              '-lrt',
            ],
          },
        }],
        [ 'OS != "win"', {
          'sources!': [
            '../bench/BenchSysTimer_windows.h',
            '../bench/BenchSysTimer_windows.cpp',
          ],
        }],
        [ 'OS in ["win", "mac", "linux", "freebsd", "openbsd", "solaris"]', {
          'sources!': [
            '../bench/BenchGpuTimer_none.h',
            '../bench/BenchGpuTimer_none.cpp',
          ],
        },{
          'sources!': [
            '../bench/BenchGpuTimer_gl.h',
            '../bench/BenchGpuTimer_gl.cpp',
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
