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
  'conditions' : [
    ['OS == "win"',
      {
        'target_defaults': {
          'msvs_cygwin_shell': 0,
          'msvs_settings': {
            'VCCLCompilerTool': {
              'WarningLevel': '1',
              'WarnAsError': 'false',
              'DebugInformationFormat': '3',
              'AdditionalOptions': '/MP',
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
      },
    ],
    ['OS == "linux"', 
      {
        'target_defaults': {
          'configurations': {
            'Debug': {
              'cflags': ['-g']
            },
            'Release': {
              'cflags': ['-O2']
            },
          },
        },
      },
    ],
    ['OS == "mac"', 
      {
        'target_defaults': {
          'configurations': {
            'Debug': {
              'cflags': ['-g']
            },
            'Release': {
              'cflags': ['-O2']
            },
          },
        },
        'xcode_settings': {
          'SYMROOT': '<(DEPTH)/xcodebuild',
        },
      },
    ],
  ],
}
# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
