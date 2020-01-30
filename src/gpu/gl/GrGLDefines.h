/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrGLDefines_DEFINED
#define GrGLDefines_DEFINED

/* Profiles */
#define GR_GL_CONTEXT_PROFILE_MASK              0x9126
#define GR_GL_CONTEXT_CORE_PROFILE_BIT          0x00000001
#define GR_GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002

// The following constants consist of the intersection of GL constants
// exported by GLES 1.0, GLES 2.0, and desktop GL required by the system.

#define GR_GL_DEPTH_BUFFER_BIT               0x00000100
#define GR_GL_STENCIL_BUFFER_BIT             0x00000400
#define GR_GL_COLOR_BUFFER_BIT               0x00004000

/* Boolean */
#define GR_GL_FALSE                          0
#define GR_GL_TRUE                           1

/* BeginMode */
#define GR_GL_POINTS                         0x0000
#define GR_GL_LINES                          0x0001
#define GR_GL_LINE_LOOP                      0x0002
#define GR_GL_LINE_STRIP                     0x0003
#define GR_GL_TRIANGLES                      0x0004
#define GR_GL_TRIANGLE_STRIP                 0x0005
#define GR_GL_TRIANGLE_FAN                   0x0006
#define GR_GL_PATCHES                        0x000E

/* AlphaFunction (not supported in ES20) */
/*      GL_NEVER */
/*      GL_LESS */
/*      GL_EQUAL */
/*      GL_LEQUAL */
/*      GL_GREATER */
/*      GL_NOTEQUAL */
/*      GL_GEQUAL */
/*      GL_ALWAYS */

/* Basic OpenGL blend equations */
#define GR_GL_FUNC_ADD                       0x8006
#define GR_GL_FUNC_SUBTRACT                  0x800A
#define GR_GL_FUNC_REVERSE_SUBTRACT          0x800B

/* GL_KHR_blend_equation_advanced */
#define GR_GL_SCREEN                         0x9295
#define GR_GL_OVERLAY                        0x9296
#define GR_GL_DARKEN                         0x9297
#define GR_GL_LIGHTEN                        0x9298
#define GR_GL_COLORDODGE                     0x9299
#define GR_GL_COLORBURN                      0x929A
#define GR_GL_HARDLIGHT                      0x929B
#define GR_GL_SOFTLIGHT                      0x929C
#define GR_GL_DIFFERENCE                     0x929E
#define GR_GL_EXCLUSION                      0x92A0
#define GR_GL_MULTIPLY                       0x9294
#define GR_GL_HSL_HUE                        0x92AD
#define GR_GL_HSL_SATURATION                 0x92AE
#define GR_GL_HSL_COLOR                      0x92AF
#define GR_GL_HSL_LUMINOSITY                 0x92B0

/* BlendingFactorDest */
#define GR_GL_ZERO                           0
#define GR_GL_ONE                            1
#define GR_GL_SRC_COLOR                      0x0300
#define GR_GL_ONE_MINUS_SRC_COLOR            0x0301
#define GR_GL_SRC_ALPHA                      0x0302
#define GR_GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GR_GL_DST_ALPHA                      0x0304
#define GR_GL_ONE_MINUS_DST_ALPHA            0x0305

/* BlendingFactorSrc */
/*      GL_ZERO */
/*      GL_ONE */
#define GR_GL_DST_COLOR                      0x0306
#define GR_GL_ONE_MINUS_DST_COLOR            0x0307
#define GR_GL_SRC_ALPHA_SATURATE             0x0308
/*      GL_SRC_ALPHA */
/*      GL_ONE_MINUS_SRC_ALPHA */
/*      GL_DST_ALPHA */
/*      GL_ONE_MINUS_DST_ALPHA */

/* ExtendedBlendFactors */
#define GR_GL_SRC1_COLOR                     0x88F9
#define GR_GL_ONE_MINUS_SRC1_COLOR           0x88FA
/*      GL_SRC1_ALPHA */
#define GR_GL_ONE_MINUS_SRC1_ALPHA           0x88FB

/* Separate Blend Functions */
#define GR_GL_BLEND_DST_RGB                  0x80C8
#define GR_GL_BLEND_SRC_RGB                  0x80C9
#define GR_GL_BLEND_DST_ALPHA                0x80CA
#define GR_GL_BLEND_SRC_ALPHA                0x80CB
#define GR_GL_CONSTANT_COLOR                 0x8001
#define GR_GL_ONE_MINUS_CONSTANT_COLOR       0x8002
#define GR_GL_CONSTANT_ALPHA                 0x8003
#define GR_GL_ONE_MINUS_CONSTANT_ALPHA       0x8004
#define GR_GL_BLEND_COLOR                    0x8005

/* Buffer Objects */
#define GR_GL_ARRAY_BUFFER                   0x8892
#define GR_GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GR_GL_DRAW_INDIRECT_BUFFER           0x8F3F
#define GR_GL_TEXTURE_BUFFER                 0x8C2A
#define GR_GL_ARRAY_BUFFER_BINDING           0x8894
#define GR_GL_ELEMENT_ARRAY_BUFFER_BINDING   0x8895
#define GR_GL_DRAW_INDIRECT_BUFFER_BINDING   0x8F43
#define GR_GL_PIXEL_PACK_BUFFER              0x88EB
#define GR_GL_PIXEL_UNPACK_BUFFER            0x88EC

#define GR_GL_PIXEL_UNPACK_TRANSFER_BUFFER_CHROMIUM 0x78EC
#define GR_GL_PIXEL_PACK_TRANSFER_BUFFER_CHROMIUM   0x78ED

#define GR_GL_STREAM_DRAW                    0x88E0
#define GR_GL_STREAM_READ                    0x88E1
#define GR_GL_STATIC_DRAW                    0x88E4
#define GR_GL_STATIC_READ                    0x88E5
#define GR_GL_DYNAMIC_DRAW                   0x88E8
#define GR_GL_DYNAMIC_READ                   0x88E9

#define GR_GL_BUFFER_SIZE                    0x8764
#define GR_GL_BUFFER_USAGE                   0x8765

#define GR_GL_CURRENT_VERTEX_ATTRIB          0x8626

/* CullFaceMode */
#define GR_GL_FRONT                          0x0404
#define GR_GL_BACK                           0x0405
#define GR_GL_FRONT_AND_BACK                 0x0408

/* DepthFunction */
/*      GL_NEVER */
/*      GL_LESS */
/*      GL_EQUAL */
/*      GL_LEQUAL */
/*      GL_GREATER */
/*      GL_NOTEQUAL */
/*      GL_GEQUAL */
/*      GL_ALWAYS */

/* EnableCap */
#define GR_GL_TEXTURE_NONE                   0x0000
#define GR_GL_TEXTURE_2D                     0x0DE1
#define GR_GL_CULL_FACE                      0x0B44
#define GR_GL_BLEND                          0x0BE2
#define GR_GL_DITHER                         0x0BD0
#define GR_GL_STENCIL_TEST                   0x0B90
#define GR_GL_DEPTH_TEST                     0x0B71
#define GR_GL_SCISSOR_TEST                   0x0C11
#define GR_GL_POLYGON_OFFSET_FILL            0x8037
#define GR_GL_SAMPLE_ALPHA_TO_COVERAGE       0x809E
#define GR_GL_SAMPLE_COVERAGE                0x80A0
#define GR_GL_POLYGON_OFFSET_FILL            0x8037
#define GR_GL_POLYGON_SMOOTH                 0x0B41
#define GR_GL_POLYGON_STIPPLE                0x0B42
#define GR_GL_COLOR_LOGIC_OP                 0x0BF2
#define GR_GL_COLOR_TABLE                    0x80D0
#define GR_GL_INDEX_LOGIC_OP                 0x0BF1
#define GR_GL_VERTEX_PROGRAM_POINT_SIZE      0x8642
#define GR_GL_LINE_STIPPLE                   0x0B24
#define GR_GL_FRAMEBUFFER_SRGB               0x8DB9
#define GR_GL_SHADER_PIXEL_LOCAL_STORAGE     0x8F64
#define GR_GL_SAMPLE_SHADING                 0x8C36

/* ErrorCode */
#define GR_GL_NO_ERROR                       0
#define GR_GL_INVALID_ENUM                   0x0500
#define GR_GL_INVALID_VALUE                  0x0501
#define GR_GL_INVALID_OPERATION              0x0502
#define GR_GL_OUT_OF_MEMORY                  0x0505
#define GR_GL_CONTEXT_LOST                   0x300E  // TODO(gman): What value?

/* FrontFaceDirection */
#define GR_GL_CW                             0x0900
#define GR_GL_CCW                            0x0901

/* GetPName */
#define GR_GL_LINE_WIDTH                     0x0B21
#define GR_GL_ALIASED_POINT_SIZE_RANGE       0x846D
#define GR_GL_ALIASED_LINE_WIDTH_RANGE       0x846E
#define GR_GL_CULL_FACE_MODE                 0x0B45
#define GR_GL_FRONT_FACE                     0x0B46
#define GR_GL_DEPTH_RANGE                    0x0B70
#define GR_GL_DEPTH_WRITEMASK                0x0B72
#define GR_GL_DEPTH_CLEAR_VALUE              0x0B73
#define GR_GL_DEPTH_FUNC                     0x0B74
#define GR_GL_STENCIL_CLEAR_VALUE            0x0B91
#define GR_GL_STENCIL_FUNC                   0x0B92
#define GR_GL_STENCIL_FAIL                   0x0B94
#define GR_GL_STENCIL_PASS_DEPTH_FAIL        0x0B95
#define GR_GL_STENCIL_PASS_DEPTH_PASS        0x0B96
#define GR_GL_STENCIL_REF                    0x0B97
#define GR_GL_STENCIL_VALUE_MASK             0x0B93
#define GR_GL_STENCIL_WRITEMASK              0x0B98
#define GR_GL_STENCIL_BACK_FUNC              0x8800
#define GR_GL_STENCIL_BACK_FAIL              0x8801
#define GR_GL_STENCIL_BACK_PASS_DEPTH_FAIL   0x8802
#define GR_GL_STENCIL_BACK_PASS_DEPTH_PASS   0x8803
#define GR_GL_STENCIL_BACK_REF               0x8CA3
#define GR_GL_STENCIL_BACK_VALUE_MASK        0x8CA4
#define GR_GL_STENCIL_BACK_WRITEMASK         0x8CA5
#define GR_GL_VIEWPORT                       0x0BA2
#define GR_GL_SCISSOR_BOX                    0x0C10
/*      GL_SCISSOR_TEST */
#define GR_GL_COLOR_CLEAR_VALUE              0x0C22
#define GR_GL_COLOR_WRITEMASK                0x0C23
#define GR_GL_UNPACK_ALIGNMENT               0x0CF5
#define GR_GL_PACK_ALIGNMENT                 0x0D05
#define GR_GL_PACK_REVERSE_ROW_ORDER         0x93A4
#define GR_GL_MAX_TEXTURE_SIZE               0x0D33
#define GR_GL_TEXTURE_MIN_LOD                0x813A
#define GR_GL_TEXTURE_MAX_LOD                0x813B
#define GR_GL_TEXTURE_BASE_LEVEL             0x813C
#define GR_GL_TEXTURE_MAX_LEVEL              0x813D
#define GR_GL_MAX_VIEWPORT_DIMS              0x0D3A
#define GR_GL_SUBPIXEL_BITS                  0x0D50
#define GR_GL_RED_BITS                       0x0D52
#define GR_GL_GREEN_BITS                     0x0D53
#define GR_GL_BLUE_BITS                      0x0D54
#define GR_GL_ALPHA_BITS                     0x0D55
#define GR_GL_DEPTH_BITS                     0x0D56
#define GR_GL_STENCIL_BITS                   0x0D57
#define GR_GL_POLYGON_OFFSET_UNITS           0x2A00
/*      GL_POLYGON_OFFSET_FILL */
#define GR_GL_POLYGON_OFFSET_FACTOR          0x8038
#define GR_GL_TEXTURE_BINDING_2D             0x8069
#define GR_GL_SAMPLE_BUFFERS                 0x80A8
#define GR_GL_SAMPLES                        0x80A9
#define GR_GL_SAMPLE_COVERAGE_VALUE          0x80AA
#define GR_GL_SAMPLE_COVERAGE_INVERT         0x80AB
#define GR_GL_RENDERBUFFER_COVERAGE_SAMPLES  0x8CAB
#define GR_GL_RENDERBUFFER_COLOR_SAMPLES     0x8E10
#define GR_GL_MAX_MULTISAMPLE_COVERAGE_MODES 0x8E11
#define GR_GL_MULTISAMPLE_COVERAGE_MODES     0x8E12
#define GR_GL_MAX_TEXTURE_BUFFER_SIZE        0x8C2B

/* GetTextureParameter */
/*      GL_TEXTURE_MAG_FILTER */
/*      GL_TEXTURE_MIN_FILTER */
/*      GL_TEXTURE_WRAP_S */
/*      GL_TEXTURE_WRAP_T */

#define GR_GL_NUM_COMPRESSED_TEXTURE_FORMATS 0x86A2
#define GR_GL_COMPRESSED_TEXTURE_FORMATS     0x86A3

/* Compressed Texture Formats */
#define GR_GL_COMPRESSED_RGB_S3TC_DXT1_EXT             0x83F0
#define GR_GL_COMPRESSED_RGBA_S3TC_DXT1_EXT            0x83F1
#define GR_GL_COMPRESSED_RGBA_S3TC_DXT3_EXT            0x83F2
#define GR_GL_COMPRESSED_RGBA_S3TC_DXT5_EXT            0x83F3

#define GR_GL_COMPRESSED_SRGB_S3TC_DXT1_EXT            0x8C4C
#define GR_GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT      0x8C4D
#define GR_GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT      0x8C4E
#define GR_GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT      0x8C4F

#define GR_GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG          0x8C00
#define GR_GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG          0x8C01
#define GR_GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG         0x8C02
#define GR_GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG         0x8C03

#define GR_GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG         0x9137
#define GR_GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG         0x9138

#define GR_GL_COMPRESSED_ETC1_RGB8                     0x8D64

#define GR_GL_COMPRESSED_R11_EAC                       0x9270
#define GR_GL_COMPRESSED_SIGNED_R11_EAC                0x9271
#define GR_GL_COMPRESSED_RG11_EAC                      0x9272
#define GR_GL_COMPRESSED_SIGNED_RG11_EAC               0x9273

#define GR_GL_COMPRESSED_RGB8_ETC2                     0x9274
#define GR_GL_COMPRESSED_SRGB8_ETC2                    0x9275
#define GR_GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1      0x9276
#define GR_GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1     0x9277
#define GR_GL_COMPRESSED_RGBA8_ETC2_EAC                0x9278
#define GR_GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC         0x9279

#define GR_GL_COMPRESSED_LUMINANCE_LATC1               0x8C70
#define GR_GL_COMPRESSED_SIGNED_LUMINANCE_LATC1        0x8C71
#define GR_GL_COMPRESSED_LUMINANCE_ALPHA_LATC2         0x8C72
#define GR_GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2  0x8C73

#define GR_GL_COMPRESSED_RED_RGTC1                     0x8DBB
#define GR_GL_COMPRESSED_SIGNED_RED_RGTC1              0x8DBC
#define GR_GL_COMPRESSED_RED_GREEN_RGTC2               0x8DBD
#define GR_GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2        0x8DBE

#define GR_GL_COMPRESSED_3DC_X                         0x87F9
#define GR_GL_COMPRESSED_3DC_XY                        0x87FA

#define GR_GL_COMPRESSED_RGBA_BPTC_UNORM               0x8E8C
#define GR_GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM         0x8E8D
#define GR_GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT         0x8E8E
#define GR_GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT       0x8E8F

#define GR_GL_COMPRESSED_RGBA_ASTC_4x4                 0x93B0
#define GR_GL_COMPRESSED_RGBA_ASTC_5x4                 0x93B1
#define GR_GL_COMPRESSED_RGBA_ASTC_5x5                 0x93B2
#define GR_GL_COMPRESSED_RGBA_ASTC_6x5                 0x93B3
#define GR_GL_COMPRESSED_RGBA_ASTC_6x6                 0x93B4
#define GR_GL_COMPRESSED_RGBA_ASTC_8x5                 0x93B5
#define GR_GL_COMPRESSED_RGBA_ASTC_8x6                 0x93B6
#define GR_GL_COMPRESSED_RGBA_ASTC_8x8                 0x93B7
#define GR_GL_COMPRESSED_RGBA_ASTC_10x5                0x93B8
#define GR_GL_COMPRESSED_RGBA_ASTC_10x6                0x93B9
#define GR_GL_COMPRESSED_RGBA_ASTC_10x8                0x93BA
#define GR_GL_COMPRESSED_RGBA_ASTC_10x10               0x93BB
#define GR_GL_COMPRESSED_RGBA_ASTC_12x10               0x93BC
#define GR_GL_COMPRESSED_RGBA_ASTC_12x12               0x93BD

#define GR_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4         0x93D0
#define GR_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4         0x93D1
#define GR_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5         0x93D2
#define GR_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5         0x93D3
#define GR_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6         0x93D4
#define GR_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5         0x93D5
#define GR_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6         0x93D6
#define GR_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8         0x93D7
#define GR_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5        0x93D8
#define GR_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6        0x93D9
#define GR_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8        0x93DA
#define GR_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10       0x93DB
#define GR_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10       0x93DC
#define GR_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12       0x93DD

/* HintMode */
#define GR_GL_DONT_CARE                      0x1100
#define GR_GL_FASTEST                        0x1101
#define GR_GL_NICEST                         0x1102

/* HintTarget */
#define GR_GL_GENERATE_MIPMAP_HINT            0x8192

/* DataType */
#define GR_GL_BYTE                           0x1400
#define GR_GL_UNSIGNED_BYTE                  0x1401
#define GR_GL_SHORT                          0x1402
#define GR_GL_UNSIGNED_SHORT                 0x1403
#define GR_GL_INT                            0x1404
#define GR_GL_UNSIGNED_INT                   0x1405
#define GR_GL_FLOAT                          0x1406
#define GR_GL_HALF_FLOAT                     0x140B
#define GR_GL_FIXED                          0x140C
#define GR_GL_HALF_FLOAT_OES                 0x8D61

/* Lighting */
#define GR_GL_LIGHTING                       0x0B50
#define GR_GL_LIGHT0                         0x4000
#define GR_GL_LIGHT1                         0x4001
#define GR_GL_LIGHT2                         0x4002
#define GR_GL_LIGHT3                         0x4003
#define GR_GL_LIGHT4                         0x4004
#define GR_GL_LIGHT5                         0x4005
#define GR_GL_LIGHT6                         0x4006
#define GR_GL_LIGHT7                         0x4007
#define GR_GL_SPOT_EXPONENT                  0x1205
#define GR_GL_SPOT_CUTOFF                    0x1206
#define GR_GL_CONSTANT_ATTENUATION           0x1207
#define GR_GL_LINEAR_ATTENUATION             0x1208
#define GR_GL_QUADRATIC_ATTENUATION          0x1209
#define GR_GL_AMBIENT                        0x1200
#define GR_GL_DIFFUSE                        0x1201
#define GR_GL_SPECULAR                       0x1202
#define GR_GL_SHININESS                      0x1601
#define GR_GL_EMISSION                       0x1600
#define GR_GL_POSITION                       0x1203
#define GR_GL_SPOT_DIRECTION                 0x1204
#define GR_GL_AMBIENT_AND_DIFFUSE            0x1602
#define GR_GL_COLOR_INDEXES                  0x1603
#define GR_GL_LIGHT_MODEL_TWO_SIDE           0x0B52
#define GR_GL_LIGHT_MODEL_LOCAL_VIEWER       0x0B51
#define GR_GL_LIGHT_MODEL_AMBIENT            0x0B53
#define GR_GL_FRONT_AND_BACK                 0x0408
#define GR_GL_SHADE_MODEL                    0x0B54
#define GR_GL_FLAT                           0x1D00
#define GR_GL_SMOOTH                         0x1D01
#define GR_GL_COLOR_MATERIAL                 0x0B57
#define GR_GL_COLOR_MATERIAL_FACE            0x0B55
#define GR_GL_COLOR_MATERIAL_PARAMETER       0x0B56
#define GR_GL_NORMALIZE                      0x0BA1

/* Matrix Mode */
#define GR_GL_MATRIX_MODE                    0x0BA0
#define GR_GL_MODELVIEW                      0x1700
#define GR_GL_PROJECTION                     0x1701
#define GR_GL_TEXTURE                        0x1702

/* multisample */
#define GR_GL_MULTISAMPLE                    0x809D
#define GR_GL_SAMPLE_POSITION                0x8E50

/* Points */
#define GR_GL_POINT_SMOOTH                   0x0B10
#define GR_GL_POINT_SIZE                     0x0B11
#define GR_GL_POINT_SIZE_GRANULARITY         0x0B13
#define GR_GL_POINT_SIZE_RANGE               0x0B12

/* Lines */
#define GR_GL_LINE_SMOOTH                    0x0B20
#define GR_GL_LINE_STIPPLE                   0x0B24
#define GR_GL_LINE_STIPPLE_PATTERN           0x0B25
#define GR_GL_LINE_STIPPLE_REPEAT            0x0B26
#define GR_GL_LINE_WIDTH                     0x0B21
#define GR_GL_LINE_WIDTH_GRANULARITY         0x0B23
#define GR_GL_LINE_WIDTH_RANGE               0x0B22

/* PolygonMode */
#define GR_GL_POINT                          0x1B00
#define GR_GL_LINE                           0x1B01
#define GR_GL_FILL                           0x1B02

/* Unsized formats */
#define GR_GL_STENCIL_INDEX                  0x1901
#define GR_GL_DEPTH_COMPONENT                0x1902
#define GR_GL_DEPTH_STENCIL                  0x84F9
#define GR_GL_RED                            0x1903
#define GR_GL_RED_INTEGER                    0x8D94
#define GR_GL_GREEN                          0x1904
#define GR_GL_BLUE                           0x1905
#define GR_GL_ALPHA                          0x1906
#define GR_GL_LUMINANCE                      0x1909
#define GR_GL_LUMINANCE_ALPHA                0x190A
#define GR_GL_RG_INTEGER                     0x8228
#define GR_GL_RGB                            0x1907
#define GR_GL_RGB_INTEGER                    0x8D98
#define GR_GL_SRGB                           0x8C40
#define GR_GL_RGBA                           0x1908
#define GR_GL_RG                             0x8227
#define GR_GL_SRGB_ALPHA                     0x8C42
#define GR_GL_RGBA_INTEGER                   0x8D99
#define GR_GL_BGRA                           0x80E1

/* Stencil index sized formats */
#define GR_GL_STENCIL_INDEX4                 0x8D47
#define GR_GL_STENCIL_INDEX8                 0x8D48
#define GR_GL_STENCIL_INDEX16                0x8D49

/* Depth component sized formats */
#define GR_GL_DEPTH_COMPONENT16              0x81A5

/* Depth stencil sized formats */
#define GR_GL_DEPTH24_STENCIL8               0x88F0

/* Red sized formats */
#define GR_GL_R8                             0x8229
#define GR_GL_R16                            0x822A
#define GR_GL_R16F                           0x822D
#define GR_GL_R32F                           0x822E

/* Red integer sized formats */
#define GR_GL_R8I                            0x8231
#define GR_GL_R8UI                           0x8232
#define GR_GL_R16I                           0x8233
#define GR_GL_R16UI                          0x8234
#define GR_GL_R32I                           0x8235
#define GR_GL_R32UI                          0x8236

/* Luminance sized formats */
#define GR_GL_LUMINANCE8                     0x8040
#define GR_GL_LUMINANCE16F                   0x881E

/* Alpha sized formats */
#define GR_GL_ALPHA8                         0x803C
#define GR_GL_ALPHA16                        0x803E
#define GR_GL_ALPHA16F                       0x881C
#define GR_GL_ALPHA32F                       0x8816

/* Alpha integer sized formats */
#define GR_GL_ALPHA8I                        0x8D90
#define GR_GL_ALPHA8UI                       0x8D7E
#define GR_GL_ALPHA16I                       0x8D8A
#define GR_GL_ALPHA16UI                      0x8D78
#define GR_GL_ALPHA32I                       0x8D84
#define GR_GL_ALPHA32UI                      0x8D72

/* RG sized formats */
#define GR_GL_RG8                            0x822B
#define GR_GL_RG16                           0x822C
#define GR_GL_R16F                           0x822D
#define GR_GL_R32F                           0x822E
#define GR_GL_RG16F                          0x822F

/* RG sized integer formats */
#define GR_GL_RG8I                           0x8237
#define GR_GL_RG8UI                          0x8238
#define GR_GL_RG16I                          0x8239
#define GR_GL_RG16UI                         0x823A
#define GR_GL_RG32I                          0x823B
#define GR_GL_RG32UI                         0x823C

/* RGB sized formats */
#define GR_GL_RGB5                           0x8050
#define GR_GL_RGB565                         0x8D62
#define GR_GL_RGB8                           0x8051
#define GR_GL_SRGB8                          0x8C41

/* RGB integer sized formats */
#define GR_GL_RGB8I                          0x8D8F
#define GR_GL_RGB8UI                         0x8D7D
#define GR_GL_RGB16I                         0x8D89
#define GR_GL_RGB16UI                        0x8D77
#define GR_GL_RGB32I                         0x8D83
#define GR_GL_RGB32UI                        0x8D71

/* RGBA sized formats */
#define GR_GL_RGBA4                          0x8056
#define GR_GL_RGB5_A1                        0x8057
#define GR_GL_RGBA8                          0x8058
#define GR_GL_RGB10_A2                       0x8059
#define GR_GL_SRGB8_ALPHA8                   0x8C43
#define GR_GL_RGBA16F                        0x881A
#define GR_GL_RGBA32F                        0x8814
#define GR_GL_RG32F                          0x8230
#define GR_GL_RGBA16                         0x805B

/* RGBA integer sized formats */
#define GR_GL_RGBA8I                         0x8D8E
#define GR_GL_RGBA8UI                        0x8D7C
#define GR_GL_RGBA16I                        0x8D88
#define GR_GL_RGBA16UI                       0x8D76
#define GR_GL_RGBA32I                        0x8D82
#define GR_GL_RGBA32UI                       0x8D70

/* BGRA sized formats */
#define GR_GL_BGRA8                          0x93A1

/* PixelType */
/*      GL_UNSIGNED_BYTE */
#define GR_GL_UNSIGNED_SHORT_4_4_4_4         0x8033
#define GR_GL_UNSIGNED_SHORT_5_5_5_1         0x8034
#define GR_GL_UNSIGNED_SHORT_5_6_5           0x8363
#define GR_GL_UNSIGNED_INT_2_10_10_10_REV    0x8368

/* Shaders */
#define GR_GL_FRAGMENT_SHADER                          0x8B30
#define GR_GL_VERTEX_SHADER                            0x8B31
#define GR_GL_GEOMETRY_SHADER                          0x8DD9
#define GR_GL_TESS_CONTROL_SHADER                      0x8E88
#define GR_GL_TESS_EVALUATION_SHADER                   0x8E87
#define GR_GL_MAX_VERTEX_ATTRIBS                       0x8869
#define GR_GL_MAX_VERTEX_UNIFORM_VECTORS               0x8DFB
#define GR_GL_MAX_VARYING_VECTORS                      0x8DFC
#define GR_GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS         0x8B4D
#define GR_GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS           0x8B4C
#define GR_GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS         0x8C29
#define GR_GL_MAX_TEXTURE_IMAGE_UNITS                  0x8872
#define GR_GL_MAX_FRAGMENT_UNIFORM_VECTORS             0x8DFD
#define GR_GL_SHADER_TYPE                              0x8B4F
#define GR_GL_DELETE_STATUS                            0x8B80
#define GR_GL_LINK_STATUS                              0x8B82
#define GR_GL_VALIDATE_STATUS                          0x8B83
#define GR_GL_ATTACHED_SHADERS                         0x8B85
#define GR_GL_ACTIVE_UNIFORMS                          0x8B86
#define GR_GL_ACTIVE_UNIFORM_MAX_LENGTH                0x8B87
#define GR_GL_ACTIVE_ATTRIBUTES                        0x8B89
#define GR_GL_ACTIVE_ATTRIBUTE_MAX_LENGTH              0x8B8A
#define GR_GL_SHADING_LANGUAGE_VERSION                 0x8B8C
#define GR_GL_CURRENT_PROGRAM                          0x8B8D
#define GR_GL_MAX_FRAGMENT_UNIFORM_COMPONENTS          0x8B49
#define GR_GL_MAX_VERTEX_UNIFORM_COMPONENTS            0x8B4A
#define GR_GL_MAX_SHADER_PIXEL_LOCAL_STORAGE_FAST_SIZE 0x8F63
#define GR_GL_SHADER_BINARY_FORMATS                    0x8DF8

/* StencilFunction */
#define GR_GL_NEVER                          0x0200
#define GR_GL_LESS                           0x0201
#define GR_GL_EQUAL                          0x0202
#define GR_GL_LEQUAL                         0x0203
#define GR_GL_GREATER                        0x0204
#define GR_GL_NOTEQUAL                       0x0205
#define GR_GL_GEQUAL                         0x0206
#define GR_GL_ALWAYS                         0x0207

/* StencilOp */
/*      GL_ZERO */
#define GR_GL_KEEP                           0x1E00
#define GR_GL_REPLACE                        0x1E01
#define GR_GL_INCR                           0x1E02
#define GR_GL_DECR                           0x1E03
#define GR_GL_INVERT                         0x150A
#define GR_GL_INCR_WRAP                      0x8507
#define GR_GL_DECR_WRAP                      0x8508

/* StringName */
#define GR_GL_VENDOR                         0x1F00
#define GR_GL_RENDERER                       0x1F01
#define GR_GL_VERSION                        0x1F02
#define GR_GL_EXTENSIONS                     0x1F03

/* StringCounts */
#define GR_GL_NUM_EXTENSIONS                 0x821D

/* Pixel Mode / Transfer */
#define GR_GL_UNPACK_ROW_LENGTH              0x0CF2
#define GR_GL_PACK_ROW_LENGTH                0x0D02


/* TextureMagFilter */
#define GR_GL_NEAREST                        0x2600
#define GR_GL_LINEAR                         0x2601

/* TextureMinFilter */
/*      GL_NEAREST */
/*      GL_LINEAR */
#define GR_GL_NEAREST_MIPMAP_NEAREST         0x2700
#define GR_GL_LINEAR_MIPMAP_NEAREST          0x2701
#define GR_GL_NEAREST_MIPMAP_LINEAR          0x2702
#define GR_GL_LINEAR_MIPMAP_LINEAR           0x2703

/* TextureUsage */
#define GR_GL_FRAMEBUFFER_ATTACHMENT         0x93A3

/* TextureParameterName */
#define GR_GL_TEXTURE_MAG_FILTER             0x2800
#define GR_GL_TEXTURE_MIN_FILTER             0x2801
#define GR_GL_TEXTURE_WRAP_S                 0x2802
#define GR_GL_TEXTURE_WRAP_T                 0x2803
#define GR_GL_TEXTURE_USAGE                  0x93A2

/* TextureTarget */
/*      GL_TEXTURE_2D */
#define GR_GL_TEXTURE                        0x1702
#define GR_GL_TEXTURE_CUBE_MAP               0x8513
#define GR_GL_TEXTURE_BINDING_CUBE_MAP       0x8514
#define GR_GL_TEXTURE_CUBE_MAP_POSITIVE_X    0x8515
#define GR_GL_TEXTURE_CUBE_MAP_NEGATIVE_X    0x8516
#define GR_GL_TEXTURE_CUBE_MAP_POSITIVE_Y    0x8517
#define GR_GL_TEXTURE_CUBE_MAP_NEGATIVE_Y    0x8518
#define GR_GL_TEXTURE_CUBE_MAP_POSITIVE_Z    0x8519
#define GR_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z    0x851A
#define GR_GL_MAX_CUBE_MAP_TEXTURE_SIZE      0x851C

/* TextureUnit */
#define GR_GL_TEXTURE0                       0x84C0
#define GR_GL_TEXTURE1                       0x84C1
#define GR_GL_TEXTURE2                       0x84C2
#define GR_GL_TEXTURE3                       0x84C3
#define GR_GL_TEXTURE4                       0x84C4
#define GR_GL_TEXTURE5                       0x84C5
#define GR_GL_TEXTURE6                       0x84C6
#define GR_GL_TEXTURE7                       0x84C7
#define GR_GL_TEXTURE8                       0x84C8
#define GR_GL_TEXTURE9                       0x84C9
#define GR_GL_TEXTURE10                      0x84CA
#define GR_GL_TEXTURE11                      0x84CB
#define GR_GL_TEXTURE12                      0x84CC
#define GR_GL_TEXTURE13                      0x84CD
#define GR_GL_TEXTURE14                      0x84CE
#define GR_GL_TEXTURE15                      0x84CF
#define GR_GL_TEXTURE16                      0x84D0
#define GR_GL_TEXTURE17                      0x84D1
#define GR_GL_TEXTURE18                      0x84D2
#define GR_GL_TEXTURE19                      0x84D3
#define GR_GL_TEXTURE20                      0x84D4
#define GR_GL_TEXTURE21                      0x84D5
#define GR_GL_TEXTURE22                      0x84D6
#define GR_GL_TEXTURE23                      0x84D7
#define GR_GL_TEXTURE24                      0x84D8
#define GR_GL_TEXTURE25                      0x84D9
#define GR_GL_TEXTURE26                      0x84DA
#define GR_GL_TEXTURE27                      0x84DB
#define GR_GL_TEXTURE28                      0x84DC
#define GR_GL_TEXTURE29                      0x84DD
#define GR_GL_TEXTURE30                      0x84DE
#define GR_GL_TEXTURE31                      0x84DF
#define GR_GL_ACTIVE_TEXTURE                 0x84E0
#define GR_GL_MAX_TEXTURE_UNITS              0x84E2
#define GR_GL_MAX_TEXTURE_COORDS             0x8871

/* TextureWrapMode */
#define GR_GL_REPEAT                         0x2901
#define GR_GL_CLAMP_TO_EDGE                  0x812F
#define GR_GL_MIRRORED_REPEAT                0x8370
#define GR_GL_CLAMP_TO_BORDER                0x812D

/* Texture Swizzle */
#define GR_GL_TEXTURE_SWIZZLE_R              0x8E42
#define GR_GL_TEXTURE_SWIZZLE_G              0x8E43
#define GR_GL_TEXTURE_SWIZZLE_B              0x8E44
#define GR_GL_TEXTURE_SWIZZLE_A              0x8E45
#define GR_GL_TEXTURE_SWIZZLE_RGBA           0x8E46

/* Texture mapping */
#define GR_GL_TEXTURE_ENV                    0x2300
#define GR_GL_TEXTURE_ENV_MODE               0x2200
#define GR_GL_TEXTURE_1D                     0x0DE0
/* GL_TEXTURE_2D */
/* GL_TEXTURE_WRAP_S */
/* GL_TEXTURE_WRAP_T */
/* GL_TEXTURE_MAG_FILTER */
/* GL_TEXTURE_MIN_FILTER */
#define GR_GL_TEXTURE_ENV_COLOR             0x2201
#define GR_GL_TEXTURE_GEN_S                 0x0C60
#define GR_GL_TEXTURE_GEN_T                 0x0C61
#define GR_GL_TEXTURE_GEN_R                 0x0C62
#define GR_GL_TEXTURE_GEN_Q                 0x0C63
#define GR_GL_TEXTURE_GEN_MODE              0x2500
#define GR_GL_TEXTURE_BORDER_COLOR          0x1004
#define GR_GL_TEXTURE_WIDTH                 0x1000
#define GR_GL_TEXTURE_HEIGHT                0x1001
#define GR_GL_TEXTURE_BORDER                0x1005
#define GR_GL_TEXTURE_COMPONENTS            0x1003
#define GR_GL_TEXTURE_RED_SIZE              0x805C
#define GR_GL_TEXTURE_GREEN_SIZE            0x805D
#define GR_GL_TEXTURE_BLUE_SIZE             0x805E
#define GR_GL_TEXTURE_ALPHA_SIZE            0x805F
#define GR_GL_TEXTURE_LUMINANCE_SIZE        0x8060
#define GR_GL_TEXTURE_INTENSITY_SIZE        0x8061
#define GR_GL_TEXTURE_INTERNAL_FORMAT       0x1003
/* GL_NEAREST_MIPMAP_NEAREST */
/* GL_NEAREST_MIPMAP_LINEAR */
/* GL_LINEAR_MIPMAP_NEAREST */
/* GL_LINEAR_MIPMAP_LINEAR */
#define GR_GL_OBJECT_LINEAR                 0x2401
#define GR_GL_OBJECT_PLANE                  0x2501
#define GR_GL_EYE_LINEAR                    0x2400
#define GR_GL_EYE_PLANE                     0x2502
#define GR_GL_SPHERE_MAP                    0x2402
#define GR_GL_DECAL                         0x2101
#define GR_GL_MODULATE                      0x2100
/* GL_NEAREST */
/* GL_REPEAT */
#define GR_GL_CLAMP                         0x2900
#define GR_GL_S                             0x2000
#define GR_GL_T                             0x2001
#define GR_GL_R                             0x2002
#define GR_GL_Q                             0x2003
#define GR_GL_TEXTURE_GEN_R                 0x0C62
#define GR_GL_TEXTURE_GEN_Q                 0x0C63

/* texture_env_combine */
#define GR_GL_COMBINE                       0x8570
#define GR_GL_COMBINE_RGB                   0x8571
#define GR_GL_COMBINE_ALPHA                 0x8572
#define GR_GL_SOURCE0_RGB                   0x8580
#define GR_GL_SOURCE1_RGB                   0x8581
#define GR_GL_SOURCE2_RGB                   0x8582
#define GR_GL_SOURCE0_ALPHA                 0x8588
#define GR_GL_SOURCE1_ALPHA                 0x8589
#define GR_GL_SOURCE2_ALPHA                 0x858A
#define GR_GL_OPERAND0_RGB                  0x8590
#define GR_GL_OPERAND1_RGB                  0x8591
#define GR_GL_OPERAND2_RGB                  0x8592
#define GR_GL_OPERAND0_ALPHA                0x8598
#define GR_GL_OPERAND1_ALPHA                0x8599
#define GR_GL_OPERAND2_ALPHA                0x859A
#define GR_GL_RGB_SCALE                     0x8573
#define GR_GL_ADD_SIGNED                    0x8574
#define GR_GL_INTERPOLATE                   0x8575
#define GR_GL_SUBTRACT                      0x84E7
#define GR_GL_CONSTANT                      0x8576
#define GR_GL_PRIMARY_COLOR                 0x8577
#define GR_GL_PREVIOUS                      0x8578
#define GR_GL_SRC0_RGB                      0x8580
#define GR_GL_SRC1_RGB                      0x8581
#define GR_GL_SRC2_RGB                      0x8582
#define GR_GL_SRC0_ALPHA                    0x8588
#define GR_GL_SRC1_ALPHA                    0x8589
#define GR_GL_SRC2_ALPHA                    0x858A

/* Uniform Types */
#define GR_GL_FLOAT_VEC2                     0x8B50
#define GR_GL_FLOAT_VEC3                     0x8B51
#define GR_GL_FLOAT_VEC4                     0x8B52
#define GR_GL_INT_VEC2                       0x8B53
#define GR_GL_INT_VEC3                       0x8B54
#define GR_GL_INT_VEC4                       0x8B55
#define GR_GL_BOOL                           0x8B56
#define GR_GL_BOOL_VEC2                      0x8B57
#define GR_GL_BOOL_VEC3                      0x8B58
#define GR_GL_BOOL_VEC4                      0x8B59
#define GR_GL_FLOAT_MAT2                     0x8B5A
#define GR_GL_FLOAT_MAT3                     0x8B5B
#define GR_GL_FLOAT_MAT4                     0x8B5C
#define GR_GL_SAMPLER_2D                     0x8B5E
#define GR_GL_SAMPLER_CUBE                   0x8B60

/* Vertex Arrays */
#define GR_GL_VERTEX_ATTRIB_ARRAY_ENABLED        0x8622
#define GR_GL_VERTEX_ATTRIB_ARRAY_SIZE           0x8623
#define GR_GL_VERTEX_ATTRIB_ARRAY_STRIDE         0x8624
#define GR_GL_VERTEX_ATTRIB_ARRAY_TYPE           0x8625
#define GR_GL_VERTEX_ATTRIB_ARRAY_NORMALIZED     0x886A
#define GR_GL_VERTEX_ATTRIB_ARRAY_POINTER        0x8645
#define GR_GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING 0x889F
#define GR_GL_VERTEX_ARRAY                       0x8074
#define GR_GL_NORMAL_ARRAY                       0x8075
#define GR_GL_COLOR_ARRAY                        0x8076
#define GR_GL_SECONDARY_COLOR_ARRAY              0x845E
#define GR_GL_INDEX_ARRAY                        0x8077
#define GR_GL_TEXTURE_COORD_ARRAY                0x8078
#define GR_GL_EDGE_FLAG_ARRAY                    0x8079
#define GR_GL_VERTEX_ARRAY_SIZE                  0x807A
#define GR_GL_VERTEX_ARRAY_TYPE                  0x807B
#define GR_GL_VERTEX_ARRAY_STRIDE                0x807C
#define GR_GL_NORMAL_ARRAY_TYPE                  0x807E
#define GR_GL_NORMAL_ARRAY_STRIDE                0x807F
#define GR_GL_COLOR_ARRAY_SIZE                   0x8081
#define GR_GL_COLOR_ARRAY_TYPE                   0x8082
#define GR_GL_COLOR_ARRAY_STRIDE                 0x8083
#define GR_GL_INDEX_ARRAY_TYPE                   0x8085
#define GR_GL_INDEX_ARRAY_STRIDE                 0x8086
#define GR_GL_TEXTURE_COORD_ARRAY_SIZE           0x8088
#define GR_GL_TEXTURE_COORD_ARRAY_TYPE           0x8089
#define GR_GL_TEXTURE_COORD_ARRAY_STRIDE         0x808A
#define GR_GL_EDGE_FLAG_ARRAY_STRIDE             0x808C
#define GR_GL_VERTEX_ARRAY_POINTER               0x808E
#define GR_GL_NORMAL_ARRAY_POINTER               0x808F
#define GR_GL_COLOR_ARRAY_POINTER                0x8090
#define GR_GL_INDEX_ARRAY_POINTER                0x8091
#define GR_GL_TEXTURE_COORD_ARRAY_POINTER        0x8092
#define GR_GL_EDGE_FLAG_ARRAY_POINTER            0x8093
#define GR_GL_V2F                                0x2A20
#define GR_GL_V3F                                0x2A21
#define GR_GL_C4UB_V2F                           0x2A22
#define GR_GL_C4UB_V3F                           0x2A23
#define GR_GL_C3F_V3F                            0x2A24
#define GR_GL_N3F_V3F                            0x2A25
#define GR_GL_C4F_N3F_V3F                        0x2A26
#define GR_GL_T2F_V3F                            0x2A27
#define GR_GL_T4F_V4F                            0x2A28
#define GR_GL_T2F_C4UB_V3F                       0x2A29
#define GR_GL_T2F_C3F_V3F                        0x2A2A
#define GR_GL_T2F_N3F_V3F                        0x2A2B
#define GR_GL_T2F_C4F_N3F_V3F                    0x2A2C
#define GR_GL_T4F_C4F_N3F_V4F                    0x2A2D
#define GR_GL_PRIMITIVE_RESTART_FIXED_INDEX      0x8D69

/* Buffer Object */
#define GR_GL_READ_ONLY                          0x88B8
#define GR_GL_WRITE_ONLY                         0x88B9
#define GR_GL_READ_WRITE                         0x88BA
#define GR_GL_BUFFER_MAPPED                      0x88BC

#define GR_GL_MAP_READ_BIT                       0x0001
#define GR_GL_MAP_WRITE_BIT                      0x0002
#define GR_GL_MAP_INVALIDATE_RANGE_BIT           0x0004
#define GR_GL_MAP_INVALIDATE_BUFFER_BIT          0x0008
#define GR_GL_MAP_FLUSH_EXPLICIT_BIT             0x0010
#define GR_GL_MAP_UNSYNCHRONIZED_BIT             0x0020

/* Read Format */
#define GR_GL_IMPLEMENTATION_COLOR_READ_TYPE   0x8B9A
#define GR_GL_IMPLEMENTATION_COLOR_READ_FORMAT 0x8B9B

/* Shader Source */
#define GR_GL_COMPILE_STATUS                 0x8B81
#define GR_GL_INFO_LOG_LENGTH                0x8B84
#define GR_GL_SHADER_SOURCE_LENGTH           0x8B88
#define GR_GL_SHADER_COMPILER                0x8DFA

/* Shader Binary */
#define GR_GL_SHADER_BINARY_FORMATS          0x8DF8
#define GR_GL_NUM_SHADER_BINARY_FORMATS      0x8DF9

/* Program Binary */
#define GR_GL_NUM_PROGRAM_BINARY_FORMATS     0x87FE

/* Shader Precision-Specified Types */
#define GR_GL_LOW_FLOAT                      0x8DF0
#define GR_GL_MEDIUM_FLOAT                   0x8DF1
#define GR_GL_HIGH_FLOAT                     0x8DF2
#define GR_GL_LOW_INT                        0x8DF3
#define GR_GL_MEDIUM_INT                     0x8DF4
#define GR_GL_HIGH_INT                       0x8DF5

/* Queries */
#define GR_GL_QUERY_COUNTER_BITS             0x8864
#define GR_GL_CURRENT_QUERY                  0x8865
#define GR_GL_QUERY_RESULT                   0x8866
#define GR_GL_QUERY_RESULT_AVAILABLE         0x8867
#define GR_GL_SAMPLES_PASSED                 0x8914
#define GR_GL_ANY_SAMPLES_PASSED             0x8C2F
#define GR_GL_TIME_ELAPSED                   0x88BF
#define GR_GL_TIMESTAMP                      0x8E28
#define GR_GL_PRIMITIVES_GENERATED           0x8C87
#define GR_GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN 0x8C88


/* Framebuffer Object. */
#define GR_GL_FRAMEBUFFER                    0x8D40
#define GR_GL_READ_FRAMEBUFFER               0x8CA8
#define GR_GL_DRAW_FRAMEBUFFER               0x8CA9

#define GR_GL_RENDERBUFFER                   0x8D41

#define GR_GL_MAX_SAMPLES                    0x8D57
// GL_IMG_multisampled_render_to_texture uses a different value for GL_MAX_SAMPLES
#define GR_GL_MAX_SAMPLES_IMG                0x9135

#define GR_GL_RENDERBUFFER_WIDTH             0x8D42
#define GR_GL_RENDERBUFFER_HEIGHT            0x8D43
#define GR_GL_RENDERBUFFER_INTERNAL_FORMAT   0x8D44
#define GR_GL_RENDERBUFFER_RED_SIZE          0x8D50
#define GR_GL_RENDERBUFFER_GREEN_SIZE        0x8D51
#define GR_GL_RENDERBUFFER_BLUE_SIZE         0x8D52
#define GR_GL_RENDERBUFFER_ALPHA_SIZE        0x8D53
#define GR_GL_RENDERBUFFER_DEPTH_SIZE        0x8D54
#define GR_GL_RENDERBUFFER_STENCIL_SIZE      0x8D55

#define GR_GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE           0x8CD0
#define GR_GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME           0x8CD1
#define GR_GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL         0x8CD2
#define GR_GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE 0x8CD3
#define GR_GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER         0x8CD4
#define GR_GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING        0x8210
#define GR_GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE        0x8211
#define GR_GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE              0x8212
#define GR_GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE            0x8213
#define GR_GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE             0x8214
#define GR_GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE            0x8215
#define GR_GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE            0x8216
#define GR_GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE          0x8217

#define GR_GL_COLOR_ATTACHMENT0              0x8CE0
#define GR_GL_DEPTH_ATTACHMENT               0x8D00
#define GR_GL_STENCIL_ATTACHMENT             0x8D20

// GL_EXT_discard_framebuffer
#define GR_GL_COLOR                          0x1800
#define GR_GL_DEPTH                          0x1801
#define GR_GL_STENCIL                        0x1802

#define GR_GL_NONE                           0
#define GR_GL_FRAMEBUFFER_DEFAULT            0x8218

#define GR_GL_FRAMEBUFFER_COMPLETE                      0x8CD5
#define GR_GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT         0x8CD6
#define GR_GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GR_GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS         0x8CD9
#define GR_GL_FRAMEBUFFER_UNSUPPORTED                   0x8CDD

#define GR_GL_FRAMEBUFFER_BINDING            0x8CA6
#define GR_GL_RENDERBUFFER_BINDING           0x8CA7
#define GR_GL_MAX_RENDERBUFFER_SIZE          0x84E8

#define GR_GL_INVALID_FRAMEBUFFER_OPERATION  0x0506

/* Path Rendering */
// commands
#define GR_GL_CLOSE_PATH                                    0x00
#define GR_GL_MOVE_TO                                       0x02
#define GR_GL_LINE_TO                                       0x04
#define GR_GL_QUADRATIC_CURVE_TO                            0x0A
#define GR_GL_CUBIC_CURVE_TO                                0x0C
#define GR_GL_CONIC_CURVE_TO                                0x1A

// path parameters
#define GR_GL_PATH_STROKE_WIDTH                             0x9075
#define GR_GL_PATH_END_CAPS                                 0x9076
#define GR_GL_PATH_JOIN_STYLE                               0x9079
#define GR_GL_PATH_MITER_LIMIT                              0x907A
#define GR_GL_PATH_STROKE_BOUND                             0x9086

// fill modes
#define GR_GL_COUNT_UP                                      0x9088

// cover mode
#define GR_GL_BOUNDING_BOX                                  0x908D
#define GR_GL_BOUNDING_BOX_OF_BOUNDING_BOXES                0x909C

// transform type
#define GR_GL_TRANSLATE_X                                   0x908E
#define GR_GL_TRANSLATE_Y                                   0x908F
#define GR_GL_TRANSLATE_2D                                  0x9090
#define GR_GL_TRANSPOSE_AFFINE_2D                           0x9096

// cap/dash values
#define GR_GL_SQUARE                                        0x90A3
#define GR_GL_ROUND                                         0x90A4

// join values
#define GR_GL_BEVEL                                         0x90A6
#define GR_GL_MITER_REVERT                                  0x90A7

// glyph loading values
#define GR_GL_STANDARD_FONT_FORMAT                          0x936C
#define GR_GL_FONT_GLYPHS_AVAILABLE                         0x9368

// NV_path_rendering extension to ARB_program_interface_query:
// .. corresponds to the set of active input variables used by the fragment
// shader stage of <program> (if a fragment stage exists).
#define GR_GL_FRAGMENT_INPUT                                0x936D

// NV_path_rendering extension to EXT_direct_state_access:
// [the matrix functions] must support the PATH_PROJECTION_NV and
// PATH_MODELVIEW_NV tokens for matrixMode.
#define GR_GL_PATH_PROJECTION                               0x1701
#define GR_GL_PATH_MODELVIEW                                0x1700

/*  ARM specific define for MSAA support on framebuffer fetch */
#define GR_GL_FETCH_PER_SAMPLE                              0x8F65

/* GL_KHR_debug */
#define GR_GL_DEBUG_OUTPUT                                  0x92E0
#define GR_GL_DEBUG_OUTPUT_SYNCHRONOUS                      0x8242
#define GR_GL_CONTEXT_FLAG_DEBUG_BIT                        0x00000002
#define GR_GL_MAX_DEBUG_MESSAGE_LENGTH                      0x9143
#define GR_GL_MAX_DEBUG_LOGGED_MESSAGES                     0x9144
#define GR_GL_DEBUG_LOGGED_MESSAGES                         0x9145
#define GR_GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH              0x8243
#define GR_GL_MAX_DEBUG_GROUP_STACK_DEPTH                   0x826C
#define GR_GL_DEBUG_GROUP_STACK_DEPTH                       0x826D
#define GR_GL_MAX_LABEL_LENGTH                              0x82E8
#define GR_GL_DEBUG_SOURCE_API                              0x8246
#define GR_GL_DEBUG_SOURCE_WINDOW_SYSTEM                    0x8247
#define GR_GL_DEBUG_SOURCE_SHADER_COMPILER                  0x8248
#define GR_GL_DEBUG_SOURCE_THIRD_PARTY                      0x8249
#define GR_GL_DEBUG_SOURCE_APPLICATION                      0x824A
#define GR_GL_DEBUG_SOURCE_OTHER                            0x824B
#define GR_GL_DEBUG_TYPE_ERROR                              0x824C
#define GR_GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR                0x824D
#define GR_GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR                 0x824E
#define GR_GL_DEBUG_TYPE_PORTABILITY                        0x824F
#define GR_GL_DEBUG_TYPE_PERFORMANCE                        0x8250
#define GR_GL_DEBUG_TYPE_OTHER                              0x8251
#define GR_GL_DEBUG_TYPE_MARKER                             0x8268
#define GR_GL_DEBUG_TYPE_PUSH_GROUP                         0x8269
#define GR_GL_DEBUG_TYPE_POP_GROUP                          0x826A
#define GR_GL_DEBUG_SEVERITY_HIGH                           0x9146
#define GR_GL_DEBUG_SEVERITY_MEDIUM                         0x9147
#define GR_GL_DEBUG_SEVERITY_LOW                            0x9148
#define GR_GL_DEBUG_SEVERITY_NOTIFICATION                   0x826B
#define GR_GL_STACK_UNDERFLOW                               0x0504
#define GR_GL_STACK_OVERFLOW                                0x0503
#define GR_GL_BUFFER                                        0x82E0
#define GR_GL_SHADER                                        0x82E1
#define GR_GL_PROGRAM                                       0x82E2
#define GR_GL_QUERY                                         0x82E3
#define GR_GL_PROGRAM_PIPELINE                              0x82E4
#define GR_GL_SAMPLER                                       0x82E6

/* GL_OES_EGL_image_external */
#define GR_GL_TEXTURE_EXTERNAL                              0x8D65
#define GR_GL_TEXTURE_BINDING_EXTERNAL                      0x8D67

/* GL_ARB_texture_rectangle or GL_ANGLE_texture_rectangle */
#define GR_GL_TEXTURE_RECTANGLE                             0x84F5
#define GR_GL_TEXTURE_BINDING_RECTANGLE                     0x84F6

/* GL_EXT_window_rectangles */
#define GR_GL_MAX_WINDOW_RECTANGLES                         0x8f14
#define GR_GL_INCLUSIVE                                     0x8f10
#define GR_GL_EXCLUSIVE                                     0x8f11

/** GL_QCOM_tiled_rendering */
#define GR_GL_COLOR_BUFFER_BIT0                             0x00000001
#define GR_GL_COLOR_BUFFER_BIT1                             0x00000002
#define GR_GL_COLOR_BUFFER_BIT2                             0x00000004
#define GR_GL_COLOR_BUFFER_BIT3                             0x00000008
#define GR_GL_COLOR_BUFFER_BIT4                             0x00000010
#define GR_GL_COLOR_BUFFER_BIT5                             0x00000020
#define GR_GL_COLOR_BUFFER_BIT6                             0x00000040
#define GR_GL_COLOR_BUFFER_BIT7                             0x00000080
#define GR_GL_DEPTH_BUFFER_BIT0                             0x00000100
#define GR_GL_DEPTH_BUFFER_BIT1                             0x00000200
#define GR_GL_DEPTH_BUFFER_BIT2                             0x00000400
#define GR_GL_DEPTH_BUFFER_BIT3                             0x00000800
#define GR_GL_DEPTH_BUFFER_BIT4                             0x00001000
#define GR_GL_DEPTH_BUFFER_BIT5                             0x00002000
#define GR_GL_DEPTH_BUFFER_BIT6                             0x00004000
#define GR_GL_DEPTH_BUFFER_BIT7                             0x00008000
#define GR_GL_STENCIL_BUFFER_BIT0                           0x00010000
#define GR_GL_STENCIL_BUFFER_BIT1                           0x00020000
#define GR_GL_STENCIL_BUFFER_BIT2                           0x00040000
#define GR_GL_STENCIL_BUFFER_BIT3                           0x00080000
#define GR_GL_STENCIL_BUFFER_BIT4                           0x00100000
#define GR_GL_STENCIL_BUFFER_BIT5                           0x00200000
#define GR_GL_STENCIL_BUFFER_BIT6                           0x00400000
#define GR_GL_STENCIL_BUFFER_BIT7                           0x00800000
#define GR_GL_MULTISAMPLE_BUFFER_BIT0                       0x01000000
#define GR_GL_MULTISAMPLE_BUFFER_BIT1                       0x02000000
#define GR_GL_MULTISAMPLE_BUFFER_BIT2                       0x04000000
#define GR_GL_MULTISAMPLE_BUFFER_BIT3                       0x08000000
#define GR_GL_MULTISAMPLE_BUFFER_BIT4                       0x10000000
#define GR_GL_MULTISAMPLE_BUFFER_BIT5                       0x20000000
#define GR_GL_MULTISAMPLE_BUFFER_BIT6                       0x40000000
#define GR_GL_MULTISAMPLE_BUFFER_BIT7                       0x80000000

/* GL_ARB_sync */
#define GR_GL_SYNC_GPU_COMMANDS_COMPLETE                    0x9117
#define GR_GL_ALREADY_SIGNALED                              0x911A
#define GR_GL_TIMEOUT_EXPIRED                               0x911B
#define GR_GL_CONDITION_SATISFIED                           0x911C
#define GR_GL_WAIT_FAILED                                   0x911D
#define GR_GL_SYNC_FLUSH_COMMANDS_BIT                       0x00000001
#define GR_GL_TIMEOUT_IGNORED                               0xFFFFFFFFFFFFFFFFull

/* GL_EXT_geometry_shader */
#define GR_GL_LINES_ADJACENCY                               0x000A

#define GR_GL_PATCH_VERTICES                                0x8E72

/* GL_ARB_internalformat_query */
#define GR_GL_NUM_SAMPLE_COUNTS                             0x9380

/* EGL Defines */
#define GR_EGL_NO_DISPLAY                                   ((GrEGLDisplay)0)
#define GR_EGL_EXTENSIONS                                   0x3055
#define GR_EGL_GL_TEXTURE_2D                                0x30B1
#define GR_EGL_GL_TEXTURE_LEVEL                             0x30BC
#define GR_EGL_IMAGE_PRESERVED                              0x30D2
#define GR_EGL_FALSE                                        0x0
#define GR_EGL_TRUE                                         0x1
#define GR_EGL_NONE                                         0x3038
#define GR_EGL_NO_IMAGE                                    ((GrEGLImage)0)

/* Programs */
#define GR_GL_PROGRAM_BINARY_RETRIEVABLE_HINT               0x8257
#define GL_PROGRAM_BINARY_LENGTH                            0x8741

/* GL_NV_conservative_raster */
#define GR_GL_CONSERVATIVE_RASTERIZATION                    0x9346

/* Barriers */
#define GR_GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT               0x0001
#define GR_GL_ELEMENT_ARRAY_BARRIER_BIT                     0x0002
#define GR_GL_UNIFORM_BARRIER_BIT                           0x0004
#define GR_GL_TEXTURE_FETCH_BARRIER_BIT                     0x0008
#define GR_GL_SHADER_IMAGE_ACCESS_BARRIER_BIT               0x0020
#define GR_GL_COMMAND_BARRIER_BIT                           0x0040
#define GR_GL_PIXEL_BUFFER_BARRIER_BIT                      0x0080
#define GR_GL_TEXTURE_UPDATE_BARRIER_BIT                    0x0100
#define GR_GL_BUFFER_UPDATE_BARRIER_BIT                     0x0200
#define GR_GL_FRAMEBUFFER_BARRIER_BIT                       0x0400
#define GR_GL_TRANSFORM_FEEDBACK_BARRIER_BIT                0x0800
#define GR_GL_ATOMIC_COUNTER_BARRIER_BIT                    0x1000
#define GR_GL_ALL_BARRIER_BITS                              0xffffffff

/** GL_NV_fence_sync */
#define GR_GL_ALL_COMPLETED                                 0x84F2

#endif
