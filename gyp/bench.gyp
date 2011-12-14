# GYP file to build performance testbench.
#
{
  'includes': [
    'apptype_console.gypi',
    'common.gypi',
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
      ],
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
