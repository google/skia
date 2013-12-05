# Build EVERYTHING provided by Skia.
# (Start with the "most" target, and then add targets that we intentionally
# left out of "most".  See most.gyp for an explanation of which targets are
# left out of "most".)
#
# We used to call this the 'all' target, but in SOME cases that
# conflicted with an automatically-generated 'all' target.
# See https://code.google.com/p/skia/issues/detail?id=932
#
{
  'targets': [
    {
      'target_name': 'everything',
      'type': 'none',
      'dependencies': [
        'most.gyp:most',
      ],
      'conditions': [
        ['skia_os in ("ios", "android", "chromeos") or (skia_os == "mac" and skia_arch_width == 32)', {
          # debugger is not supported on this platform
        }, {
          'dependencies': [
            'debugger.gyp:debugger',
            'pdfviewer.gyp:pdfviewer',
            #'v8.gyp:SkV8Example',
            ],
        }],
        # DM assumes you've got a GPU.
        ['skia_gpu == 1', {
          'dependencies': [ 'dm.gyp:dm' ],
        }],
      ],
    },
  ],
}
