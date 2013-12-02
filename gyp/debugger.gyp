{
  'variables': {
    'conditions': [
      [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris", "chromeos"]', {
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
          # This is the default location for the version of Qt current on 10/11/12
          'qt_sdk%': 'C:/Qt/4.8.3/',
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
          '<(qt_sdk)/lib/QtOpenGL4.lib',
        ],
      }],
    ],
    'moc_src_dir': '../debugger/QT',
    'moc_gen_dir': '<(SHARED_INTERMEDIATE_DIR)/debugger/QT',
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
        '../src/utils/debugger',
        '../debugger',      # To pull SkDebugger.h
        '../src/gpu',       # To pull gl/GrGLUtil.h
        '../src/ports',     # To pull SkFontDescriptor.h
        '../bench',
        '../tools',
      ],
      'sources': [
        '../debugger/SkDebugger.cpp',
        '../src/utils/debugger/SkDebugCanvas.h',
        '../src/utils/debugger/SkDebugCanvas.cpp',
        '../src/utils/debugger/SkDrawCommand.h',
        '../src/utils/debugger/SkDrawCommand.cpp',
        '../src/utils/debugger/SkObjectParser.h',
        '../src/utils/debugger/SkObjectParser.cpp',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'bench.gyp:bench_timer',
        'tools.gyp:picture_renderer',
      ],
      'conditions': [
        [ 'skia_os == "nacl"', {
          'include_dirs': [
            '../src/utils',
          ],
          'sources': [
            '../platform_tools/nacl/src/nacl_debugger.cpp',
          ],
        }, { # skia_os != "nacl"
          'include_dirs': [
            '../debugger/QT',   # For all the QT UI Goodies
            '<@(qt_includes)',
          ],
          'sources': [
            '../debugger/debuggermain.cpp',
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
            '../debugger/QT/SkImageWidget.h',
            '../debugger/QT/SkImageWidget.cpp',

            # To update this file edit SkIcons.qrc and rerun rcc to generate cpp
            '../debugger/QT/qrc_SkIcons.cpp',

            # Generated MOC files
            '<(moc_gen_dir)/moc_SkCanvasWidget.cpp',
            '<(moc_gen_dir)/moc_SkDebuggerGUI.cpp',
            '<(moc_gen_dir)/moc_SkInspectorWidget.cpp',
            '<(moc_gen_dir)/moc_SkSettingsWidget.cpp',
            '<(moc_gen_dir)/moc_SkRasterWidget.cpp',
            '<(moc_gen_dir)/moc_SkImageWidget.cpp',
            '<(moc_gen_dir)/moc_SkGLWidget.cpp',
          ],
          'dependencies': [
            'debugger_qt_mocs',
          ],
          'link_settings': {
            'libraries': [
              '<@(qt_libs)',
            ],
          },
        }],
      ],
    },
  ],
  'conditions': [
    [ 'skia_os != "nacl"', {
      'targets': [
        {
          'target_name': 'debugger_qt_mocs',
          'type': 'none',
          'sources': [
            '<(moc_src_dir)/SkCanvasWidget.h',
            '<(moc_src_dir)/SkDebuggerGUI.h',
            '<(moc_src_dir)/SkInspectorWidget.h',
            '<(moc_src_dir)/SkSettingsWidget.h',
            '<(moc_src_dir)/SkRasterWidget.h',
            '<(moc_src_dir)/SkImageWidget.h',
            '<(moc_src_dir)/SkGLWidget.h',
          ],
          'rules': [
            {
              'rule_name': 'generate_moc',
              'extension': 'h',
              'outputs': [ '<(moc_gen_dir)/moc_<(RULE_INPUT_ROOT).cpp' ],
              'action': [ '<(qt_moc)', '-DSK_SUPPORT_GPU=<(skia_gpu)',
                          '<(RULE_INPUT_PATH)',
                          '-o', '<(moc_gen_dir)/moc_<(RULE_INPUT_ROOT).cpp' ],
              'message': 'Generating <(RULE_INPUT_ROOT).cpp.',
            },
          ],
        },
      ],
    }],
  ],
}
