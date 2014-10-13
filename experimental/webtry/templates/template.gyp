{
  'targets': [
    {
      'configurations': {
        'Debug': { },
        'Release': { }
      },
      'cflags!': [
        '-Werror'
      ],
      'target_name': '{{.Hash}}',
      'type': 'executable',
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'flags.gyp:flags',
        'tools.gyp:sk_tool_utils'

      ],
      'include_dirs': [
        '../include/config',
        '../include/core',
        '../tools/flags',
        '../src/core',
      ],
      'conditions': [
        ['skia_os == "mac"', {
                'defines': ['SK_UNSAFE_BUILD_DESKTOP_ONLY=1']
        }]
      ],
      'sources': [
        '../../cache/src/{{.Hash}}.cpp',
        '../experimental/webtry/main.cpp'
      ],
    }
  ]
}
