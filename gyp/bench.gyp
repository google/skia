{
  'includes': [
    'target_defaults.gypi',
  ],
  'targets': [
    {
      'target_name': 'bench',
      'type': 'executable',
      'mac_bundle' : 1,
      'include_dirs': [
        '../include/effects',
        '../include/utils',
        '../include/images',
      ],
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
        'images.gyp:images',
        'utils.gyp:utils',
        'gpu.gyp:gr',
        'gpu.gyp:skgr',
      ],
      'msvs_settings': {
        'VCLinkerTool': {
          'SubSystem': '1',
          'EntryPointSymbol': 'mainCRTStartup',
          'AdditionalDependencies': [
              'OpenGL32.lib',
              'usp10.lib',
          ],
        },
      },
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2: