/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

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

#ifndef SK_API
#define SK_API
#endif

///////////////////////////////////////////////////////////////////////////////////////

SK_C_PLUS_PLUS_BEGIN_GUARD

typedef uint32_t sk_color_t;

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
    BGRA_8888_SK_COLORTYPE,
    Index_8_SK_COLORTYPE,
    Gray_8_SK_COLORTYPE,
    RGBA_F16_SK_COLORTYPE,
} sk_colortype_t;

typedef enum {
    UNKNOWN_SK_ALPHATYPE,
    OPAQUE_SK_ALPHATYPE,
    PREMUL_SK_ALPHATYPE,
    UNPREMUL_SK_ALPHATYPE,
} sk_alphatype_t;

typedef enum {
    INTERSECT_SK_CLIPTYPE,
    DIFFERENCE_SK_CLIPTYPE,
} sk_cliptype_t;

typedef enum {
    UNKNOWN_SK_PIXELGEOMETRY,
    RGB_H_SK_PIXELGEOMETRY,
    BGR_H_SK_PIXELGEOMETRY,
    RGB_V_SK_PIXELGEOMETRY,
    BGR_V_SK_PIXELGEOMETRY,
} sk_pixelgeometry_t;

typedef struct {
    int32_t         width;
    int32_t         height;
    sk_colortype_t  colorType;
    sk_alphatype_t  alphaType;
} sk_imageinfo_t;

typedef struct {
    sk_pixelgeometry_t pixelGeometry;
} sk_surfaceprops_t;

typedef struct {
    float   x;
    float   y;
} sk_point_t;

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

typedef struct {
    float   mat[9];
} sk_matrix_t;

/**
    A sk_canvas_t encapsulates all of the state about drawing into a
    destination This includes a reference to the destination itself,
    and a stack of matrix/clip values.
*/
typedef struct sk_canvas_t sk_canvas_t;
/**
    A sk_data_ holds an immutable data buffer.
*/
typedef struct sk_data_t sk_data_t;
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

typedef enum {
    CLEAR_SK_XFERMODE_MODE,
    SRC_SK_XFERMODE_MODE,
    DST_SK_XFERMODE_MODE,
    SRCOVER_SK_XFERMODE_MODE,
    DSTOVER_SK_XFERMODE_MODE,
    SRCIN_SK_XFERMODE_MODE,
    DSTIN_SK_XFERMODE_MODE,
    SRCOUT_SK_XFERMODE_MODE,
    DSTOUT_SK_XFERMODE_MODE,
    SRCATOP_SK_XFERMODE_MODE,
    DSTATOP_SK_XFERMODE_MODE,
    XOR_SK_XFERMODE_MODE,
    PLUS_SK_XFERMODE_MODE,
    MODULATE_SK_XFERMODE_MODE,
    SCREEN_SK_XFERMODE_MODE,
    OVERLAY_SK_XFERMODE_MODE,
    DARKEN_SK_XFERMODE_MODE,
    LIGHTEN_SK_XFERMODE_MODE,
    COLORDODGE_SK_XFERMODE_MODE,
    COLORBURN_SK_XFERMODE_MODE,
    HARDLIGHT_SK_XFERMODE_MODE,
    SOFTLIGHT_SK_XFERMODE_MODE,
    DIFFERENCE_SK_XFERMODE_MODE,
    EXCLUSION_SK_XFERMODE_MODE,
    MULTIPLY_SK_XFERMODE_MODE,
    HUE_SK_XFERMODE_MODE,
    SATURATION_SK_XFERMODE_MODE,
    COLOR_SK_XFERMODE_MODE,
    LUMINOSITY_SK_XFERMODE_MODE,
} sk_xfermode_mode_t;

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
    uint32_t fFlags;            // Bit field to identify which values are unknown
    float    fTop;              // The greatest distance above the baseline for any glyph (will be <= 0)
    float    fAscent;           // The recommended distance above the baseline (will be <= 0)
    float    fDescent;          // The recommended distance below the baseline (will be >= 0)
    float    fBottom;           // The greatest distance below the baseline for any glyph (will be >= 0)
    float    fLeading;          // The recommended distance to add between lines of text (will be >= 0)
    float    fAvgCharWidth;     // the average character width (>= 0)
    float    fMaxCharWidth;     // the max character width (>= 0)
    float    fXMin;             // The minimum bounding box x value for all glyphs
    float    fXMax;             // The maximum bounding box x value for all glyphs
    float    fXHeight;          // The height of an 'x' in px, or 0 if no 'x' in face
    float    fCapHeight;        // The cap height (> 0), or 0 if cannot be determined.
    float    fUnderlineThickness; // underline thickness, or 0 if cannot be determined
    float    fUnderlinePosition; // underline position, or 0 if cannot be determined
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
    UTF8_ENCODING,
    UTF16_ENCODING,
    UTF32_ENCODING
} sk_encoding_t;

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
    NORMAL_TYPEFACE_STYLE      = 0,
    BOLD_TYPEFACE_STYLE        = 0x01,
    ITALIC_TYPEFACE_STYLE      = 0x02,
    BOLD_ITALIC_TYPEFACE_STYLE = 0x03
} sk_typeface_style_t;

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
    UNKNOWN_SK_IMAGE_ENCODER_TYPE,
    BMP_SK_IMAGE_ENCODER_TYPE,
    GIF_SK_IMAGE_ENCODER_TYPE,
    ICO_SK_IMAGE_ENCODER_TYPE,
    JPEG_SK_IMAGE_ENCODER_TYPE,
    PNG_SK_IMAGE_ENCODER_TYPE,
    WBMP_SK_IMAGE_ENCODER_TYPE,
    WEBP_SK_IMAGE_ENCODER_TYPE,
    KTX_SK_IMAGE_ENCODER_TYPE,
} sk_image_encoder_t;

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

/**
 *  Enum describing format of encoded data.
 */
typedef enum {
    UNKNOWN_SK_ENCODED_FORMAT,
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
    DNG_SK_ENCODED_FORMAT
} sk_encoded_format_t;

typedef enum {
    TOP_LEFT_SK_CODEC_ORIGIN     = 1, // Default
    TOP_RIGHT_SK_CODEC_ORIGIN    = 2, // Reflected across y-axis
    BOTTOM_RIGHT_SK_CODEC_ORIGIN = 3, // Rotated 180
    BOTTOM_LEFT_SK_CODEC_ORIGIN  = 4, // Reflected across x-axis
    LEFT_TOP_SK_CODEC_ORIGIN     = 5, // Reflected across x-axis, Rotated 90 CCW
    RIGHT_TOP_SK_CODEC_ORIGIN    = 6, // Rotated 90 CW
    RIGHT_BOTTOM_SK_CODEC_ORIGIN = 7, // Reflected across x-axis, Rotated 90 CW
    LEFT_BOTTOM_SK_CODEC_ORIGIN  = 8, // Rotated 90 CCW
} sk_codec_origin_t;

typedef enum {
    SUCCESS_SK_CODEC_RESULT,
    INCOMPLETE_INPUT_SK_CODEC_RESULT,
    INVALID_CONVERSION_SK_CODEC_RESULT,
    INVALID_SCALE_SK_CODEC_RESULT,
    INVALID_PARAMETERS_SK_CODEC_RESULT,
    INVALID_INPUT_SK_CODEC_RESULT,
    COULD_NOT_REWIND_SK_CODEC_RESULT,
    UNIMPLEMENTED_SK_CODEC_RESULT,
} sk_codec_result_t;

typedef enum {
    YES_SK_CODEC_ZERO_INITIALIZED,
    NO_SK_CODEC_ZERO_INITIALIZED,
} sk_codec_zero_initialized_t;

typedef struct {
    sk_codec_zero_initialized_t fZeroInitialized;
    sk_irect_t fSubset;
    bool fHasSubset;
} sk_codec_options_t;

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
    TRANSLATE_SK_PATH_EFFECT_1D_STYLE,
    ROTATE_SK_PATH_EFFECT_1D_STYLE,
    MORPH_SK_PATH_EFFECT_1D_STYLE,
} sk_path_effect_1d_style_t;

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

SK_C_PLUS_PLUS_END_GUARD

#endif
