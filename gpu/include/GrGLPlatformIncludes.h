/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#ifndef GrGLPlatformIncludes_DEFINED
#define GrGLPlatformIncludes_DEFINED

#include "GrConfig.h"

#if !defined(GR_GL_CUSTOM_SETUP)
    #define GR_GL_CUSTOM_SETUP 0
#endif

/**
 * We need to pull in the right GL headers and determine whether we are
 * compiling for ES1, ES2, or desktop GL. (We allow ES1 and ES2 to both be
 * supported in the same build but not ESx and desktop). We also need to know
 * the platform-specific way to get extension function pointers (e.g.
 * eglGetProcAddress). The port specifies this info explicitly or we will infer
 * it from the GR_*_BUILD flag.
 *
 * To specify GL setup directly define GR_GL_CUSTOM_SETUP to 1 and define:
 *      GR_SUPPORT_GLDESKTOP or (GR_SUPPORT_GLES1 and/or GR_SUPPORT_GLES2) to 1
 *
 *      if GR_SUPPORT_GLDESKTOP is 1 then provide:
 *          1. The name of your GL header in GR_INCLUDE_GLDESKTOP
 *          2. If necessary, the name of a file that includes extension
 *             definitions in GR_INCLUDE_GLDESKTOPext.
 *      if GR_SUPPORT_GLES1 is 1 then provide:
 *          1. The name of your GL header in GR_INCLUDE_GLES1
 *          2. If necessary, the name of a file that includes extension
 *             definitions in GR_INCLUDE_GLES1ext.
 *      if GR_SUPPORT_GLES2 is 1 then provide:
 *          1. The name of your GL header in GR_INCLUDE_GLES2
 *          2. If necessary, the name of a file that includes extension
 *             definitions in GR_INCLUDE_GLES2ext.
 *
 *      Optionally, define GR_GL_FUNC to any qualifier needed on GL function
 *      pointer declarations (e.g. __stdcall).
 *
 *      Define GR_GL_PROC_ADDRESS to take a gl function and produce a
 *      function pointer. Two examples:
 *          1. Your platform doesn't require a proc address function, just take
 *             the address of the function:
 *             #define GR_GL_PROC_ADDRESS(X) &X
 *          2. Your platform uses eglGetProcAddress:
 *             #define GR_GL_PROC_ADDRESS eglGetProcAddress(#X)
 *
 *     Optionally define GR_GL_PROC_ADDRESS_HEADER to include any additional
 *     header necessary to use GR_GL_PROC_ADDRESS (e.g. <EGL/egl.h>)
 *
 * Alternatively, define GR_GL_CUSTOM_SETUP_HEADER (and not GR_GL_CUSTOM_SETUP)
 * to a header that can be included. This file should:
 *      1. Define the approprate GR_SUPPORT_GL* macro(s) to 1
 *      2. Includes all necessary GL headers.
 *      3. Optionally define GR_GL_FUNC.
 *      4. Define GR_GL_PROC_ADDRESS.
 *      5. Optionally define GR_GL_PROC_ADDRESS_HEADER
 */

#if GR_GL_CUSTOM_SETUP

    #ifdef GR_SUPPORT_GLES1
        #include GR_INCLUDE_GLES1
        #if defined(GR_INCLUDE_GLES1ext)
            #include GR_INCLUDE_GLES1ext
        #endif
    #endif

    #ifdef GR_SUPPORT_GLES2
        #include GR_INCLUDE_GLES2
        #if defined(GR_INCLUDE_GLES2ext)
            #include GR_INCLUDE_GLES2ext
        #endif
    #endif

    #ifdef GR_SUPPORT_GLDESKTOP
        #include GR_INCLUDE_GLDESKTOP
        #if defined(GR_INCLUDE_GLDESKTOPext)
            #include GR_INCLUDE_GLDESKTOPext
        #endif
    #endif

#elif defined(GR_GL_CUSTOM_SETUP_HEADER)

    #include GR_GL_CUSTOM_SETUP_HEADER

#else
    #undef GR_GL_FUNCTION_TYPE
    #undef GR_GL_PROC_ADDRESS
    #undef GR_GL_PROC_ADDRESS_HEADER

    #if GR_WIN32_BUILD
        #define GR_SUPPORT_GLDESKTOP        1
        #include <Windows.h>
        #include <GL/gl.h>
        // remove stupid windows defines
        #undef near
        #undef far
        #define GR_GL_EMIT_GL_CONSTANTS     1
        #define GR_GL_FUNCTION_TYPE __stdcall
        #define GR_GL_PROC_ADDRESS(X)       wglGetProcAddress(#X)
        #define GR_GL_PROC_ADDRESS_HEADER   <windows.h>

        // Force querying for the existence of these extensions on Windows
        // builds.
        #define GL_APPLE_framebuffer_multisample        1
        #define GL_EXT_framebuffer_object               1
        #define GL_IMG_multisampled_render_to_texture   1
        #define GL_OES_mapbuffer                        1
        #define GL_OES_mapbuffer                        1
    #elif GR_MAC_BUILD
        #define GR_SUPPORT_GLDESKTOP        1
        #include <OpenGL/gl.h>
        #include <OpenGL/glext.h>
        #define GR_GL_PROC_ADDRESS(X)       &X
    #elif GR_IOS_BUILD
        #define GR_SUPPORT_GLES1            1
        #include <OpenGLES/ES1/gl.h>
        #include <OpenGLES/ES1/glext.h>
        #define GR_SUPPORT_GLES2            1
        #include <OpenGLES/ES2/gl.h>
        #include <OpenGLES/ES2/glext.h>
        #define GR_GL_PROC_ADDRESS(X)       &X
    #elif GR_ANDROID_BUILD
        #ifndef GL_GLEXT_PROTOTYPES
            #define GL_GLEXT_PROTOTYPES
        #endif
        #define GR_SUPPORT_GLES2            1
        #include <GLES2/gl2.h>
        #include <GLES2/gl2ext.h>
        #define GR_GL_PROC_ADDRESS(X)       eglGetProcAddress(#X)
        #define GR_GL_PROC_ADDRESS_HEADER   <EGL/egl.h>
    #elif GR_QNX_BUILD
        #ifndef GL_GLEXT_PROTOTYPES
            #define GL_GLEXT_PROTOTYPES
        #endif
         #define GR_SUPPORT_GLES2           1
        // This is needed by the QNX GLES2 headers
        #define GL_API_EXT
        #include <GLES2/gl2.h>
        #include <GLES2/gl2ext.h>
        #define GR_GL_PROC_ADDRESS(X)       eglGetProcAddress(#X)
        #define GR_GL_PROC_ADDRESS_HEADER   <EGL/egl.h>
    #elif GR_LINUX_BUILD
        #ifndef GL_GLEXT_PROTOTYPES
            #define GL_GLEXT_PROTOTYPES
        #endif
        #include <GL/gl.h>
        #include <GL/glext.h>
        #define GR_GL_PROC_ADDRESS(X)       glXGetProcAddress(reinterpret_cast<const GLubyte*>(#X))
        #define GR_SUPPORT_GLDESKTOP        1
        #define GR_GL_PROC_ADDRESS_HEADER   <GL/glx.h>
    #else
        #error "unsupported GR_???_BUILD"
    #endif

#endif

#if !defined(GR_SUPPORT_GLDESKTOP)
    #define GR_SUPPORT_GLDESKTOP    0
#endif
#if !defined(GR_SUPPORT_GLES1)
    #define GR_SUPPORT_GLES1        0
#endif
#if !defined(GR_SUPPORT_GLES2)
    #define GR_SUPPORT_GLES2        0
#endif

#define GR_SUPPORT_GLES ((GR_SUPPORT_GLES1) || (GR_SUPPORT_GLES2))

#if !GR_SUPPORT_GLES && !GR_SUPPORT_GLDESKTOP
    #error "Either desktop or ES GL must be supported"
#elif GR_SUPPORT_GLES && GR_SUPPORT_GLDESKTOP
    #error "Cannot support both desktop and ES GL"
#endif

#if GR_WIN32_BUILD && GR_GL_EMIT_GL_CONSTANTS

// The windows GL headers do not provide the constants used for extensions and
// some versions of GL.  Define them here.

// OpenGL 1.2 Defines
#define GL_SMOOTH_POINT_SIZE_RANGE 0x0B12
#define GL_SMOOTH_POINT_SIZE_GRANULARITY 0x0B13
#define GL_SMOOTH_LINE_WIDTH_RANGE 0x0B22
#define GL_SMOOTH_LINE_WIDTH_GRANULARITY 0x0B23
#define GL_UNSIGNED_BYTE_3_3_2 0x8032
#define GL_UNSIGNED_SHORT_4_4_4_4 0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1 0x8034
#define GL_UNSIGNED_INT_8_8_8_8 0x8035
#define GL_UNSIGNED_INT_10_10_10_2 0x8036
#define GL_RESCALE_NORMAL 0x803A
#define GL_TEXTURE_BINDING_3D 0x806A
#define GL_PACK_SKIP_IMAGES 0x806B
#define GL_PACK_IMAGE_HEIGHT 0x806C
#define GL_UNPACK_SKIP_IMAGES 0x806D
#define GL_UNPACK_IMAGE_HEIGHT 0x806E
#define GL_TEXTURE_3D 0x806F
#define GL_PROXY_TEXTURE_3D 0x8070
#define GL_TEXTURE_DEPTH 0x8071
#define GL_TEXTURE_WRAP_R 0x8072
#define GL_MAX_3D_TEXTURE_SIZE 0x8073
#define GL_BGR 0x80E0
#define GL_BGRA 0x80E1
#define GL_MAX_ELEMENTS_VERTICES 0x80E8
#define GL_MAX_ELEMENTS_INDICES 0x80E9
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_TEXTURE_MIN_LOD 0x813A
#define GL_TEXTURE_MAX_LOD 0x813B
#define GL_TEXTURE_BASE_LEVEL 0x813C
#define GL_TEXTURE_MAX_LEVEL 0x813D
#define GL_LIGHT_MODEL_COLOR_CONTROL 0x81F8
#define GL_SINGLE_COLOR 0x81F9
#define GL_SEPARATE_SPECULAR_COLOR 0x81FA
#define GL_UNSIGNED_BYTE_2_3_3_REV 0x8362
#define GL_UNSIGNED_SHORT_5_6_5 0x8363
#define GL_UNSIGNED_SHORT_5_6_5_REV 0x8364
#define GL_UNSIGNED_SHORT_4_4_4_4_REV 0x8365
#define GL_UNSIGNED_SHORT_1_5_5_5_REV 0x8366
#define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367
#define GL_UNSIGNED_INT_2_10_10_10_REV 0x8368
#define GL_ALIASED_POINT_SIZE_RANGE 0x846D
#define GL_ALIASED_LINE_WIDTH_RANGE 0x846E

// OpenGL 1.3 Multi-Sample Constant Values
#define GL_MULTISAMPLE 0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE 0x809E
#define GL_SAMPLE_ALPHA_TO_ONE 0x809F
#define GL_SAMPLE_COVERAGE 0x80A0
#define GL_SAMPLE_BUFFERS 0x80A8
#define GL_SAMPLES 0x80A9
#define GL_SAMPLE_COVERAGE_VALUE 0x80AA
#define GL_SAMPLE_COVERAGE_INVERT 0x80AB
#define GL_CLAMP_TO_BORDER 0x812D
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1
#define GL_TEXTURE2 0x84C2
#define GL_TEXTURE3 0x84C3
#define GL_TEXTURE4 0x84C4
#define GL_TEXTURE5 0x84C5
#define GL_TEXTURE6 0x84C6
#define GL_TEXTURE7 0x84C7
#define GL_TEXTURE8 0x84C8
#define GL_TEXTURE9 0x84C9
#define GL_TEXTURE10 0x84CA
#define GL_TEXTURE11 0x84CB
#define GL_TEXTURE12 0x84CC
#define GL_TEXTURE13 0x84CD
#define GL_TEXTURE14 0x84CE
#define GL_TEXTURE15 0x84CF
#define GL_TEXTURE16 0x84D0
#define GL_TEXTURE17 0x84D1
#define GL_TEXTURE18 0x84D2
#define GL_TEXTURE19 0x84D3
#define GL_TEXTURE20 0x84D4
#define GL_TEXTURE21 0x84D5
#define GL_TEXTURE22 0x84D6
#define GL_TEXTURE23 0x84D7
#define GL_TEXTURE24 0x84D8
#define GL_TEXTURE25 0x84D9
#define GL_TEXTURE26 0x84DA
#define GL_TEXTURE27 0x84DB
#define GL_TEXTURE28 0x84DC
#define GL_TEXTURE29 0x84DD
#define GL_TEXTURE30 0x84DE
#define GL_TEXTURE31 0x84DF
#define GL_ACTIVE_TEXTURE 0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE 0x84E1
#define GL_MAX_TEXTURE_UNITS 0x84E2
#define GL_TRANSPOSE_MODELVIEW_MATRIX 0x84E3
#define GL_TRANSPOSE_PROJECTION_MATRIX 0x84E4
#define GL_TRANSPOSE_TEXTURE_MATRIX 0x84E5
#define GL_TRANSPOSE_COLOR_MATRIX 0x84E6
#define GL_SUBTRACT 0x84E7
#define GL_COMPRESSED_ALPHA 0x84E9
#define GL_COMPRESSED_LUMINANCE 0x84EA
#define GL_COMPRESSED_LUMINANCE_ALPHA 0x84EB
#define GL_COMPRESSED_INTENSITY 0x84EC
#define GL_COMPRESSED_RGB 0x84ED
#define GL_COMPRESSED_RGBA 0x84EE
#define GL_TEXTURE_COMPRESSION_HINT 0x84EF
#define GL_NORMAL_MAP 0x8511
#define GL_REFLECTION_MAP 0x8512
#define GL_TEXTURE_CUBE_MAP 0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP 0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP 0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE 0x851C
#define GL_COMBINE 0x8570
#define GL_COMBINE_RGB 0x8571
#define GL_COMBINE_ALPHA 0x8572
#define GL_RGB_SCALE 0x8573
#define GL_ADD_SIGNED 0x8574
#define GL_INTERPOLATE 0x8575
#define GL_CONSTANT 0x8576
#define GL_PRIMARY_COLOR 0x8577
#define GL_PREVIOUS 0x8578
#define GL_SOURCE0_RGB 0x8580
#define GL_SOURCE1_RGB 0x8581
#define GL_SOURCE2_RGB 0x8582
#define GL_SOURCE0_ALPHA 0x8588
#define GL_SOURCE1_ALPHA 0x8589
#define GL_SOURCE2_ALPHA 0x858A
#define GL_OPERAND0_RGB 0x8590
#define GL_OPERAND1_RGB 0x8591
#define GL_OPERAND2_RGB 0x8592
#define GL_OPERAND0_ALPHA 0x8598
#define GL_OPERAND1_ALPHA 0x8599
#define GL_OPERAND2_ALPHA 0x859A
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE 0x86A0
#define GL_TEXTURE_COMPRESSED 0x86A1
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS 0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS 0x86A3
#define GL_DOT3_RGB 0x86AE
#define GL_DOT3_RGBA 0x86AF
#define GL_MULTISAMPLE_BIT 0x20000000

// OpenGL 1.4 Defines
#define GL_CONSTANT_COLOR 0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR 0x8002
#define GL_CONSTANT_ALPHA 0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA 0x8004
#define GL_BLEND_DST_RGB 0x80C8
#define GL_BLEND_SRC_RGB 0x80C9
#define GL_BLEND_DST_ALPHA 0x80CA
#define GL_BLEND_SRC_ALPHA 0x80CB
#define GL_POINT_SIZE_MIN 0x8126
#define GL_POINT_SIZE_MAX 0x8127
#define GL_POINT_FADE_THRESHOLD_SIZE 0x8128
#define GL_POINT_DISTANCE_ATTENUATION 0x8129
#define GL_GENERATE_MIPMAP 0x8191
#define GL_GENERATE_MIPMAP_HINT 0x8192
#define GL_DEPTH_COMPONENT16 0x81A5
#define GL_DEPTH_COMPONENT24 0x81A6
#define GL_DEPTH_COMPONENT32 0x81A7
#define GL_MIRRORED_REPEAT 0x8370
#define GL_FOG_COORDINATE_SOURCE 0x8450
#define GL_FOG_COORDINATE 0x8451
#define GL_FRAGMENT_DEPTH 0x8452
#define GL_CURRENT_FOG_COORDINATE 0x8453
#define GL_FOG_COORDINATE_ARRAY_TYPE 0x8454
#define GL_FOG_COORDINATE_ARRAY_STRIDE 0x8455
#define GL_FOG_COORDINATE_ARRAY_POINTER 0x8456
#define GL_FOG_COORDINATE_ARRAY 0x8457
#define GL_COLOR_SUM 0x8458
#define GL_CURRENT_SECONDARY_COLOR 0x8459
#define GL_SECONDARY_COLOR_ARRAY_SIZE 0x845A
#define GL_SECONDARY_COLOR_ARRAY_TYPE 0x845B
#define GL_SECONDARY_COLOR_ARRAY_STRIDE 0x845C
#define GL_SECONDARY_COLOR_ARRAY_POINTER 0x845D
#define GL_SECONDARY_COLOR_ARRAY 0x845E
#define GL_MAX_TEXTURE_LOD_BIAS 0x84FD
#define GL_TEXTURE_FILTER_CONTROL 0x8500
#define GL_TEXTURE_LOD_BIAS 0x8501
#define GL_INCR_WRAP 0x8507
#define GL_DECR_WRAP 0x8508
#define GL_TEXTURE_DEPTH_SIZE 0x884A
#define GL_DEPTH_TEXTURE_MODE 0x884B
#define GL_TEXTURE_COMPARE_MODE 0x884C
#define GL_TEXTURE_COMPARE_FUNC 0x884D
#define GL_COMPARE_R_TO_TEXTURE 0x884E

// OpenGL 1.5 Defines
#define GL_FOG_COORD_SRC GL_FOG_COORDINATE_SOURCE
#define GL_FOG_COORD GL_FOG_COORDINATE
#define GL_FOG_COORD_ARRAY GL_FOG_COORDINATE_ARRAY
#define GL_SRC0_RGB GL_SOURCE0_RGB
#define GL_FOG_COORD_ARRAY_POINTER GL_FOG_COORDINATE_ARRAY_POINTER
#define GL_FOG_COORD_ARRAY_TYPE GL_FOG_COORDINATE_ARRAY_TYPE
#define GL_SRC1_ALPHA GL_SOURCE1_ALPHA
#define GL_CURRENT_FOG_COORD GL_CURRENT_FOG_COORDINATE
#define GL_FOG_COORD_ARRAY_STRIDE GL_FOG_COORDINATE_ARRAY_STRIDE
#define GL_SRC0_ALPHA GL_SOURCE0_ALPHA
#define GL_SRC1_RGB GL_SOURCE1_RGB
#define GL_FOG_COORD_ARRAY_BUFFER_BINDING GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING
#define GL_SRC2_ALPHA GL_SOURCE2_ALPHA
#define GL_SRC2_RGB GL_SOURCE2_RGB
#define GL_BUFFER_SIZE 0x8764
#define GL_BUFFER_USAGE 0x8765
#define GL_QUERY_COUNTER_BITS 0x8864
#define GL_CURRENT_QUERY 0x8865
#define GL_QUERY_RESULT 0x8866
#define GL_QUERY_RESULT_AVAILABLE 0x8867
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_ARRAY_BUFFER_BINDING 0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING 0x8895
#define GL_VERTEX_ARRAY_BUFFER_BINDING 0x8896
#define GL_NORMAL_ARRAY_BUFFER_BINDING 0x8897
#define GL_COLOR_ARRAY_BUFFER_BINDING 0x8898
#define GL_INDEX_ARRAY_BUFFER_BINDING 0x8899
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING 0x889A
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING 0x889B
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING 0x889C
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING 0x889D
#define GL_WEIGHT_ARRAY_BUFFER_BINDING 0x889E
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING 0x889F
#define GL_READ_ONLY 0x88B8
#define GL_WRITE_ONLY 0x88B9
#define GL_READ_WRITE 0x88BA
#define GL_BUFFER_ACCESS 0x88BB
#define GL_BUFFER_MAPPED 0x88BC
#define GL_BUFFER_MAP_POINTER 0x88BD
#define GL_STREAM_DRAW 0x88E0
#define GL_STREAM_READ 0x88E1
#define GL_STREAM_COPY 0x88E2
#define GL_STATIC_DRAW 0x88E4
#define GL_STATIC_READ 0x88E5
#define GL_STATIC_COPY 0x88E6
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_DYNAMIC_READ 0x88E9
#define GL_DYNAMIC_COPY 0x88EA
#define GL_SAMPLES_PASSED 0x8914

// OpenGL 2.0 Defines
#define GL_BLEND_EQUATION_RGB GL_BLEND_EQUATION
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED 0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE 0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE 0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE 0x8625
#define GL_CURRENT_VERTEX_ATTRIB 0x8626
#define GL_VERTEX_PROGRAM_POINT_SIZE 0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE 0x8643
#define GL_VERTEX_ATTRIB_ARRAY_POINTER 0x8645
#define GL_STENCIL_BACK_FUNC 0x8800
#define GL_STENCIL_BACK_FAIL 0x8801
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL 0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS 0x8803
#define GL_MAX_DRAW_BUFFERS 0x8824
#define GL_DRAW_BUFFER0 0x8825
#define GL_DRAW_BUFFER1 0x8826
#define GL_DRAW_BUFFER2 0x8827
#define GL_DRAW_BUFFER3 0x8828
#define GL_DRAW_BUFFER4 0x8829
#define GL_DRAW_BUFFER5 0x882A
#define GL_DRAW_BUFFER6 0x882B
#define GL_DRAW_BUFFER7 0x882C
#define GL_DRAW_BUFFER8 0x882D
#define GL_DRAW_BUFFER9 0x882E
#define GL_DRAW_BUFFER10 0x882F
#define GL_DRAW_BUFFER11 0x8830
#define GL_DRAW_BUFFER12 0x8831
#define GL_DRAW_BUFFER13 0x8832
#define GL_DRAW_BUFFER14 0x8833
#define GL_DRAW_BUFFER15 0x8834
#define GL_BLEND_EQUATION_ALPHA 0x883D
#define GL_POINT_SPRITE 0x8861
#define GL_COORD_REPLACE 0x8862
#define GL_MAX_VERTEX_ATTRIBS 0x8869
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED 0x886A
#define GL_MAX_TEXTURE_COORDS 0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS 0x8872
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS 0x8B49
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS 0x8B4A
#define GL_MAX_VARYING_FLOATS 0x8B4B
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS 0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
#define GL_SHADER_TYPE 0x8B4F
#define GL_FLOAT_VEC2 0x8B50
#define GL_FLOAT_VEC3 0x8B51
#define GL_FLOAT_VEC4 0x8B52
#define GL_INT_VEC2 0x8B53
#define GL_INT_VEC3 0x8B54
#define GL_INT_VEC4 0x8B55
#define GL_BOOL 0x8B56
#define GL_BOOL_VEC2 0x8B57
#define GL_BOOL_VEC3 0x8B58
#define GL_BOOL_VEC4 0x8B59
#define GL_FLOAT_MAT2 0x8B5A
#define GL_FLOAT_MAT3 0x8B5B
#define GL_FLOAT_MAT4 0x8B5C
#define GL_SAMPLER_1D 0x8B5D
#define GL_SAMPLER_2D 0x8B5E
#define GL_SAMPLER_3D 0x8B5F
#define GL_SAMPLER_CUBE 0x8B60
#define GL_SAMPLER_1D_SHADOW 0x8B61
#define GL_SAMPLER_2D_SHADOW 0x8B62
#define GL_DELETE_STATUS 0x8B80
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_VALIDATE_STATUS 0x8B83
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_ATTACHED_SHADERS 0x8B85
#define GL_ACTIVE_UNIFORMS 0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH 0x8B87
#define GL_SHADER_SOURCE_LENGTH 0x8B88
#define GL_ACTIVE_ATTRIBUTES 0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH 0x8B8A
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT 0x8B8B
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#define GL_CURRENT_PROGRAM 0x8B8D
#define GL_POINT_SPRITE_COORD_ORIGIN 0x8CA0
#define GL_LOWER_LEFT 0x8CA1
#define GL_UPPER_LEFT 0x8CA2
#define GL_STENCIL_BACK_REF 0x8CA3
#define GL_STENCIL_BACK_VALUE_MASK 0x8CA4
#define GL_STENCIL_BACK_WRITEMASK 0x8CA5

#endif  // GR_WIN32_BUILD

#endif
