# Top-level gyp configuration for Skia.
#
# Projects that use Skia should depend on one or more of the targets
# defined here.
#
# More targets are defined within the gyp/ directory, but those are
# not intended for external use and may change without notice.
#
# Full documentation at https://sites.google.com/site/skiadocs/
#
{
  'targets': [
    {
      'target_name': 'alltargets',
      'type': 'none',
      'dependencies': [
        'gyp/everything.gyp:everything',
        'gyp/most.gyp:most',
      ],
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
