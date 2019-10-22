/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_types_DEFINED
#define sk_types_DEFINED

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
    #define SK_C_PLUS_PLUS_BEGIN_GUARD    extern "C" {
    #define SK_C_PLUS_PLUS_END_GUARD      }
#else
    #include <stdbool.h>
    #define SK_C_PLUS_PLUS_BEGIN_GUARD
    #define SK_C_PLUS_PLUS_END_GUARD
#endif

#if !defined(SK_C_API)
    #if defined(SKIA_C_DLL)
        #if defined(_MSC_VER)
            #if SKIA_IMPLEMENTATION
                #define SK_C_API __declspec(dllexport)
            #else
                #define SK_C_API __declspec(dllimport)
            #endif
        #else
            #define SK_C_API __attribute__((visibility("default")))
        #endif
    #else
        #define SK_C_API
    #endif
#endif

///////////////////////////////////////////////////////////////////////////////////////

SK_C_PLUS_PLUS_BEGIN_GUARD

typedef struct sk_refcnt_t sk_refcnt_t;
typedef struct sk_nvrefcnt_t sk_nvrefcnt_t;

typedef uint32_t sk_color_t;
typedef uint32_t sk_pmcolor_t;

/* This macro assumes all arguments are >=0 and <=255. */
#define sk_color_set_argb(a, r, g, b)   (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))
#define sk_color_get_a(c)               (((c) >> 24) & 0xFF)
#define sk_color_get_r(c)               (((c) >> 16) & 0xFF)
#define sk_color_get_g(c)               (((c) >>  8) & 0xFF)
#define sk_color_get_b(c)               (((c) >>  0) & 0xFF)

typedef enum {
    UNKNOWN_SK_COLORTYPE = 0,
    ALPHA_8_SK_COLORTYPE,
    RGB_565_SK_COLORTYPE,
    ARGB_4444_SK_COLORTYPE,
    RGBA_8888_SK_COLORTYPE,
    RGB_888X_SK_COLORTYPE,
    BGRA_8888_SK_COLORTYPE,
    RGBA_1010102_SK_COLORTYPE,
    RGB_101010X_SK_COLORTYPE,
    GRAY_8_SK_COLORTYPE,
    RGBA_F16_NORM_SK_COLORTYPE,
    RGBA_F16_SK_COLORTYPE,
    RGBA_F32_SK_COLORTYPE,
} sk_colortype_t;

typedef enum {
    UNKNOWN_SK_ALPHATYPE,
    OPAQUE_SK_ALPHATYPE,
    PREMUL_SK_ALPHATYPE,
    UNPREMUL_SK_ALPHATYPE,
} sk_alphatype_t;

typedef enum {
    UNKNOWN_SK_PIXELGEOMETRY,
    RGB_H_SK_PIXELGEOMETRY,
    BGR_H_SK_PIXELGEOMETRY,
    RGB_V_SK_PIXELGEOMETRY,
    BGR_V_SK_PIXELGEOMETRY,
} sk_pixelgeometry_t;

typedef enum {
    USE_DEVICE_INDEPENDENT_FONTS_SK_SURFACE_PROPS_FLAGS = 1 << 0,
} sk_surfaceprops_flags_t;

typedef struct sk_surfaceprops_t sk_surfaceprops_t;

typedef struct {
    float   x;
    float   y;
} sk_point_t;

typedef sk_point_t sk_vector_t;

typedef struct {
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
} sk_irect_t;

typedef struct {
    float   left;
    float   top;
    float   right;
    float   bottom;
} sk_rect_t;

/**
    The sk_matrix_t struct holds a 3x3 perspective matrix for
    transforming coordinates:

        (X,Y) = T[M]((x,y))
        X = (M[0] * x + M[1] * y + M[2]) / (M[6] * x + M[7] * y + M[8]);
        Y = (M[3] * x + M[4] * y + M[5]) / (M[6] * x + M[7] * y + M[8]);

    Therefore, the identity matrix is

        sk_matrix_t identity = {{1, 0, 0,
                                 0, 1, 0,
                                 0, 0, 1}};

    A matrix that scales by sx and sy is:

        sk_matrix_t scale = {{sx, 0,  0,
                              0,  sy, 0,
                              0,  0,  1}};

    A matrix that translates by tx and ty is:

        sk_matrix_t translate = {{1, 0, tx,
                                  0, 1, ty,
                                  0, 0, 1}};

    A matrix that rotates around the origin by A radians:

        sk_matrix_t rotate = {{cos(A), -sin(A), 0,
                               sin(A),  cos(A), 0,
                               0,       0,      1}};

    Two matrixes can be concatinated by:

         void concat_matrices(sk_matrix_t* dst,
                             const sk_matrix_t* matrixU,
                             const sk_matrix_t* matrixV) {
            const float* u = matrixU->mat;
            const float* v = matrixV->mat;
            sk_matrix_t result = {{
                    u[0] * v[0] + u[1] * v[3] + u[2] * v[6],
                    u[0] * v[1] + u[1] * v[4] + u[2] * v[7],
                    u[0] * v[2] + u[1] * v[5] + u[2] * v[8],
                    u[3] * v[0] + u[4] * v[3] + u[5] * v[6],
                    u[3] * v[1] + u[4] * v[4] + u[5] * v[7],
                    u[3] * v[2] + u[4] * v[5] + u[5] * v[8],
                    u[6] * v[0] + u[7] * v[3] + u[8] * v[6],
                    u[6] * v[1] + u[7] * v[4] + u[8] * v[7],
                    u[6] * v[2] + u[7] * v[5] + u[8] * v[8]
            }};
            *dst = result;
        }
*/
typedef struct {
    float   mat[9];
} sk_matrix_t;

typedef struct sk_matrix44_t sk_matrix44_t;

typedef enum {
    IDENTITY_SK_MATRIX44_TYPE_MASK = 0,
    TRANSLATE_SK_MATRIX44_TYPE_MASK = 0x01,
    SCALE_SK_MATRIX44_TYPE_MASK = 0x02,
    AFFINE_SK_MATRIX44_TYPE_MASK = 0x04,
    PERSPECTIVE_SK_MATRIX44_TYPE_MASK = 0x08 
} sk_matrix44_type_mask_t;

/**
    A sk_canvas_t encapsulates all of the state about drawing into a
    destination This includes a reference to the destination itself,
    and a stack of matrix/clip values.
*/
typedef struct sk_canvas_t sk_canvas_t;
typedef struct sk_nodraw_canvas_t sk_nodraw_canvas_t;
typedef struct sk_nway_canvas_t sk_nway_canvas_t;
typedef struct sk_overdraw_canvas_t sk_overdraw_canvas_t;
/**
    A sk_data_ holds an immutable data buffer.
*/
typedef struct sk_data_t sk_data_t;
/**
    A sk_drawable_t is a abstraction for drawings that changed while
    drawing.
*/
typedef struct sk_drawable_t sk_drawable_t;
/**
    A sk_image_t is an abstraction for drawing a rectagle of pixels.
    The content of the image is always immutable, though the actual
    storage may change, if for example that image can be re-created via
    encoded data or other means.
*/
typedef struct sk_image_t sk_image_t;
/**
    A sk_maskfilter_t is an object that perform transformations on an
    alpha-channel mask before drawing it; it may be installed into a
    sk_paint_t.  Each time a primitive is drawn, it is first
    scan-converted into a alpha mask, which os handed to the
    maskfilter, which may create a new mask is to render into the
    destination.
 */
typedef struct sk_maskfilter_t sk_maskfilter_t;
/**
    A sk_paint_t holds the style and color information about how to
    draw geometries, text and bitmaps.
*/
typedef struct sk_paint_t sk_paint_t;
typedef struct sk_font_t sk_font_t;
/**
    A sk_path_t encapsulates compound (multiple contour) geometric
    paths consisting of straight line segments, quadratic curves, and
    cubic curves.
*/
typedef struct sk_path_t sk_path_t;
/**
    A sk_picture_t holds recorded canvas drawing commands to be played
    back at a later time.
*/
typedef struct sk_picture_t sk_picture_t;
/**
    A sk_picture_recorder_t holds a sk_canvas_t that records commands
    to create a sk_picture_t.
*/
typedef struct sk_picture_recorder_t sk_picture_recorder_t;
/**
    A sk_shader_t specifies the source color(s) for what is being drawn. If a
    paint has no shader, then the paint's color is used. If the paint
    has a shader, then the shader's color(s) are use instead, but they
    are modulated by the paint's alpha.
*/
typedef struct sk_shader_t sk_shader_t;
/**
    A sk_surface_t holds the destination for drawing to a canvas. For
    raster drawing, the destination is an array of pixels in memory.
    For GPU drawing, the destination is a texture or a framebuffer.
*/
typedef struct sk_surface_t sk_surface_t;
/**
    The sk_region encapsulates the geometric region used to specify
    clipping areas for drawing.
*/
typedef struct sk_region_t sk_region_t;

typedef enum {
    CLEAR_SK_BLENDMODE,
    SRC_SK_BLENDMODE,
    DST_SK_BLENDMODE,
    SRCOVER_SK_BLENDMODE,
    DSTOVER_SK_BLENDMODE,
    SRCIN_SK_BLENDMODE,
    DSTIN_SK_BLENDMODE,
    SRCOUT_SK_BLENDMODE,
    DSTOUT_SK_BLENDMODE,
    SRCATOP_SK_BLENDMODE,
    DSTATOP_SK_BLENDMODE,
    XOR_SK_BLENDMODE,
    PLUS_SK_BLENDMODE,
    MODULATE_SK_BLENDMODE,
    SCREEN_SK_BLENDMODE,
    OVERLAY_SK_BLENDMODE,
    DARKEN_SK_BLENDMODE,
    LIGHTEN_SK_BLENDMODE,
    COLORDODGE_SK_BLENDMODE,
    COLORBURN_SK_BLENDMODE,
    HARDLIGHT_SK_BLENDMODE,
    SOFTLIGHT_SK_BLENDMODE,
    DIFFERENCE_SK_BLENDMODE,
    EXCLUSION_SK_BLENDMODE,
    MULTIPLY_SK_BLENDMODE,
    HUE_SK_BLENDMODE,
    SATURATION_SK_BLENDMODE,
    COLOR_SK_BLENDMODE,
    LUMINOSITY_SK_BLENDMODE,
} sk_blendmode_t;

//////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
    float   x;
    float   y;
    float   z;
} sk_point3_t;

typedef struct {
    float   x;
    float   y;
} sk_ipoint_t;

typedef struct {
    float   w;
    float   h;
} sk_size_t;

typedef struct {
    float   w;
    float   h;
} sk_isize_t;

typedef struct {
    uint32_t fFlags;
    float    fTop;
    float    fAscent;
    float    fDescent;
    float    fBottom;
    float    fLeading;
    float    fAvgCharWidth;
    float    fMaxCharWidth;
    float    fXMin;
    float    fXMax;
    float    fXHeight;
    float    fCapHeight;
    float    fUnderlineThickness;
    float    fUnderlinePosition;
    float    fStrikeoutThickness;
    float    fStrikeoutPosition;
} sk_fontmetrics_t;

// Flags for fFlags member of sk_fontmetrics_t
#define FONTMETRICS_FLAGS_UNDERLINE_THICKNESS_IS_VALID (1U << 0)
#define FONTMETRICS_FLAGS_UNDERLINE_POSITION_IS_VALID (1U << 1)

/**
    A lightweight managed string.
*/
typedef struct sk_string_t sk_string_t;
/**

    A sk_bitmap_t is an abstraction that specifies a raster bitmap.
*/
typedef struct sk_bitmap_t sk_bitmap_t;
typedef struct sk_pixmap_t sk_pixmap_t;
typedef struct sk_colorfilter_t sk_colorfilter_t;
typedef struct sk_imagefilter_t sk_imagefilter_t;
typedef struct sk_imagefilter_croprect_t sk_imagefilter_croprect_t;

/**
   A sk_typeface_t pecifies the typeface and intrinsic style of a font.
    This is used in the paint, along with optionally algorithmic settings like
    textSize, textSkewX, textScaleX, kFakeBoldText_Mask, to specify
    how text appears when drawn (and measured).

    Typeface objects are immutable, and so they can be shared between threads.
*/
typedef struct sk_typeface_t sk_typeface_t;
typedef uint32_t sk_font_table_tag_t;
typedef struct sk_fontmgr_t sk_fontmgr_t;
typedef struct sk_fontstyle_t sk_fontstyle_t;
typedef struct sk_fontstyleset_t sk_fontstyleset_t;
/**
 *  Abstraction layer directly on top of an image codec.
 */
typedef struct sk_codec_t sk_codec_t;
typedef struct sk_colorspace_t sk_colorspace_t;
/**
   Various stream types
*/
typedef struct sk_stream_t sk_stream_t;
typedef struct sk_stream_filestream_t sk_stream_filestream_t;
typedef struct sk_stream_asset_t sk_stream_asset_t;
typedef struct sk_stream_memorystream_t sk_stream_memorystream_t;
typedef struct sk_stream_streamrewindable_t sk_stream_streamrewindable_t;
typedef struct sk_wstream_t sk_wstream_t;
typedef struct sk_wstream_filestream_t sk_wstream_filestream_t;
typedef struct sk_wstream_dynamicmemorystream_t sk_wstream_dynamicmemorystream_t;
/**
   High-level API for creating a document-based canvas.
*/
typedef struct sk_document_t sk_document_t;

typedef enum {
    POINTS_SK_POINT_MODE,
    LINES_SK_POINT_MODE,
    POLYGON_SK_POINT_MODE
} sk_point_mode_t;

typedef enum {
    LEFT_SK_TEXT_ALIGN,
    CENTER_SK_TEXT_ALIGN,
    RIGHT_SK_TEXT_ALIGN
} sk_text_align_t;

typedef enum {
    UTF8_SK_TEXT_ENCODING,
    UTF16_SK_TEXT_ENCODING,
    UTF32_SK_TEXT_ENCODING,
    GLYPH_ID_SK_TEXT_ENCODING
} sk_text_encoding_t;

typedef enum {
    WINDING_SK_PATH_FILLTYPE,
    EVENODD_SK_PATH_FILLTYPE,
    INVERSE_WINDING_SK_PATH_FILLTYPE,
    INVERSE_EVENODD_SK_PATH_FILLTYPE,
} sk_path_filltype_t;

typedef enum {
    UPRIGHT_SK_FONT_STYLE_SLANT = 0,
    ITALIC_SK_FONT_STYLE_SLANT  = 1,
    OBLIQUE_SK_FONT_STYLE_SLANT = 2,
} sk_font_style_slant_t;

typedef enum {
    NONE_SK_FILTER_QUALITY,
    LOW_SK_FILTER_QUALITY,
    MEDIUM_SK_FILTER_QUALITY,
    HIGH_SK_FILTER_QUALITY
} sk_filter_quality_t;

typedef enum {
    HAS_LEFT_SK_CROP_RECT_FLAG   = 0x01,
    HAS_TOP_SK_CROP_RECT_FLAG    = 0x02,
    HAS_WIDTH_SK_CROP_RECT_FLAG  = 0x04,
    HAS_HEIGHT_SK_CROP_RECT_FLAG = 0x08,
    HAS_ALL_SK_CROP_RECT_FLAG    = 0x0F,
} sk_crop_rect_flags_t;

typedef enum {
    DRAW_SHADOW_AND_FOREGROUND_SK_DROP_SHADOW_IMAGE_FILTER_SHADOW_MODE,
    DRAW_SHADOW_ONLY_SK_DROP_SHADOW_IMAGE_FILTER_SHADOW_MODE,
} sk_drop_shadow_image_filter_shadow_mode_t;

typedef enum {
    UNKNOWN_SK_DISPLACEMENT_MAP_EFFECT_CHANNEL_SELECTOR_TYPE,
    R_SK_DISPLACEMENT_MAP_EFFECT_CHANNEL_SELECTOR_TYPE,
    G_SK_DISPLACEMENT_MAP_EFFECT_CHANNEL_SELECTOR_TYPE,
    B_SK_DISPLACEMENT_MAP_EFFECT_CHANNEL_SELECTOR_TYPE,
    A_SK_DISPLACEMENT_MAP_EFFECT_CHANNEL_SELECTOR_TYPE,
} sk_displacement_map_effect_channel_selector_type_t;

typedef enum {
    CLAMP_SK_MATRIX_CONVOLUTION_TILEMODE,
    REPEAT_SK_MATRIX_CONVOLUTION_TILEMODE,
    CLAMP_TO_BLACK_SK_MATRIX_CONVOLUTION_TILEMODE,
} sk_matrix_convolution_tilemode_t;

/**
    The logical operations that can be performed when combining two regions.
*/
typedef enum {
    DIFFERENCE_SK_REGION_OP,          //!< subtract the op region from the first region
    INTERSECT_SK_REGION_OP,           //!< intersect the two regions
    UNION_SK_REGION_OP,               //!< union (inclusive-or) the two regions
    XOR_SK_REGION_OP,                 //!< exclusive-or the two regions
    REVERSE_DIFFERENCE_SK_REGION_OP,  //!< subtract the first region from the op region
    REPLACE_SK_REGION_OP,             //!< replace the dst region with the op region
} sk_region_op_t;

typedef enum {
    DIFFERENCE_SK_CLIPOP,
    INTERSECT_SK_CLIPOP,
} sk_clipop_t;

/**
 *  Enum describing format of encoded data.
 */
typedef enum {
    BMP_SK_ENCODED_FORMAT,
    GIF_SK_ENCODED_FORMAT,
    ICO_SK_ENCODED_FORMAT,
    JPEG_SK_ENCODED_FORMAT,
    PNG_SK_ENCODED_FORMAT,
    WBMP_SK_ENCODED_FORMAT,
    WEBP_SK_ENCODED_FORMAT,
    PKM_SK_ENCODED_FORMAT,
    KTX_SK_ENCODED_FORMAT,
    ASTC_SK_ENCODED_FORMAT,
    DNG_SK_ENCODED_FORMAT,
    HEIF_SK_ENCODED_FORMAT,
} sk_encoded_image_format_t;

typedef enum {
    TOP_LEFT_SK_ENCODED_ORIGIN     = 1, // Default
    TOP_RIGHT_SK_ENCODED_ORIGIN    = 2, // Reflected across y-axis
    BOTTOM_RIGHT_SK_ENCODED_ORIGIN = 3, // Rotated 180
    BOTTOM_LEFT_SK_ENCODED_ORIGIN  = 4, // Reflected across x-axis
    LEFT_TOP_SK_ENCODED_ORIGIN     = 5, // Reflected across x-axis, Rotated 90 CCW
    RIGHT_TOP_SK_ENCODED_ORIGIN    = 6, // Rotated 90 CW
    RIGHT_BOTTOM_SK_ENCODED_ORIGIN = 7, // Reflected across x-axis, Rotated 90 CW
    LEFT_BOTTOM_SK_ENCODED_ORIGIN  = 8, // Rotated 90 CCW
    DEFAULT_SK_ENCODED_ORIGIN      = TOP_LEFT_SK_ENCODED_ORIGIN,
} sk_encodedorigin_t;

typedef enum {
    SUCCESS_SK_CODEC_RESULT,
    INCOMPLETE_INPUT_SK_CODEC_RESULT,
    ERROR_IN_INPUT_SK_CODEC_RESULT,
    INVALID_CONVERSION_SK_CODEC_RESULT,
    INVALID_SCALE_SK_CODEC_RESULT,
    INVALID_PARAMETERS_SK_CODEC_RESULT,
    INVALID_INPUT_SK_CODEC_RESULT,
    COULD_NOT_REWIND_SK_CODEC_RESULT,
    INTERNAL_ERROR_SK_CODEC_RESULT,
    UNIMPLEMENTED_SK_CODEC_RESULT,
} sk_codec_result_t;

typedef enum {
    YES_SK_CODEC_ZERO_INITIALIZED,
    NO_SK_CODEC_ZERO_INITIALIZED,
} sk_codec_zero_initialized_t;

typedef struct {
    sk_codec_zero_initialized_t fZeroInitialized;
    sk_irect_t* fSubset;
    int fFrameIndex;
    int fPriorFrame;
} sk_codec_options_t;

typedef enum {
    TOP_DOWN_SK_CODEC_SCANLINE_ORDER,
    BOTTOM_UP_SK_CODEC_SCANLINE_ORDER,
} sk_codec_scanline_order_t;

// The verbs that can be foudn on a path
typedef enum {
    MOVE_SK_PATH_VERB,
    LINE_SK_PATH_VERB,
    QUAD_SK_PATH_VERB,
    CONIC_SK_PATH_VERB,
    CUBIC_SK_PATH_VERB,
    CLOSE_SK_PATH_VERB,
    DONE_SK_PATH_VERB
} sk_path_verb_t;

typedef struct sk_path_iterator_t sk_path_iterator_t;
typedef struct sk_path_rawiterator_t sk_path_rawiterator_t;

typedef enum {
    APPEND_SK_PATH_ADD_MODE,
    EXTEND_SK_PATH_ADD_MODE,
} sk_path_add_mode_t;

typedef enum {
    LINE_SK_PATH_SEGMENT_MASK  = 1 << 0,
    QUAD_SK_PATH_SEGMENT_MASK  = 1 << 1,
    CONIC_SK_PATH_SEGMENT_MASK = 1 << 2,
    CUBIC_SK_PATH_SEGMENT_MASK = 1 << 3,
} sk_path_segment_mask_t;

typedef enum {
    TRANSLATE_SK_PATH_EFFECT_1D_STYLE,
    ROTATE_SK_PATH_EFFECT_1D_STYLE,
    MORPH_SK_PATH_EFFECT_1D_STYLE,
} sk_path_effect_1d_style_t;

typedef enum {
    NORMAL_SK_PATH_EFFECT_TRIM_MODE,
    INVERTED_SK_PATH_EFFECT_TRIM_MODE,
} sk_path_effect_trim_mode_t;

typedef struct sk_path_effect_t sk_path_effect_t;  

typedef enum {
    BUTT_SK_STROKE_CAP,
    ROUND_SK_STROKE_CAP,
    SQUARE_SK_STROKE_CAP
} sk_stroke_cap_t;

typedef enum {
    MITER_SK_STROKE_JOIN,
    ROUND_SK_STROKE_JOIN,
    BEVEL_SK_STROKE_JOIN
} sk_stroke_join_t;

typedef enum {
    CLAMP_SK_SHADER_TILEMODE,
    REPEAT_SK_SHADER_TILEMODE,
    MIRROR_SK_SHADER_TILEMODE,
    DECAL_SK_SHADER_TILEMODE,
} sk_shader_tilemode_t;

typedef enum {
    NORMAL_SK_BLUR_STYLE,   //!< fuzzy inside and outside
    SOLID_SK_BLUR_STYLE,    //!< solid inside, fuzzy outside
    OUTER_SK_BLUR_STYLE,    //!< nothing inside, fuzzy outside
    INNER_SK_BLUR_STYLE,    //!< fuzzy inside, nothing outside
} sk_blurstyle_t;

typedef enum {
    CW_SK_PATH_DIRECTION,
    CCW_SK_PATH_DIRECTION,
} sk_path_direction_t;

typedef enum {
    SMALL_SK_PATH_ARC_SIZE,
    LARGE_SK_PATH_ARC_SIZE,
} sk_path_arc_size_t;

typedef enum {
    FILL_SK_PAINT_STYLE,
    STROKE_SK_PAINT_STYLE,
    STROKE_AND_FILL_SK_PAINT_STYLE,
} sk_paint_style_t;

typedef enum {
    NONE_SK_FONT_HINTING,
    SLIGHT_SK_FONT_HINTING,
    NORMAL_SK_FONT_HINTING,
    FULL_SK_FONT_HINTING,
} sk_font_hinting_t;

typedef enum {
    ALIAS_SK_FONT_EDGING,
    ANTIALIAS_SK_FONT_EDGING,
    SUBPIXEL_ANTIALIAS_SK_FONT_EDGING,
} sk_font_edging_t;

typedef struct sk_colortable_t sk_colortable_t;

typedef struct sk_pixelref_factory_t sk_pixelref_factory_t;

typedef enum {
    TOP_LEFT_GR_SURFACE_ORIGIN,
    BOTTOM_LEFT_GR_SURFACE_ORIGIN,
} gr_surfaceorigin_t;

typedef enum {
    UNKNOWN_GR_PIXEL_CONFIG,
    ALPHA_8_GR_PIXEL_CONFIG,
    ALPHA_8_AS_ALPHA_GR_PIXEL_CONFIG,
    ALPHA_8_AS_RED_GR_PIXEL_CONFIG,
    GRAY_8_GR_PIXEL_CONFIG,
    GRAY_8_AS_LUM_GR_PIXEL_CONFIG,
    GRAY_8_AS_RED_GR_PIXEL_CONFIG,
    RGB_565_GR_PIXEL_CONFIG,
    RGBA_4444_GR_PIXEL_CONFIG,
    RGBA_8888_GR_PIXEL_CONFIG,
    RGB_888_GR_PIXEL_CONFIG,
    RGB_888X_GR_PIXEL_CONFIG,
    RG_88_GR_PIXEL_CONFIG,
    BGRA_8888_GR_PIXEL_CONFIG,
    SRGBA_8888_GR_PIXEL_CONFIG,
    RGBA_1010102_GR_PIXEL_CONFIG,
    ALPHA_HALF_GR_PIXEL_CONFIG,
    ALPHA_HALF_AS_LUM_GR_PIXEL_CONFIG,
    ALPHA_HALF_AS_RED_GR_PIXEL_CONFIG,
    RGBA_HALF_GR_PIXEL_CONFIG,
    RGBA_HALF_CLAMPED_GR_PIXEL_CONFIG,
    RGB_ETC1_GR_PIXEL_CONFIG,
    ALPHA_16_GR_PIXEL_CONFIG,
    RG_1616_GR_PIXEL_CONFIG,

    // Experimental (for Y416 and mutant P016/P010)
    RGBA_16161616_GR_PIXEL_CONFIG,
    RG_HALF_GR_PIXEL_CONFIG,
} gr_pixelconfig_t;

typedef enum {
    BW_SK_MASK_FORMAT,             //!< 1bit per pixel mask (e.g. monochrome)
    A8_SK_MASK_FORMAT,             //!< 8bits per pixel mask (e.g. antialiasing)
    THREE_D_SK_MASK_FORMAT,        //!< 3 8bit per pixl planes: alpha, mul, add
    ARGB32_SK_MASK_FORMAT,         //!< SkPMColor
    LCD16_SK_MASK_FORMAT,          //!< 565 alpha for r/g/b
} sk_mask_format_t;

typedef struct {
    uint8_t*          fImage;
    sk_irect_t        fBounds;
    uint32_t          fRowBytes;
    sk_mask_format_t  fFormat;
} sk_mask_t;

typedef enum {
    NONE_GR_CONTEXT_FLUSHBITS = 0,
    DISCARD_GR_CONTEXT_FLUSHBITS = 0x2,
} gr_context_flushbits_t;

typedef intptr_t gr_backendobject_t;

typedef struct gr_backendrendertarget_t gr_backendrendertarget_t;
typedef struct gr_backendtexture_t gr_backendtexture_t;

typedef struct gr_context_t gr_context_t;

typedef enum {
    METAL_GR_BACKEND,
    DAWN_GR_BACKEND,
    OPENGL_GR_BACKEND,
    VULKAN_GR_BACKEND,
} gr_backend_t;

typedef intptr_t gr_backendcontext_t;

typedef struct gr_glinterface_t gr_glinterface_t;

typedef void (*gr_gl_func_ptr)(void);
typedef gr_gl_func_ptr (*gr_gl_get_proc)(void* ctx, const char* name);

typedef struct {
    unsigned int fTarget;
    unsigned int fID;
    unsigned int fFormat;
} gr_gl_textureinfo_t;

typedef struct {
    unsigned int fFBOID;
    unsigned int fFormat;
} gr_gl_framebufferinfo_t;

typedef enum {
    DIFFERENCE_SK_PATHOP,
    INTERSECT_SK_PATHOP,
    UNION_SK_PATHOP,
    XOR_SK_PATHOP,
    REVERSE_DIFFERENCE_SK_PATHOP,
} sk_pathop_t;

typedef struct sk_opbuilder_t sk_opbuilder_t;

typedef enum {
    UNKNOWN_SK_PATH_CONVEXITY,
    CONVEX_SK_PATH_CONVEXITY,
    CONCAVE_SK_PATH_CONVEXITY,
} sk_path_convexity_t;

typedef enum {
    DEFAULT_SK_LATTICE_RECT_TYPE,
    TRANSPARENT_SK_LATTICE_RECT_TYPE,
    FIXED_COLOR_SK_LATTICE_RECT_TYPE,
} sk_lattice_recttype_t;

typedef struct {
    const int* fXDivs;
    const int* fYDivs;
    const sk_lattice_recttype_t* fRectTypes;
    int fXCount;
    int fYCount;
    const sk_irect_t* fBounds;
    const sk_color_t* fColors;
} sk_lattice_t;

typedef struct sk_pathmeasure_t sk_pathmeasure_t;

typedef enum {
    GET_POSITION_SK_PATHMEASURE_MATRIXFLAGS = 0x01,
    GET_TANGENT_SK_PATHMEASURE_MATRIXFLAGS = 0x02,
    GET_POS_AND_TAN_SK_PATHMEASURE_MATRIXFLAGS = GET_POSITION_SK_PATHMEASURE_MATRIXFLAGS | GET_TANGENT_SK_PATHMEASURE_MATRIXFLAGS,
} sk_pathmeasure_matrixflags_t;

typedef void (*sk_bitmap_release_proc)(void* addr, void* context);

typedef void (*sk_data_release_proc)(const void* ptr, void* context);

typedef void (*sk_image_raster_release_proc)(const void* addr, void* context);
typedef void (*sk_image_texture_release_proc)(void* context);

typedef void (*sk_surface_raster_release_proc)(void* addr, void* context);

typedef enum {
    ALLOW_SK_IMAGE_CACHING_HINT,
    DISALLOW_SK_IMAGE_CACHING_HINT,
} sk_image_caching_hint_t;

typedef enum {
    ZERO_PIXELS_SK_BITMAP_ALLOC_FLAGS = 1 << 0,
} sk_bitmap_allocflags_t;

typedef struct {
    int16_t  fTimeZoneMinutes;
    uint16_t fYear;
    uint8_t  fMonth;
    uint8_t  fDayOfWeek;
    uint8_t  fDay;
    uint8_t  fHour;
    uint8_t  fMinute;
    uint8_t  fSecond;
} sk_time_datetime_t;

typedef struct {
    sk_string_t*        fTitle;
    sk_string_t*        fAuthor;
    sk_string_t*        fSubject;
    sk_string_t*        fKeywords;
    sk_string_t*        fCreator;
    sk_string_t*        fProducer;
    sk_time_datetime_t* fCreation;
    sk_time_datetime_t* fModified;
    float               fRasterDPI;
    bool                fPDFA;
    int                 fEncodingQuality;
} sk_document_pdf_metadata_t;

typedef struct {
    sk_colorspace_t* colorspace;
    int32_t          width;
    int32_t          height;
    sk_colortype_t   colorType;
    sk_alphatype_t   alphaType;
} sk_imageinfo_t;

typedef enum {
    KEEP_SK_CODEC_ANIMATION_DISPOSAL_METHOD               = 1,
    RESTORE_BG_COLOR_SK_CODEC_ANIMATION_DISPOSAL_METHOD   = 2,
    RESTORE_PREVIOUS_SK_CODEC_ANIMATION_DISPOSAL_METHOD   = 3,
} sk_codecanimation_disposalmethod_t;

typedef struct {
    int fRequiredFrame;
    int fDuration;
    bool fFullyReceived;
    sk_alphatype_t fAlphaType;
    sk_codecanimation_disposalmethod_t fDisposalMethod;
} sk_codec_frameinfo_t;

typedef struct sk_xmlstreamwriter_t sk_xmlstreamwriter_t;
typedef struct sk_xmlwriter_t sk_xmlwriter_t;

typedef struct sk_svgcanvas_t sk_svgcanvas_t;

typedef struct sk_3dview_t sk_3dview_t;

typedef enum {
    TRIANGLES_SK_VERTICES_VERTEX_MODE,
    TRIANGLE_STRIP_SK_VERTICES_VERTEX_MODE,
    TRIANGLE_FAN_SK_VERTICES_VERTEX_MODE,
} sk_vertices_vertex_mode_t;

typedef struct sk_vertices_t sk_vertices_t;

typedef enum {
    SRGB_SK_NAMED_TRANSFER_FN,
    TWO_DOT_TWO_SK_NAMED_TRANSFER_FN,
    LINEAR_SK_NAMED_TRANSFER_FN,
    REC2020_SK_NAMED_TRANSFER_FN,
} sk_named_transfer_fn_t;

typedef enum {
    SRGB_SK_NAMED_GAMUT,
    ADOBE_RGB_SK_NAMED_GAMUT,
    DCIP3_D65_SK_NAMED_GAMUT,
    REC2020_SK_NAMED_GAMUT,
} sk_named_gamut_t;

typedef struct {
    float fRX;
    float fRY;
    float fGX;
    float fGY;
    float fBX;
    float fBY;
    float fWX;
    float fWY;
} sk_colorspace_primaries_t;

typedef enum {
    NO_INVERT_SK_HIGH_CONTRAST_CONFIG_INVERT_STYLE,
    INVERT_BRIGHTNESS_SK_HIGH_CONTRAST_CONFIG_INVERT_STYLE,
    INVERT_LIGHTNESS_SK_HIGH_CONTRAST_CONFIG_INVERT_STYLE,
} sk_highcontrastconfig_invertstyle_t;

typedef struct {
    bool fGrayscale;
    sk_highcontrastconfig_invertstyle_t fInvertStyle;
    float fContrast;
} sk_highcontrastconfig_t;

typedef enum {
    ZERO_SK_PNGENCODER_FILTER_FLAGS  = 0x00,
    NONE_SK_PNGENCODER_FILTER_FLAGS  = 0x08,
    SUB_SK_PNGENCODER_FILTER_FLAGS   = 0x10,
    UP_SK_PNGENCODER_FILTER_FLAGS    = 0x20,
    AVG_SK_PNGENCODER_FILTER_FLAGS   = 0x40,
    PAETH_SK_PNGENCODER_FILTER_FLAGS = 0x80,
    ALL_SK_PNGENCODER_FILTER_FLAGS   = NONE_SK_PNGENCODER_FILTER_FLAGS |
                                       SUB_SK_PNGENCODER_FILTER_FLAGS |
                                       UP_SK_PNGENCODER_FILTER_FLAGS | 
                                       AVG_SK_PNGENCODER_FILTER_FLAGS | 
                                       PAETH_SK_PNGENCODER_FILTER_FLAGS,
} sk_pngencoder_filterflags_t;

typedef struct {
    sk_pngencoder_filterflags_t fFilterFlags;
    int fZLibLevel;
    void* fComments;
} sk_pngencoder_options_t;

typedef enum {
    DOWNSAMPLE_420_SK_JPEGENCODER_DOWNSAMPLE,
    DOWNSAMPLE_422_SK_JPEGENCODER_DOWNSAMPLE,
    DOWNSAMPLE_444_SK_JPEGENCODER_DOWNSAMPLE,
} sk_jpegencoder_downsample_t;

typedef enum {
    IGNORE_SK_JPEGENCODER_ALPHA_OPTION,
    BLEND_ON_BLACK_SK_JPEGENCODER_ALPHA_OPTION,
} sk_jpegencoder_alphaoption_t;

typedef struct {
    int fQuality;
    sk_jpegencoder_downsample_t fDownsample;
    sk_jpegencoder_alphaoption_t fAlphaOption;
} sk_jpegencoder_options_t;

typedef enum {
    LOSSY_SK_WEBPENCODER_COMPTRESSION,
    LOSSLESS_SK_WEBPENCODER_COMPTRESSION,
} sk_webpencoder_compression_t;

typedef struct {
    sk_webpencoder_compression_t fCompression;
    float fQuality;
} sk_webpencoder_options_t;

typedef struct sk_rrect_t sk_rrect_t;

typedef enum {
    EMPTY_SK_RRECT_TYPE,
    RECT_SK_RRECT_TYPE,
    OVAL_SK_RRECT_TYPE,
    SIMPLE_SK_RRECT_TYPE,
    NINE_PATCH_SK_RRECT_TYPE,
    COMPLEX_SK_RRECT_TYPE,
} sk_rrect_type_t;

typedef enum {
    UPPER_LEFT_SK_RRECT_CORNER,
    UPPER_RIGHT_SK_RRECT_CORNER,
    LOWER_RIGHT_SK_RRECT_CORNER,
    LOWER_LEFT_SK_RRECT_CORNER,
} sk_rrect_corner_t;

typedef struct sk_textblob_t sk_textblob_t;
typedef struct sk_textblob_builder_t sk_textblob_builder_t;

typedef struct {
    void* glyphs;
    void* pos;
    void* utf8text;
    void* clusters;
} sk_textblob_builder_runbuffer_t;

SK_C_PLUS_PLUS_END_GUARD

#endif
