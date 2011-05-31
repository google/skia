# Creates a Makefile that is capable of building all executable targets.
{
  'targets': [
    {
      'target_name': 'all',
      'type': 'none',
      'dependencies': [
        'SampleApp.gyp:SampleApp'
      ],
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
