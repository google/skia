{
  'target_defaults': {
    'conditions': [
      ['skia_os != "win"', {
        'sources/': [ ['exclude', '_win.(h|cpp)$'],
        ],
      }],
      ['skia_os != "mac"', {
        'sources/': [ ['exclude', '_mac.(h|cpp|m|mm)$'],
        ],
      }],
      ['skia_os != "linux"', {
        'sources/': [ ['exclude', '_unix.(h|cpp)$'],
        ],
      }],
      ['skia_os != "ios"', {
        'sources/': [ ['exclude', '_iOS.(h|cpp|m|mm)$'],
        ],
      }],
      ['skia_os != "android"', {
        'sources/': [ ['exclude', '_android.(h|cpp)$'],
        ],
      }],
      [ 'skia_os == "android"', {
        'defines': [
          'GR_ANDROID_BUILD=1',
        ],
      }],
      [ 'skia_os == "mac"', {
        'defines': [
          'GR_MAC_BUILD=1',
        ],
      }],
      [ 'skia_os == "linux"', {
        'defines': [
          'GR_LINUX_BUILD=1',
        ],
      }],
      [ 'skia_os == "ios"', {
        'defines': [
          'GR_IOS_BUILD=1',
        ],
      }],
      [ 'skia_os == "win"', {
        'defines': [
          'GR_WIN32_BUILD=1',
        ],
      }],
      # nullify the targets in this gyp file if skia_gpu is 0
      [ 'skia_gpu == 0', {
        'sources/': [
          ['exclude', '.*'],
        ],
        'defines/': [
          ['exclude', '.*'],
        ],
        'include_dirs/': [
           ['exclude', '.*'],
        ],
        'link_settings': {
          'libraries/': [
            ['exclude', '.*'],
          ],
        },
        'direct_dependent_settings': {
          'defines/': [
            ['exclude', '.*'],
          ],
          'include_dirs/': [
            ['exclude', '.*'],
          ],
        },
      }],
    ],
    'direct_dependent_settings': {
      'conditions': [
        [ 'skia_os == "android"', {
          'defines': [
            'GR_ANDROID_BUILD=1',
          ],
        }],
        [ 'skia_os == "mac"', {
          'defines': [
            'GR_MAC_BUILD=1',
          ],
        }],
        [ 'skia_os == "linux"', {
          'defines': [
            'GR_LINUX_BUILD=1',
          ],
        }],
        [ 'skia_os == "ios"', {
          'defines': [
            'GR_IOS_BUILD=1',
          ],
        }],
        [ 'skia_os == "win"', {
          'defines': [
            'GR_WIN32_BUILD=1',
            'GR_GL_FUNCTION_TYPE=__stdcall',
          ],
        }],
      ],
      'include_dirs': [
        '../include/gpu',
      ],
    },
  },
  'targets': [
    {
      'target_name': 'skgr',
      'type': 'static_library',
      'includes': [
        'gpu.gypi',
      ],
      'include_dirs': [
        '../include/config',
        '../include/core',
        '../src/core',
        '../include/gpu',
        '../src/gpu',
      ],
      'dependencies': [
        'angle.gyp:*',
      ],
      'export_dependent_settings': [
        'angle.gyp:*',
      ],
      'sources': [
        '<@(skgr_sources)',
        '<@(skgr_native_gl_sources)',
        '<@(skgr_angle_gl_sources)',
        '<@(skgr_mesa_gl_sources)',
        '<@(skgr_debug_gl_sources)',
        '<@(skgr_null_gl_sources)',
        'gpu.gypi', # Makes the gypi appear in IDEs (but does not modify the build).
      ],
      'conditions': [
        [ 'not skia_mesa', {
          'sources!': [
            '../src/gpu/gl/mesa/SkMesaGLContext.cpp',
          ],
        }],
        [ 'skia_mesa and skia_os == "mac"', {
          'include_dirs': [
             '$(SDKROOT)/usr/X11/include/',
          ],
        }],
        [ 'not skia_angle', {
          'sources!': [
            '../include/gpu/gl/SkANGLEGLContext.h',
            '../src/gpu/gl/angle/SkANGLEGLContext.cpp',
            '../src/gpu/gl/angle/GrGLCreateANGLEInterface.cpp',
          ],
        }],
      ],
    },
    {
      'target_name': 'gr',
      'type': 'static_library',
      'includes': [
        'gpu.gypi',
      ],
     'include_dirs': [
        '../include/core',
        '../include/config',
        '../include/gpu',
        '../src/core', # SkRasterClip.h
        '../src/gpu'
      ],
      'dependencies': [
        'angle.gyp:*',
      ],
      'export_dependent_settings': [
        'angle.gyp:*',
      ],
      'sources': [
        '<@(gr_sources)',
        '<@(gr_native_gl_sources)',
        '<@(gr_angle_gl_sources)',
        '<@(gr_mesa_gl_sources)',
        '<@(gr_debug_gl_sources)',
        '<@(gr_null_gl_sources)',
        'gpu.gypi', # Makes the gypi appear in IDEs (but does not modify the build).
      ],
      'defines': [
        'GR_IMPLEMENTATION=1',
      ],
      'conditions': [
        [ 'skia_nv_path_rendering', {
          'defines': [
            'GR_GL_USE_NV_PATH_RENDERING=1',
          ],
        }],
        [ 'skia_os == "linux"', {
          'sources!': [
            '../src/gpu/gl/GrGLDefaultInterface_none.cpp',
            '../src/gpu/gl/GrGLCreateNativeInterface_none.cpp',
          ],
          'link_settings': {
            'libraries': [
              '-lGL',
              '-lX11',
            ],
          },
        }],
        [ 'skia_mesa and skia_os == "linux"', {
          'link_settings': {
            'libraries': [
              '-lOSMesa',
            ],
          },
        }],
        [ 'skia_os == "mac"', {
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/OpenGL.framework',
            ],
          },
          'sources!': [
            '../src/gpu/gl/GrGLDefaultInterface_none.cpp',
            '../src/gpu/gl/GrGLCreateNativeInterface_none.cpp',
          ],
        }],
        [ 'skia_mesa and skia_os == "mac"', {
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/usr/X11/lib/libOSMesa.dylib',
            ],
          },
          'include_dirs': [
             '$(SDKROOT)/usr/X11/include/',
          ],
        }],
        [ 'not skia_mesa', {
          'sources!': [
            '../src/gpu/gl/mesa/GrGLCreateMesaInterface.cpp',
          ],
        }],
        [ 'skia_os in ["win", "ios"]', {
          'sources!': [
            '../src/gpu/gl/GrGLDefaultInterface_none.cpp',
            '../src/gpu/gl/GrGLCreateNativeInterface_none.cpp',
          ],
        }],
        [ 'not skia_angle', {
          'sources!': [
            '../include/gpu/gl/SkANGLEGLContext.h',

            '../src/gpu/gl/angle/GrGLCreateANGLEInterface.cpp',
            '../src/gpu/gl/angle/SkANGLEGLContext.cpp',
          ],
        }],
        [ 'skia_os == "android"', {
          'sources!': [
            '../src/gpu/gl/GrGLDefaultInterface_none.cpp',
            '../src/gpu/gl/GrGLCreateNativeInterface_none.cpp',
          ],
          'link_settings': {
            'libraries': [
              '-lGLESv2',
              '-lEGL',
            ],
          },
        }],
      ],
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
