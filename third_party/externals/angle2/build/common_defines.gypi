# Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
    'variables':
    {
        'component%': 'static_library',
        'windows_sdk_path%': 'C:/Program Files (x86)/Windows Kits/8.1',
        # build/gyp_angle sets this to 1.
        'angle_standalone%': '0',

        'angle_build_winrt%': '0',
        'angle_build_winphone%': '0',
        'conditions':
        [
            ['OS=="linux" and use_x11==1 and chromeos==0', {
                'angle_use_glx%': 1,
            },
            {
                'angle_use_glx%': 0,
            }],
        ],

        # root of the OSX SDK for Chromium builds, the system root for standalone builds
        'SDKROOT%': "",
    },
    'msvs_disabled_warnings':
    [
        4100, # Unreferenced formal parameter. Not interesting.
        4127, # conditional expression is constant. Too noisy to be useful.

        # TODO: 4702 doesn't fire on xtree in VS2015 (CTP6). We can remove C4702 after moving to VS2015.
        4702, # Unreachable code. Should only fire on system header xtree.

        4718, # Recursive call has no side effects. Fires on xtree too.
    ],
    'conditions':
    [
        ['component=="shared_library"',
        {
            'defines': [ 'COMPONENT_BUILD' ],
            'msvs_disabled_warnings':
            [
                4251, # STL objects do not have DLL interface, needed by ShaderVars.h
            ],
        }],
        # Normally the WinRT project should rely on the default SDK header include paths
        # However, the WinPhone projects also need the Windows SDK path for DXProgrammableCapture.h
        ['angle_build_winrt==0 or angle_build_winphone==1',
        {
            'msvs_system_include_dirs':
            [
                '<(windows_sdk_path)/Include/shared',
                '<(windows_sdk_path)/Include/um',
            ],
        }],
        ['angle_standalone==1',
        {
            'defines':
            [
                'ANGLE_STANDALONE_BUILD',
            ],
        }],
    ],
    'msvs_settings':
    {
        'VCCLCompilerTool':
        {
            'PreprocessorDefinitions':
            [
                '_CRT_SECURE_NO_DEPRECATE',
                '_SCL_SECURE_NO_WARNINGS',
                '_HAS_EXCEPTIONS=0',
                'NOMINMAX',
            ],
        },
        'VCLinkerTool':
        {
            'conditions':
            [
                ['angle_build_winrt==0',
                {
                    'AdditionalDependencies':
                    [
                        'kernel32.lib',
                        'gdi32.lib',
                        'winspool.lib',
                        'comdlg32.lib',
                        'advapi32.lib',
                        'shell32.lib',
                        'ole32.lib',
                        'oleaut32.lib',
                        'user32.lib',
                        'uuid.lib',
                        'odbc32.lib',
                        'odbccp32.lib',
                        'delayimp.lib',
                    ],
                }],
                # winrt compilation is dynamic depending on the project
                # type. AdditionalDependencies is automatically configured
                # with the required .libs
                ['angle_build_winrt==1',
                {
                    'AdditionalDependencies':
                    [
                        '%(AdditionalDependencies)',
                        'uuid.lib',
                    ],
                }],
            ],
        },
    },

    # Windows SDK library directories for the configurations
    'configurations':
    {
        'conditions':
        [
            ['angle_build_winrt==0',
            {
                'x86_Base':
                {
                    'abstract': 1,
                    'msvs_settings':
                    {
                        'VCLinkerTool':
                        {
                            'AdditionalLibraryDirectories':
                            [
                                '<(windows_sdk_path)/Lib/winv6.3/um/x86',
                            ],
                        },
                        'VCLibrarianTool':
                        {
                            'AdditionalLibraryDirectories':
                            [
                                '<(windows_sdk_path)/Lib/winv6.3/um/x86',
                            ],
                        },
                    },
                },
                'x64_Base':
                {
                    'abstract': 1,
                    'msvs_settings':
                    {
                        'VCLinkerTool':
                        {
                            'AdditionalLibraryDirectories':
                            [
                                '<(windows_sdk_path)/Lib/winv6.3/um/x64',
                            ],
                        },
                        'VCLibrarianTool':
                        {
                            'AdditionalLibraryDirectories':
                            [
                                '<(windows_sdk_path)/Lib/winv6.3/um/x64',
                            ],
                        },
                    },
                },
            }],
        ],
    },
}
