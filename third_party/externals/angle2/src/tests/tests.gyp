# Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
    'includes':
    [
        'deqp.gypi',
        '../../build/common_defines.gypi',
    ],
    'variables':
    {
        'angle_build_conformance_tests%': '0',
        'angle_build_deqp_tests%': '0',

        'rapidjson_include_dir': 'third_party/rapidjson/include',
        'rapidjson_headers':
        [
            'third_party/rapidjson/include/rapidjson/allocators.h',
            'third_party/rapidjson/include/rapidjson/document.h',
            'third_party/rapidjson/include/rapidjson/encodedstream.h',
            'third_party/rapidjson/include/rapidjson/encodings.h',
            'third_party/rapidjson/include/rapidjson/filereadstream.h',
            'third_party/rapidjson/include/rapidjson/filestream.h',
            'third_party/rapidjson/include/rapidjson/filewritestream.h',
            'third_party/rapidjson/include/rapidjson/memorybuffer.h',
            'third_party/rapidjson/include/rapidjson/memorystream.h',
            'third_party/rapidjson/include/rapidjson/prettywriter.h',
            'third_party/rapidjson/include/rapidjson/rapidjson.h',
            'third_party/rapidjson/include/rapidjson/reader.h',
            'third_party/rapidjson/include/rapidjson/stringbuffer.h',
            'third_party/rapidjson/include/rapidjson/writer.h',
            'third_party/rapidjson/include/rapidjson/error/en.h',
            'third_party/rapidjson/include/rapidjson/error/error.h',
            'third_party/rapidjson/include/rapidjson/internal/dtoa.h',
            'third_party/rapidjson/include/rapidjson/internal/itoa.h',
            'third_party/rapidjson/include/rapidjson/internal/meta.h',
            'third_party/rapidjson/include/rapidjson/internal/pow10.h',
            'third_party/rapidjson/include/rapidjson/internal/stack.h',
            'third_party/rapidjson/include/rapidjson/internal/strfunc.h',
            'third_party/rapidjson/include/rapidjson/msinttypes/inttypes.h',
            'third_party/rapidjson/include/rapidjson/msinttypes/stdint.h',
        ],
    },
    'conditions':
    [
        # GoogleTest doesn't support WinRT
        ['angle_build_winrt==0',
        {
            'targets':
            [
                {
                    'target_name': 'angle_test_support',
                    'type': 'none',
                    'conditions':
                    [
                        ['angle_standalone==1',
                        {
                            'dependencies': [
                                'angle_internal_gmock',
                                'angle_internal_gtest',
                            ],
                        },
                        {
                            'dependencies': [
                                '<(DEPTH)/testing/gmock.gyp:gmock',
                                '<(DEPTH)/testing/gtest.gyp:gtest',
                            ],
                            'all_dependent_settings':
                            {
                                'include_dirs':
                                [
                                    '<(DEPTH)/testing/gmock/include',
                                    '<(DEPTH)/testing/gtest/include',
                                ],
                            },
                        }],
                    ],
                },
            ],
        }],
        ['angle_standalone==1 and angle_build_winrt==0',
        {
            'targets':
            [
                # Hide these targets from Chromium, because it can't
                # find our standalone copy of the gtest/gmock sources.
                {
                    'target_name': 'angle_internal_gtest',
                    'type': 'static_library',
                    'includes': [ '../../build/common_defines.gypi', ],
                    'include_dirs':
                    [
                        'third_party/googletest',
                        'third_party/googletest/include',
                    ],
                    'sources':
                    [
                        'third_party/googletest/src/gtest-all.cc',
                    ],
                    'defines':
                    [
                        '_VARIADIC_MAX=10',
                    ],
                    'all_dependent_settings':
                    {
                        'defines':
                        [
                            '_VARIADIC_MAX=10',
                        ],
                        'include_dirs':
                        [
                            'third_party/googletest',
                            'third_party/googletest/include',
                        ],
                    },
                },

                {
                    'target_name': 'angle_internal_gmock',
                    'type': 'static_library',
                    'includes': [ '../../build/common_defines.gypi', ],
                    'include_dirs':
                    [
                        'third_party/googlemock',
                        'third_party/googlemock/include',
                        'third_party/googletest/include',
                    ],
                    'sources':
                    [
                        'third_party/googlemock/src/gmock-all.cc',
                    ],
                    'defines':
                    [
                        '_VARIADIC_MAX=10',
                    ],
                    'all_dependent_settings':
                    {
                        'defines':
                        [
                            '_VARIADIC_MAX=10',
                        ],
                        'include_dirs':
                        [
                            'third_party/googlemock',
                            'third_party/googlemock/include',
                            'third_party/googletest/include',
                        ],
                    },
                },

                # These same target names exist on the Chromium side,
                # which is forbidden, so we make them conditional on
                # ANGLE's standalone build.
                {
                    'target_name': 'angle_unittests',
                    'type': 'executable',
                    'includes':
                    [
                        '../../build/common_defines.gypi',
                        'angle_unittests.gypi',
                    ],
                    'sources':
                    [
                        'angle_unittests_main.cpp',
                    ],
                    'msvs_settings':
                    {
                        'VCLinkerTool':
                        {
                            'conditions':
                            [
                                ['angle_build_winrt==1',
                                {
                                    'AdditionalDependencies':
                                    [
                                        'runtimeobject.lib',
                                    ],
                                }],
                            ],
                        },
                    },
                    'msvs_disabled_warnings':
                    [
                        4244, # Conversion from 'type1' to 'type2', possible loss of data
                    ],
                },
                {
                    'target_name': 'angle_end2end_tests',
                    'type': 'executable',
                    'includes':
                    [
                        '../../build/common_defines.gypi',
                        'angle_end2end_tests.gypi',
                    ],
                    'sources':
                    [
                        'angle_end2end_tests_main.cpp',
                    ],
                },
                {
                    'target_name': 'angle_perftests',
                    'type': 'executable',
                    'includes':
                    [
                        '../../build/common_defines.gypi',
                        'angle_perftests.gypi',
                    ],
                    'sources':
                    [
                        'angle_perftests_main.cpp',
                    ],
                },
            ],
        }],
        ['OS=="win"',
        {
            'conditions':
            [
                ['angle_build_conformance_tests',
                {
                    'variables':
                    {
                        'gles_conformance_tests_output_dir': '<(SHARED_INTERMEDIATE_DIR)/conformance_tests',
                        'gles_conformance_tests_input_dir': 'third_party/gles_conformance_tests/conform/GTF_ES/glsl/GTF',
                        'gles_conformance_tests_generator_script': 'gles_conformance_tests/generate_gles_conformance_tests.py',
                    },
                    'targets':
                    [
                        {
                            'target_name': 'angle_gles2_conformance_tests',
                            'type': 'executable',
                            'includes': [ '../../build/common_defines.gypi', ],
                            'dependencies':
                            [
                                '<(angle_path)/src/angle.gyp:libGLESv2',
                                '<(angle_path)/src/angle.gyp:libEGL',
                                'third_party/gles_conformance_tests/conform/GTF_ES/glsl/GTF/es_cts.gyp:es_cts_test_data',
                                'third_party/gles_conformance_tests/conform/GTF_ES/glsl/GTF/es_cts.gyp:es2_cts',
                                'angle_test_support',
                            ],
                            'variables':
                            {
                                'gles2_conformance_tests_input_file': '<(gles_conformance_tests_input_dir)/mustpass_es20.run',
                                'gles2_conformance_tests_generated_file': '<(gles_conformance_tests_output_dir)/generated_gles2_conformance_tests.cpp',
                            },
                            'sources':
                            [
                                'gles_conformance_tests/gles_conformance_tests.cpp',
                                'gles_conformance_tests/gles_conformance_tests.h',
                                'gles_conformance_tests/gles_conformance_tests_main.cpp',
                                '<(gles2_conformance_tests_generated_file)',
                            ],
                            'include_dirs':
                            [
                                '<(angle_path)/include',
                                'gles_conformance_tests',
                            ],
                            'defines':
                            [
                                'CONFORMANCE_TESTS_TYPE=CONFORMANCE_TESTS_ES2',
                            ],
                            'msvs_settings':
                            {
                                'VCCLCompilerTool':
                                {
                                    # MSVS has trouble compiling this due to the obj files becoming too large.
                                    'AdditionalOptions': [ '/bigobj' ],
                                },
                            },
                            'actions':
                            [
                                {
                                    'action_name': 'generate_gles2_conformance_tests',
                                    'message': 'Generating ES2 conformance tests...',
                                    'msvs_cygwin_shell': 0,
                                    'inputs':
                                    [
                                        '<(gles_conformance_tests_generator_script)',
                                        '<(gles2_conformance_tests_input_file)',
                                    ],
                                    'outputs':
                                    [
                                        '<(gles2_conformance_tests_generated_file)',
                                    ],
                                    'action':
                                    [
                                        'python',
                                        '<(gles_conformance_tests_generator_script)',
                                        '<(gles2_conformance_tests_input_file)',
                                        '<(gles_conformance_tests_input_dir)',
                                        '<(gles2_conformance_tests_generated_file)',
                                    ],
                                },
                            ],
                        },
                        {
                            'target_name': 'angle_gles3_conformance_tests',
                            'type': 'executable',
                            'includes': [ '../../build/common_defines.gypi', ],
                            'dependencies':
                            [
                                '<(angle_path)/src/angle.gyp:libGLESv2',
                                '<(angle_path)/src/angle.gyp:libEGL',
                                'third_party/gles_conformance_tests/conform/GTF_ES/glsl/GTF/es_cts.gyp:es_cts_test_data',
                                'third_party/gles_conformance_tests/conform/GTF_ES/glsl/GTF/es_cts.gyp:es3_cts',
                                'angle_test_support',
                            ],
                            'variables':
                            {
                                'gles3_conformance_tests_input_file': '<(gles_conformance_tests_input_dir)/mustpass_es30.run',
                                'gles3_conformance_tests_generated_file': '<(gles_conformance_tests_output_dir)/generated_gles3_conformance_tests.cpp',
                            },
                            'sources':
                            [
                                'gles_conformance_tests/gles_conformance_tests.cpp',
                                'gles_conformance_tests/gles_conformance_tests.h',
                                'gles_conformance_tests/gles_conformance_tests_main.cpp',
                                '<(gles3_conformance_tests_generated_file)',
                            ],
                            'include_dirs':
                            [
                                '<(angle_path)/include',
                                'gles_conformance_tests',
                            ],
                            'defines':
                            [
                                'CONFORMANCE_TESTS_TYPE=CONFORMANCE_TESTS_ES3',
                            ],
                            'msvs_settings':
                            {
                                'VCCLCompilerTool':
                                {
                                    # MSVS has trouble compiling this due to the obj files becoming too large.
                                    'AdditionalOptions': [ '/bigobj' ],
                                },
                            },
                            'actions':
                            [
                                {
                                    'action_name': 'generate_gles3_conformance_tests',
                                    'message': 'Generating ES3 conformance tests...',
                                    'msvs_cygwin_shell': 0,
                                    'inputs':
                                    [
                                        '<(gles_conformance_tests_generator_script)',
                                        '<(gles3_conformance_tests_input_file)',
                                    ],
                                    'outputs':
                                    [
                                        '<(gles3_conformance_tests_generated_file)',
                                    ],
                                    'action':
                                    [
                                        'python',
                                        '<(gles_conformance_tests_generator_script)',
                                        '<(gles3_conformance_tests_input_file)',
                                        '<(gles_conformance_tests_input_dir)',
                                        '<(gles3_conformance_tests_generated_file)',
                                    ],
                                },
                            ],
                        },
                    ],
                }],
                ['angle_build_deqp_tests',
                {
                    'targets':
                    [
                        {
                            'target_name': 'angle_deqp_tests',
                            'type': 'executable',
                            'includes': [ '../../build/common_defines.gypi', ],
                            'dependencies':
                            [
                                '<(angle_path)/src/angle.gyp:libGLESv2',
                                '<(angle_path)/src/angle.gyp:libEGL',
                                'third_party/deqp/src/deqp/modules/gles3/gles3.gyp:deqp-gles3',
                                'third_party/deqp/src/deqp/framework/platform/platform.gyp:tcutil-platform',
                                'angle_test_support',
                            ],
                            'include_dirs':
                            [
                                '<(angle_path)/include',
                                'deqp_tests',
                            ],
                            'variables':
                            {
                                'deqp_tests_output_dir': '<(SHARED_INTERMEDIATE_DIR)/deqp_tests',
                                'deqp_tests_input_file': 'deqp_tests/deqp_tests.txt',
                                'deqp_tests_generated_file': '<(deqp_tests_output_dir)/generated_deqp_tests.cpp',
                            },
                            'sources':
                            [
                                'deqp_tests/deqp_test_main.cpp',
                                'deqp_tests/deqp_tests.cpp',
                                'deqp_tests/deqp_tests.h',
                                '<(deqp_tests_generated_file)',
                            ],
                            'actions':
                            [
                                {
                                    'action_name': 'generate_deqp_tests',
                                    'message': 'Generating dEQP tests...',
                                    'msvs_cygwin_shell': 0,
                                    'variables':
                                    {
                                        'deqp_tests_generator_script': 'deqp_tests/generate_deqp_tests.py',
                                    },
                                    'inputs':
                                    [
                                        '<(deqp_tests_generator_script)',
                                        '<(deqp_tests_input_file)',
                                    ],
                                    'outputs':
                                    [
                                        '<(deqp_tests_generated_file)',
                                    ],
                                    'action':
                                    [
                                        'python',
                                        '<(deqp_tests_generator_script)',
                                        '<(deqp_tests_input_file)',
                                        '<(deqp_tests_generated_file)',
                                    ],
                                },
                            ],
                        },
                    ],
                }],
            ],
        }],
    ],
}
