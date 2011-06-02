# target_defaults used for executable targets that generate a console app
{
  'target_defaults': {
    'mac_bundle' : 1,
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
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
