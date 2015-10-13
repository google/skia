# Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
    # Everything below this is duplicated in the GN build. If you change
    # anything also change angle/BUILD.gn
    'targets':
    [
        {
            'target_name': 'libEGL',
            'type': '<(angle_gl_library_type)',
            'dependencies':
            [
                'libANGLE',
                'libGLESv2',
            ],
            'includes':
            [
                '../build/common_defines.gypi',
            ],
            'include_dirs':
            [
                '.',
                '../include',
            ],
            'sources':
            [
                '<@(libegl_sources)',
            ],
            'conditions':
            [
                ['angle_build_winrt==1',
                {
                    'msvs_requires_importlibrary' : 'true',
                    'msvs_settings':
                    {
                        'VCLinkerTool':
                        {
                            'EnableCOMDATFolding': '1',
                            'OptimizeReferences': '1',
                        }
                    },
                }],
            ],
        },
    ],
}
