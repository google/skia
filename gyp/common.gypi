# Copyright (C) 2011 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
{
  # Define all variables, allowing for override in GYP_DEFINES.
  #
  # One such variable is 'skia_os', which we use instead of 'OS' throughout
  # our gyp files.  We set it automatically based on 'OS', but allow the
  # user to override it via GYP_DEFINES if they like.
  'variables': {
    'skia_scalar%': 'float',
    'skia_os%': '<(OS)',
  },
  'skia_scalar%': '<(skia_scalar)',
  'skia_os': '<(skia_os)',

  'target_defaults': {

    # Validate the 'skia_os' setting against 'OS', because only certain
    # combinations work.  You should only override 'skia_os' for certain
    # situations, like building for iOS on a Mac.
    'variables': {
      'conditions': [
        ['skia_os != OS and not (skia_os == "ios" and OS == "mac")',
          {'error': '<!(Cannot build with skia_os=<(skia_os) on OS=<(OS))'}],
      ],
    },

    'configurations': {
      'Debug': {
        'defines': [
          'SK_DEBUG',
          'GR_DEBUG=1',
        ],
      },
      'Release': {
        'defines': [
          'SK_RELEASE',
          'GR_RELEASE=1',
        ],
      },
    },

    'conditions' : [

      [ 'skia_scalar == "float"',
        {
          'defines': [
            'SK_SCALAR_IS_FLOAT',
            'SK_CAN_USE_FLOAT',
          ],
        }, { # else, skia_scalar != "float"
          'defines': [
            'SK_SCALAR_IS_FIXED',
            'SK_CAN_USE_FLOAT',  # we can still use floats along the way
          ],
        }
      ],

      ['skia_os == "win"',
        {
          'defines': [
            'SK_BUILD_FOR_WIN32',
            'SK_IGNORE_STDINT_DOT_H',
          ],
          'msvs_cygwin_shell': 0,
          'msvs_settings': {
            'VCCLCompilerTool': {
              'WarningLevel': '1',
              'WarnAsError': 'false',
              'DebugInformationFormat': '3',
              'AdditionalOptions': '/MP',
            },
            'VCLinkerTool': {
              'AdditionalDependencies': [
                'OpenGL32.lib',
                'usp10.lib',
              ],
            },
          },
          'configurations': {
            'Debug': {
              'msvs_settings': {
                'VCCLCompilerTool': {
                  'Optimization': '0',    # 0 = /Od
                  'PreprocessorDefinitions': ['_DEBUG'],
                  'RuntimeLibrary': '3',  # 3 = /MDd (debug DLL)
                },
                'VCLinkerTool': {
                  'GenerateDebugInformation': 'true',
                },
              },
            },
            'Release': {
              'msvs_settings': {
                'VCCLCompilerTool': {
                  'Optimization': '2',    # 2 = /Os
                  'PreprocessorDefinitions': ['NDEBUG'],
                  'RuntimeLibrary': '2',  # 2 = /MD (nondebug DLL)
                },
                'VCLinkerTool': {
                  'GenerateDebugInformation': 'false',
                },
              },
            },
          },
        },
      ],

      ['skia_os in ["linux", "freebsd", "openbsd", "solaris"]',
        {
          'defines': [
            'SK_SAMPLES_FOR_X',
            'SK_BUILD_FOR_UNIX',
          ],
          'configurations': {
            'Debug': {
              'cflags': ['-g']
            },
            'Release': {
              'cflags': ['-O2']
            },
          },
          'cflags': [ '-Wall', '-Wextra', '-Wno-unused' ],
          'include_dirs' : [
            '/usr/include/freetype2',
          ],
        },
      ],

      ['skia_os == "mac"', 
        {
          'defines': [
            'SK_BUILD_FOR_MAC',
          ],
          'configurations': {
            'Debug': {
              'cflags': ['-g']
            },
            'Release': {
              'cflags': ['-O2']
            },
          },
          'xcode_settings': {
            'SYMROOT': '<(DEPTH)/xcodebuild',
          },
        },
      ],

    ], # end 'conditions'
  }, # end 'target_defaults'
}
# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
