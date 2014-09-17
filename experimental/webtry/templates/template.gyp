{
  'targets': [
    {
      'configurations': {
        'Debug': { },
        'Release': { }
      },
      'target_name': '{{.Hash}}',
      'type': 'executable',
      'dependencies': [
        '../skia/gyp/skia_lib.gyp:skia_lib',
        '../skia/gyp/flags.gyp:flags'
      ],
      'include_dirs': [
        '../skia/include/config',
        '../skia/include/core',
        '../skia/tools/flags',
        '../skia/src/core',
      ],
      'conditions': [
        ['skia_os == "mac"', {
                'defines': ['SK_UNSAFE_BUILD_DESKTOP_ONLY=1']
        }]
      ],
      'sources': [
        'src/{{.Hash}}.cpp',
        '../skia/experimental/webtry/main.cpp'
      ],
      'ldflags': [
        '-lskia', '-stdlib=libc++', '-std=c++11'
      ],
      'cflags': [
        '-Werror', '-W', '-Wall', '-Wextra', '-Wno-unused-parameter', '-g', '-O0'
      ]
    }
  ]
}
