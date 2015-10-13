# Copyright (c) 2010 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
    'targets':
    [
        {
            'target_name': 'shader_translator',
            'type': 'executable',
            'includes': [ '../build/common_defines.gypi', ],
            'dependencies': [ '../src/angle.gyp:translator_static', ],
            'include_dirs': [ '../include', ],
            'sources': [ 'shader_translator/shader_translator.cpp' ],
        },
        {
            'target_name': 'sample_util',
            'type': 'static_library',
            'includes': [ '../build/common_defines.gypi', ],
            'dependencies':
            [
                '<(angle_path)/src/angle.gyp:libEGL',
                '<(angle_path)/src/angle.gyp:libGLESv2',
                '<(angle_path)/util/util.gyp:angle_util',
            ],
            'export_dependent_settings':
            [
                '<(angle_path)/util/util.gyp:angle_util',
            ],
            'include_dirs':
            [
                '<(angle_path)/include',
                'sample_util',
            ],
            'sources':
            [
                'sample_util/SampleApplication.cpp',
                'sample_util/SampleApplication.h',
                'sample_util/texture_utils.cpp',
                'sample_util/texture_utils.h',
                'sample_util/tga_utils.cpp',
                'sample_util/tga_utils.h',
            ],
            'defines':
            [
                'GL_GLEXT_PROTOTYPES',
                'EGL_EGLEXT_PROTOTYPES',
            ],
            'msvs_disabled_warnings': [ 4201 ],
            'direct_dependent_settings':
            {
                'msvs_disabled_warnings': [ 4201 ],
                'include_dirs':
                [
                    'sample_util',
                ],
                'defines':
                [
                    'GL_GLEXT_PROTOTYPES',
                    'EGL_EGLEXT_PROTOTYPES',
                ],
            },
        },
        {
            'target_name': 'hello_triangle',
            'type': 'executable',
            'dependencies': [ 'sample_util' ],
            'includes': [ '../build/common_defines.gypi', ],
            'sources': [ 'hello_triangle/HelloTriangle.cpp', ],
        },
        {
            'target_name': 'mip_map_2d',
            'type': 'executable',
            'dependencies': [ 'sample_util' ],
            'includes': [ '../build/common_defines.gypi', ],
            'sources': [ 'mip_map_2d/MipMap2D.cpp', ],
        },
        {
            'target_name': 'multi_texture',
            'type': 'executable',
            'dependencies': [ 'sample_util' ],
            'includes': [ '../build/common_defines.gypi', ],
            'sources': [ 'multi_texture/MultiTexture.cpp', ],
            'copies':
            [
                {
                    'destination': '<(PRODUCT_DIR)',
                    'files':
                    [
                        'multi_texture/basemap.tga',
                        'multi_texture/lightmap.tga',
                    ],
                },
            ]
        },

        {
            'target_name': 'multi_window',
            'type': 'executable',
            'dependencies': [ 'sample_util' ],
            'includes': [ '../build/common_defines.gypi', ],
            'sources': [ 'multi_window/MultiWindow.cpp', ],
        },

        {
            'target_name': 'multiple_draw_buffers',
            'type': 'executable',
            'dependencies': [ 'sample_util' ],
            'includes': [ '../build/common_defines.gypi', ],
            'sources': [ 'multiple_draw_buffers/MultipleDrawBuffers.cpp', ],
            'copies':
            [
                {
                    'destination': '<(PRODUCT_DIR)',
                    'files':
                    [
                        'multiple_draw_buffers/multiple_draw_buffers_copy_fs.glsl',
                        'multiple_draw_buffers/multiple_draw_buffers_fs.glsl',
                        'multiple_draw_buffers/multiple_draw_buffers_vs.glsl',
                    ],
                }
            ]
        },

        {
            'target_name': 'particle_system',
            'type': 'executable',
            'dependencies': [ 'sample_util' ],
            'includes': [ '../build/common_defines.gypi', ],
            'sources': [ 'particle_system/ParticleSystem.cpp', ],
            'copies':
            [
                {
                    'destination': '<(PRODUCT_DIR)',
                    'files':
                    [
                        'particle_system/smoke.tga',
                    ],
                }
            ]
        },
        {
            'target_name': 'post_sub_buffer',
            'type': 'executable',
            'dependencies': [ 'sample_util' ],
            'includes': [ '../build/common_defines.gypi', ],
            'sources': [ 'post_sub_buffer/PostSubBuffer.cpp', ],
        },

        {
            'target_name': 'simple_instancing',
            'type': 'executable',
            'dependencies': [ 'sample_util' ],
            'includes': [ '../build/common_defines.gypi', ],
            'sources': [ 'simple_instancing/SimpleInstancing.cpp', ],
        },

        {
            'target_name': 'simple_texture_2d',
            'type': 'executable',
            'dependencies': [ 'sample_util' ],
            'includes': [ '../build/common_defines.gypi', ],
            'sources': [ 'simple_texture_2d/SimpleTexture2D.cpp', ],
        },

        {
            'target_name': 'simple_texture_cubemap',
            'type': 'executable',
            'dependencies': [ 'sample_util' ],
            'includes': [ '../build/common_defines.gypi', ],
            'sources': [ 'simple_texture_cubemap/SimpleTextureCubemap.cpp', ],
        },

        {
            'target_name': 'simple_vertex_shader',
            'type': 'executable',
            'dependencies': [ 'sample_util' ],
            'includes': [ '../build/common_defines.gypi', ],
            'sources': [ 'simple_vertex_shader/SimpleVertexShader.cpp', ],
        },

        {
            'target_name': 'stencil_operations',
            'type': 'executable',
            'dependencies': [ 'sample_util' ],
            'includes': [ '../build/common_defines.gypi', ],
            'sources': [ 'stencil_operations/StencilOperations.cpp', ],
        },

        {
            'target_name': 'tex_redef_microbench',
            'type': 'executable',
            'dependencies': [ 'sample_util' ],
            'includes': [ '../build/common_defines.gypi', ],
            'sources': [ 'tex_redef_microbench/TexRedefMicroBench.cpp', ],
        },

        {
            'target_name': 'texture_wrap',
            'type': 'executable',
            'dependencies': [ 'sample_util' ],
            'includes': [ '../build/common_defines.gypi', ],
            'sources': [ 'texture_wrap/TextureWrap.cpp', ],
        },

        {
            'target_name': 'tri_fan_microbench',
            'type': 'executable',
            'dependencies': [ 'sample_util' ],
            'includes': [ '../build/common_defines.gypi', ],
            'sources': [ 'tri_fan_microbench/TriFanMicroBench.cpp', ],
        },

        {
            'target_name': 'window_test',
            'type': 'executable',
            'dependencies': [ 'sample_util' ],
            'includes': [ '../build/common_defines.gypi', ],
            'sources': [ 'WindowTest/WindowTest.cpp', ],
        },
    ],
}
