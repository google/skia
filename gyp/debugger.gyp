{
  'variables': {
    'conditions': [
      [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris"]', {
        # Use the systemwide Qt libs by default
        'variables': {
          'qt_sdk%': '/usr',
        },
        'qt_sdk': '<(qt_sdk)',
        'qt_moc%': 'moc',
        'qt_includes': [
          '<(qt_sdk)/include',
          '<(qt_sdk)/include/QtCore',
          '<(qt_sdk)/include/QtGui',
          '<(qt_sdk)/include/QtOpenGL',

          # Systemwide Qt libs are not contained under a single tree,
          # so we're adding package-qualified paths as a fallback.
          '<(qt_sdk)/include/qt4',
          '<(qt_sdk)/include/qt4/QtCore',
          '<(qt_sdk)/include/qt4/QtGui',
          '<(qt_sdk)/include/qt4/QtOpenGL',
        ],
        'qt_libs': [
          '-lQtCore',
          '-lQtGui',
          '-lQtOpenGL'
        ],
      }],
      [ 'skia_os == "mac"', {
        # Use the systemwide Qt libs by default
        'variables': {
          'qt_sdk%': '/Library/Frameworks',
        },
        'qt_sdk': '<(qt_sdk)',
        'qt_moc%': 'moc',
        'qt_includes': [
          '<(qt_sdk)/QtCore.framework/Headers/',
          '<(qt_sdk)/QtGui.framework/Headers/',
          '<(qt_sdk)/QtOpenGL.framework/Headers/',
        ],
        'qt_libs': [
          '<(qt_sdk)/QtCore.framework',
          '<(qt_sdk)/QtGui.framework',
          '<(qt_sdk)/QtOpenGL.framework',
        ],
      }],
      [ 'skia_os == "win"', {
        'variables': {
          # TODO: location of systemwide Qt on Win?
          'qt_sdk%': 'C:/Qt/4.6.4/',
        },
        'qt_sdk': '<(qt_sdk)',
        'qt_moc%': '<(qt_sdk)/bin/moc',
        'qt_includes': [
          '<(qt_sdk)/include',
          '<(qt_sdk)/include/QtCore',
          '<(qt_sdk)/include/QtGui',
          '<(qt_sdk)/include/QtOpenGL',
        ],
        'qt_libs': [
          '<(qt_sdk)/lib/QtCore4.lib',
          '<(qt_sdk)/lib/QtGui4.lib',
          '<(qt_sdk)/lib/QtOpenGL.lib',
        ],
      }],
    ],
    'moc_src_dir': '../debugger/QT',
  },
  'targets': [
    {
      'target_name': 'debugger',
      'type': 'executable',
      'mac_bundle': 1,
      'mac_framework_dirs': [
        '/Library/Frameworks',
      ],
      'include_dirs' : [
        '../src/core',
        '../debugger',      # To pull SkDebugger.h
        '../debugger/QT',   # For all the QT UI Goodies
        '../src/gpu',       # To pull gl/GrGLUtil.h
        '<@(qt_includes)',
      ],
      'sources': [
        '../debugger/debuggermain.cpp',
        '../debugger/SkDebugCanvas.h',
        '../debugger/SkDebugCanvas.cpp',
        '../debugger/SkDebugger.cpp',
        '../debugger/SkDrawCommand.h',
        '../debugger/SkDrawCommand.cpp',
        '../debugger/SkObjectParser.h',
        '../debugger/SkObjectParser.cpp',
        '../debugger/QT/SkDebuggerGUI.cpp',
        '../debugger/QT/SkDebuggerGUI.h',
        '../debugger/QT/SkCanvasWidget.cpp',
        '../debugger/QT/SkCanvasWidget.h',
        '../debugger/QT/SkInspectorWidget.h',
        '../debugger/QT/SkInspectorWidget.cpp',
        '../debugger/QT/SkListWidget.h',
        '../debugger/QT/SkListWidget.cpp',
        '../debugger/QT/SkSettingsWidget.h',
        '../debugger/QT/SkSettingsWidget.cpp',
        '../debugger/QT/SkGLWidget.h',
        '../debugger/QT/SkGLWidget.cpp',
        '../debugger/QT/SkRasterWidget.h',
        '../debugger/QT/SkRasterWidget.cpp',

        # To update this file edit SkIcons.qrc and rerun rcc to generate cpp
        '../debugger/QT/qrc_SkIcons.cpp',

        # Generated MOC files
        '<(moc_src_dir)/moc_SkCanvasWidget.cpp',
        '<(moc_src_dir)/moc_SkDebuggerGUI.cpp',
        '<(moc_src_dir)/moc_SkInspectorWidget.cpp',
        '<(moc_src_dir)/moc_SkSettingsWidget.cpp',
        '<(moc_src_dir)/moc_SkRasterWidget.cpp',
        '<(moc_src_dir)/moc_SkGLWidget.cpp',
      ],
      'dependencies': [
        'core.gyp:core',
        'images.gyp:images',
        'ports.gyp:ports',
        'effects.gyp:effects',
        'gpu.gyp:gr',
        'gpu.gyp:skgr',
        'debugger_mocs',
      ],
      'link_settings': {
        'libraries': [
          '<@(qt_libs)',
        ],
      },
    },
    {
      'target_name': 'debugger_mocs',
      'type': 'none',
      'sources': [
        '<(moc_src_dir)/SkCanvasWidget.h',
        '<(moc_src_dir)/SkDebuggerGUI.h',
        '<(moc_src_dir)/SkInspectorWidget.h',
        '<(moc_src_dir)/SkSettingsWidget.h',
        '<(moc_src_dir)/SkRasterWidget.h',
        '<(moc_src_dir)/SkGLWidget.h',
      ],
      'rules': [
        {
          'rule_name': 'generate_moc',
          'extension': 'h',
          'outputs': [ '<(RULE_INPUT_DIRNAME)/moc_<(RULE_INPUT_ROOT).cpp' ],
          'action': [ '<(qt_moc)', '<(RULE_INPUT_PATH)', '-o', '<(moc_src_dir)/moc_<(RULE_INPUT_ROOT).cpp' ],
          'message': 'Generating <(RULE_INPUT_ROOT).cpp.',
        },
      ],
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
