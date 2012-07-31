{
  'targets': [
    {
      'target_name': 'debugger',
      'type': 'executable',
      'include_dirs' : [
        '../src/core',
        '../debugger', # To pull SkDebugger.h
        '../debugger/QT', # For all the QT UI Goodies
        '../include/gpu/',
        '../src/gpu', # To pull gl/GrGLUtil.h
      ],
      'sources': [
        '../debugger/SkDebugCanvas.h',
        '../debugger/SkDebugCanvas.cpp',
        '../debugger/SkDebugger.cpp',
        '../debugger/SkDrawCommand.h',
        '../debugger/SkDrawCommand.cpp',
        '../debugger/QT/moc_SkCanvasWidget.cpp',
        '../debugger/QT/moc_SkDebuggerGUI.cpp',
        '../debugger/QT/moc_SkInspectorWidget.cpp',
        '../debugger/QT/moc_SkSettingsWidget.cpp',
        '../debugger/QT/SkDebuggerGUI.cpp',
        '../debugger/QT/SkDebuggerGUI.h',
        '../debugger/QT/SkCanvasWidget.cpp',
        '../debugger/QT/SkCanvasWidget.h',
        '../debugger/QT/SkInspectorWidget.h',
        '../debugger/QT/SkInspectorWidget.cpp',
        '../debugger/QT/SkListWidget.h',
        '../debugger/QT/SkListWidget.cpp',
        '../debugger/SkObjectParser.h',
        '../debugger/SkObjectParser.cpp',
        '../debugger/QT/SkSettingsWidget.h',
        '../debugger/QT/SkSettingsWidget.cpp',
        '../debugger/QT/SkGLWidget.h',
        '../debugger/QT/SkGLWidget.cpp',
        '../debugger/QT/SkRasterWidget.h',
        '../debugger/QT/SkRasterWidget.cpp',

        # To update this file edit SkIcons.qrc and rerun rcc to generate cpp
        '../debugger/QT/qrc_SkIcons.cpp',
      ],
      'dependencies': [
        'core.gyp:core',
        'images.gyp:images',
        'ports.gyp:ports',
        'effects.gyp:effects',
        'gpu.gyp:gr',
        'gpu.gyp:skgr',
      ],
      'conditions': [
        [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris"]', {
          'include_dirs': [
            '/usr/include/qt4',
            '/usr/include/qt4/QtCore',
            '/usr/include/qt4/QtGui',
            '/usr/include/qt4/QtOpenGL',
          ],
          'link_settings': {
            'libraries' : [
              '-lQtCore',
              '-lQtGui',
              '-lQtOpenGL'
            ],
          },
        }],
        [ 'skia_os == "mac" and skia_arch_width != 64', {
          'error': '<!(skia_arch_width must be 64 bits when building the debugger on mac'
        }],
        [ 'skia_os == "mac"', {
          'mac_bundle' : 1,
          'include_dirs': [
            '/Library/Frameworks/QtCore.framework/Headers/',
            '/Library/Frameworks/QtGui.framework/Headers/',
            '/Library/Frameworks/QtOpenGL.framework/Headers/',
          ],
          'link_settings': {
            'libraries': [
              '/Library/Frameworks/QtCore.framework',
              '/Library/Frameworks/QtGui.framework',
              '/Library/Frameworks/QtOpenGL.framework',
            ],
          },
        }],
        [ 'skia_os == "win"', {
          'include_dirs': [
            # TODO(chudy): Dynamically generate these paths?
            'C:/Qt/4.6.4/include',
            'C:/Qt/4.6.4/include/QtCore',
            'C:/Qt/4.6.4/include/QtGui',
            'C:/Qt/4.6.4/include/QtOpenGL',
          ],
          'link_settings': {
            'libraries': [
              'C:/Qt/4.6.4/lib/QtCore4.lib',
              'C:/Qt/4.6.4/lib/QtGui4.lib',
              'C:/Qt/4.6.4/lib/QtOpenGL.lib',
            ],
          },
        }],
      ]
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
