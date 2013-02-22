#Animated widgets are views which use animator.

{
  'targets': [
    {
      'target_name': 'views_animated',
      'type': 'static_library',
      'include_dirs': [
        '../include/animator',
        '../include/config',
        '../include/core',
        '../include/effects',
        '../include/images',
        '../include/lazy',
        '../include/utils',
        '../include/views',
        '../include/views/animated',
        '../include/views/unix',
        '../include/xml',
      ],
      'sources': [
        '../include/views/animated/SkBorderView.h',
        '../include/views/animated/SkImageView.h',
        '../include/views/animated/SkProgressBarView.h',
        '../include/views/animated/SkScrollBarView.h',
        '../include/views/animated/SkWidgetViews.h',

        '../src/views/animated/SkBorderView.cpp',
        '../src/views/animated/SkImageView.cpp',
        '../src/views/animated/SkProgressBarView.cpp',
        '../src/views/animated/SkScrollBarView.cpp',
        '../src/views/animated/SkStaticTextView.cpp',
        '../src/views/animated/SkWidgetViews.cpp',
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
