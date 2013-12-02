# GYP file to build performance testbench.
#
{
  'includes': [
    'apptype_console.gypi',
  ],
  'targets': [
    {
      'target_name': 'bench',
      'type': 'executable',
      'include_dirs' : [
        '../src/core',
        '../src/effects',
        '../src/utils',
      ],
      'includes': [
        'bench.gypi'
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'bench_timer',
        'flags.gyp:flags',
      ],
      'conditions': [
        ['skia_gpu == 1',
          {
            'include_dirs' : [
              '../src/gpu',
            ],
          },
        ],
      ],
    },
    {
      'target_name' : 'bench_timer',
      'type': 'static_library',
      'sources': [
        '../bench/BenchTimer.h',
        '../bench/BenchTimer.cpp',
        '../bench/BenchSysTimer_mach.h',
        '../bench/BenchSysTimer_mach.cpp',
        '../bench/BenchSysTimer_posix.h',
        '../bench/BenchSysTimer_posix.cpp',
        '../bench/BenchSysTimer_windows.h',
        '../bench/BenchSysTimer_windows.cpp',
      ],
        'include_dirs': [
        '../src/core',
        '../src/gpu',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
      ],
      'conditions': [
        [ 'skia_os not in ["mac", "ios"]', {
          'sources!': [
            '../bench/BenchSysTimer_mach.h',
            '../bench/BenchSysTimer_mach.cpp',
          ],
        }],
        [ 'skia_os not in ["linux", "freebsd", "openbsd", "solaris", "android", "chromeos"]', {
          'sources!': [
            '../bench/BenchSysTimer_posix.h',
            '../bench/BenchSysTimer_posix.cpp',
          ],
        }],
        [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris", "chromeos"]', {
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
        ['skia_gpu == 1', {
          'sources': [
            '../bench/BenchGpuTimer_gl.h',
            '../bench/BenchGpuTimer_gl.cpp',
          ],
        }],
      ],
    }
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
