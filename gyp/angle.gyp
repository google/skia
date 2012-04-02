# ANGLE is the Windows-specific translator from OGL ES 2.0 to D3D 9

{
  'targets': [
    {
      'target_name': 'angle',
      'type': 'none',

      'conditions': [
        [ 'skia_angle', {
          'direct_dependent_settings': {
             'libraries': [
               '../../third_party/externals/angle/lib/Debug/libEGL.lib',
               '../../third_party/externals/angle/lib/Debug/libGLESv2.lib',
             ],
             'include_dirs': [
               '../third_party/externals/angle/include'
             ],
          },
        }],
      ]
    }
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
