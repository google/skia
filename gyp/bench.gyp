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
        '../src/gpu',
      ],
      'includes': [
        'bench.gypi'
      ],
      'dependencies': [
        'core.gyp:core',
        'effects.gyp:effects',
        'gpu.gyp:gr',
        'gpu.gyp:skgr',
        'images.gyp:images',
        'ports.gyp:ports',
        'utils.gyp:utils',
        'bench_timer',
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
        '../bench/BenchGpuTimer_gl.h',
        '../bench/BenchGpuTimer_gl.cpp',
      ],
        'include_dirs': [
        '../src/core',
        '../src/gpu',
      ],
      'dependencies': [
        'core.gyp:core',
        'gpu.gyp:gr',
      ],
      'conditions': [
        [ 'skia_os != "mac"', {
          'sources!': [
            '../bench/BenchSysTimer_mach.h',
            '../bench/BenchSysTimer_mach.cpp',
          ],
        }],
        [ 'skia_os not in ["linux", "freebsd", "openbsd", "solaris", "android"]', {
          'sources!': [
            '../bench/BenchSysTimer_posix.h',
            '../bench/BenchSysTimer_posix.cpp',
          ],
        }],
        [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris"]', {
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
        [ 'skia_os == "android"', {
          'dependencies!': [
            'android_system.gyp:Android_EntryPoint',
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
