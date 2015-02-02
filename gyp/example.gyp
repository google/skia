# GYP file to build hello world example.
{
  'targets': [
    {
      'target_name': 'HelloWorld',
      'type': 'executable',
      'mac_bundle' : 1,
      'include_dirs' : [
        '../include/gpu',
      ],
      'sources': [
        '../example/HelloWorld.h',
        '../example/HelloWorld.cpp',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'views.gyp:views',
      ],
      'conditions' : [
        [ 'skia_os == "win"', {
          'sources' : [
            '../src/views/win/SkOSWindow_Win.cpp',
            '../src/views/win/skia_win.cpp',
          ],
        }],
        [ 'skia_os == "mac"', {
          'sources': [
            '../example/mac/HelloWorldNSView.mm',
            '../example/mac/HelloWorldDelegate.mm',

            '../src/views/mac/SkEventNotifier.mm',
            '../src/views/mac/skia_mac.mm',
            '../src/views/mac/SkNSView.mm',
            '../src/views/mac/SkOptionsTableView.mm',
            '../src/views/mac/SkOSWindow_Mac.mm',
            '../src/views/mac/SkTextFieldCell.m',
          ],
          'include_dirs' : [
            '../src/views/mac/'
          ],
          'xcode_settings' : {
            'INFOPLIST_FILE' : '../example/mac/HelloWorld-Info.plist',
          },
          'mac_bundle_resources' : [
            '../example/mac/HelloWorld.xib'
          ],
        }],
      ],
    },
  ],
}
