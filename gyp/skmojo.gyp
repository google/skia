# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'targets': [
    {
      'target_name': 'mojo',
      'type': 'static_library',
      'variables': { 'mojo_parent_dir': '../third_party/externals' },
      'include_dirs': [ '<(mojo_parent_dir)' ],
      'all_dependent_settings': { 'include_dirs': [ '<(mojo_parent_dir)' ] },
      'xcode_settings': { 'OTHER_CFLAGS': [ '-w' ], },
      'sources': [
        '<!@(python find.py <(mojo_parent_dir)/mojo/public/cpp "*.cc")',
        '<(mojo_parent_dir)/mojo/public/platform/native/system_thunks.c',
        '<(mojo_parent_dir)/mojo/public/interfaces/application/application.mojom.cc',
        '<(mojo_parent_dir)/mojo/public/interfaces/application/application.mojom.h',
        '<(mojo_parent_dir)/mojo/public/interfaces/application/service_provider.mojom.cc',
        '<(mojo_parent_dir)/mojo/public/interfaces/application/service_provider.mojom.h',
        '<(mojo_parent_dir)/mojo/public/interfaces/bindings/interface_control_messages.mojom.cc',
        '<(mojo_parent_dir)/mojo/public/interfaces/bindings/interface_control_messages.mojom.h',
        '<(mojo_parent_dir)/mojo/public/interfaces/bindings/tests/ping_service.mojom.cc',
        '<(mojo_parent_dir)/mojo/public/interfaces/bindings/tests/ping_service.mojom.h',
      ],
      'sources!': [
        '<!@(python find.py <(mojo_parent_dir)/mojo/public/cpp "*_unittest.cc")',
        '<!@(python find.py <(mojo_parent_dir)/mojo/public/cpp "*_perftest.cc")',
        '<!@(python find.py <(mojo_parent_dir)/mojo/public/cpp "*_apptest.cc")',
        '<!@(python find.py <(mojo_parent_dir)/mojo/public/cpp "*_test_*.cc")',
        '<!@(python find.py <(mojo_parent_dir)/mojo/public/cpp "*_win.cc")',
      ],
      'actions':[
        {
          'action_name': 'generate_from_mojoms',
          'inputs': [
            '../experimental/mojo/generate.py',
            '<(mojo_parent_dir)/mojo/public/tools/bindings/mojom_parser/bin/linux64/mojom_parser.sha1',
            '<(mojo_parent_dir)/mojo/public/tools/bindings/mojom_parser/bin/mac64/mojom_parser.sha1',
            '<(mojo_parent_dir)/mojo/public/tools/bindings/mojom_bindings_generator.py',
            '<(mojo_parent_dir)/mojo/public/interfaces/bindings/interface_control_messages.mojom',
            '<(mojo_parent_dir)/mojo/public/interfaces/application/service_provider.mojom',
            '<(mojo_parent_dir)/mojo/public/interfaces/bindings/tests/ping_service.mojom',
            '<(mojo_parent_dir)/mojo/public/interfaces/application/application.mojom',
          ],
          'outputs': [
            '<(mojo_parent_dir)/mojo/public/interfaces/application/application.mojom.cc',
            '<(mojo_parent_dir)/mojo/public/interfaces/application/application.mojom.h',
            '<(mojo_parent_dir)/mojo/public/interfaces/application/service_provider.mojom.cc',
            '<(mojo_parent_dir)/mojo/public/interfaces/application/service_provider.mojom.h',
            '<(mojo_parent_dir)/mojo/public/interfaces/bindings/interface_control_messages.mojom.cc',
            '<(mojo_parent_dir)/mojo/public/interfaces/bindings/interface_control_messages.mojom.h',
            '<(mojo_parent_dir)/mojo/public/interfaces/bindings/tests/ping_service.mojom.cc',
            '<(mojo_parent_dir)/mojo/public/interfaces/bindings/tests/ping_service.mojom.h',
          ],
          'action': ['python', '../experimental/mojo/generate.py']
        },
      ],    },
    {
      'target_name': 'skmojo',
      'type': 'static_library',
      'variables': {
        'mojo_dir': '../third_party/externals/mojo/public'
      },
      'dependencies': [ 'mojo' ],
      'defines': [ 'SK_MOJO' ],
      'sources': [ '../experimental/mojo/SkMojo.mojom.cc', ],
      'include_dirs': [ '../experimental/mojo', ],
      'all_dependent_settings': {
        'include_dirs': [ '../experimental/mojo' ],
        'defines': [ 'SK_MOJO' ],
      },
      'actions':[
        {
          'action_name': 'generate_from_mojoms',
          'inputs': [
            '../experimental/mojo/generate.py',
            '../experimental/mojo/SkMojo.mojom',
          ],
          'outputs': [
            '../experimental/mojo/SkMojo.mojom.h',
            '../experimental/mojo/SkMojo.mojom.cc'
          ],
          'action': ['python', '../experimental/mojo/generate.py', '../experimental/mojo/SkMojo.mojom']
        },
      ],
    },
  ],
}
