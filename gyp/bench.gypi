# sources and conditions used in skia's bench.gyp and chromium's skia.gyp
#
{
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

    '../bench/SkBenchmark.h',
    '../bench/SkBenchmark.cpp',

    '../bench/BitmapBench.cpp',
    '../bench/BlurBench.cpp',
    '../bench/ChromeBench.cpp',
    '../bench/DecodeBench.cpp',
    '../bench/GradientBench.cpp',
    '../bench/MathBench.cpp',
    '../bench/MatrixBench.cpp',
    '../bench/PathBench.cpp',
    '../bench/RectBench.cpp',
    '../bench/RepeatTileBench.cpp',
    '../bench/ScalarBench.cpp',
    '../bench/TextBench.cpp',
    '../bench/VertBench.cpp',
  ],
  'conditions': [
    [ 'skia_os != "mac"', {
      'sources!': [
        '../bench/BenchSysTimer_mach.h',
        '../bench/BenchSysTimer_mach.cpp',
      ],
    }],
    [ 'skia_os not in ["linux", "freebsd", "openbsd", "solaris"]', {
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
    [ 'skia_os != "win"', {
      'sources!': [
        '../bench/BenchSysTimer_windows.h',
        '../bench/BenchSysTimer_windows.cpp',
      ],
    }],
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
