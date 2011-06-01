{
  'includes': [
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
      'msvs_settings': {
        'VCLinkerTool': {
          #Allows for creation / output to console.
          #Console (/SUBSYSTEM:CONSOLE)
          'SubSystem': '1',
          
          #Console app, use main/wmain
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