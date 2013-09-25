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
      ['skia_os != "linux" and skia_os != "chromeos"', {
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
      ['skia_os != "nacl"', {
        'sources/': [ ['exclude', '_nacl.(h|cpp)$'],
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
      [ 'skia_texture_cache_mb_limit != 0', {
        'defines': [
          'GR_DEFAULT_TEXTURE_CACHE_MB_LIMIT=<(skia_texture_cache_mb_limit)',
        ],
      }],
    ],
    'direct_dependent_settings': {
      'conditions': [
        [ 'skia_os == "win"', {
          'defines': [
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
      'target_name': 'skgpu',
      'product_name': 'skia_skgpu',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        'angle.gyp:*',
        'core.gyp:*',
        'utils.gyp:*',
      ],
      'includes': [
        'gpu.gypi',
      ],
      'include_dirs': [
        '../include/gpu',
        '../src/core',
        '../src/gpu',
      ],
      'export_dependent_settings': [
        'angle.gyp:*',
      ],
      'sources': [
        '<@(skgpu_sources)',
        '<@(skgpu_native_gl_sources)',
        '<@(skgpu_angle_gl_sources)',
        '<@(skgpu_mesa_gl_sources)',
        '<@(skgpu_debug_gl_sources)',
        '<@(skgpu_null_gl_sources)',
        'gpu.gypi', # Makes the gypi appear in IDEs (but does not modify the build).
      ],
      'conditions': [
        [ 'skia_nv_path_rendering', {
          'defines': [
            'GR_GL_USE_NV_PATH_RENDERING=1',
          ],
        }],
        [ 'skia_stroke_path_rendering', {
          'sources': [
            '../experimental/StrokePathRenderer/GrStrokePathRenderer.h',
            '../experimental/StrokePathRenderer/GrStrokePathRenderer.cpp',
          ],
          'defines': [
            'GR_STROKE_PATH_RENDERING=1',
          ],
        }],
        [ 'skia_android_path_rendering', {
          'sources': [
            '../experimental/AndroidPathRenderer/GrAndroidPathRenderer.cpp',
            '../experimental/AndroidPathRenderer/GrAndroidPathRenderer.h',
            '../experimental/AndroidPathRenderer/AndroidPathRenderer.cpp',
            '../experimental/AndroidPathRenderer/AndroidPathRenderer.h',
            '../experimental/AndroidPathRenderer/Vertex.h',
          ],
          'defines': [
            'GR_ANDROID_PATH_RENDERING=1',
          ],
        }],
        [ 'skia_os == "linux" or skia_os == "chromeos"', {
          'sources!': [
            '../src/gpu/gl/GrGLDefaultInterface_none.cpp',
            '../src/gpu/gl/GrGLCreateNativeInterface_none.cpp',
          ],
          'link_settings': {
            'libraries': [
              '-lGL',
              '-lGLU',
              '-lX11',
            ],
          },
        }],
        [ 'skia_os == "nacl"', {
          'link_settings': {
            'libraries': [
              '-lppapi_gles2',
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
        [ 'not skia_mesa', {
          'sources!': [
            '../src/gpu/gl/mesa/SkMesaGLContext.cpp',
            '../src/gpu/gl/mesa/GrGLCreateMesaInterface.cpp',
          ],
        }],
        [ 'skia_mesa and skia_os == "mac"', {
          'link_settings': {
            'libraries': [
              '/opt/X11/lib/libOSMesa.dylib',
            ],
          },
          'include_dirs': [
             '/opt/X11/include/',
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
            '<@(skgpu_angle_gl_sources)',
          ],
          'dependencies!': [
            'angle.gyp:*',
          ],
          'export_dependent_settings!': [
            'angle.gyp:*',
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
