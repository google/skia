#Animated widgets are views which use animator.

{
  'targets': [
    {
      'target_name': 'views_animated',
      'type': 'static_library',
      'dependencies': [
        'animator.gyp:animator',
        'core.gyp:core',
        'images.gyp:images',
        'ports.gyp:ports',
        'views.gyp:views',
        'xml.gyp:xml',
      ],
      'include_dirs': [
        '../include/views/animated',
      ],
      'sources': [
        '../include/views/animated/SkWidgetViews.h',

        '../src/views/animated/SkBorderView.cpp',
        '../src/views/animated/SkImageView.cpp',
        '../src/views/animated/SkListWidget.cpp',
        '../src/views/animated/SkProgressBarView.cpp',
        '../src/views/animated/SkScrollBarView.cpp',
        '../src/views/animated/SkStaticTextView.cpp',
        '../src/views/animated/SkWidgetViews.cpp',
      ],
      'sources!' : [
        '../src/views/animated/SkListView.cpp',   #depends on missing SkListSource implementation
        '../src/views/animated/SkListWidget.cpp', #depends on missing SkListSource implementation
      ],
      'conditions': [
        [ 'skia_os == "mac"', {
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/Cocoa.framework',
              '$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
            ],
          },
        }],
        [ 'skia_os == "android"', {
          # Android does not support animator so we need to remove all files
          # that have references to it.
          'include_dirs!': [
            '../include/animator',
          ],
          'sources!': [
            '../src/views/animated/SkBorderView.cpp',
            '../src/views/animated/SkImageView.cpp',
            '../src/views/animated/SkListWidget.cpp',
            '../src/views/animated/SkProgressBarView.cpp',
            '../src/views/animated/SkScrollBarView.cpp',
            '../src/views/animated/SkStaticTextView.cpp',
            '../src/views/animated/SkWidgetViews.cpp',
          ],
        }],
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../include/views/animated',
        ],
      },
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
