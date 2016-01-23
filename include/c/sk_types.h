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
    UNKNOWN_SK_COLORTYPE,
    RGBA_8888_SK_COLORTYPE,
    BGRA_8888_SK_COLORTYPE,
    ALPHA_8_SK_COLORTYPE,
    RGB_565_SK_COLORTYPE,
    N_32_SK_COLORTYPE,
} sk_colortype_t;

typedef enum {
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

/**
    Return the default sk_colortype_t; this is operating-system dependent.
*/
SK_API sk_colortype_t sk_colortype_get_default_8888();

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
    A sk_bitmap_t is an abstraction that specifies a raster bitmap.
*/
typedef struct sk_bitmap_t sk_bitmap_t;
/**
    Base class for decoding compressed images into a sk_bitmap_t
*/
typedef struct sk_imagedecoder_t sk_imagedecoder_t;
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
/**
   A sk_typeface_t pecifies the typeface and intrinsic style of a font.
    This is used in the paint, along with optionally algorithmic settings like
    textSize, textSkewX, textScaleX, kFakeBoldText_Mask, to specify
    how text appears when drawn (and measured).

    Typeface objects are immutable, and so they can be shared between threads.
*/
typedef struct sk_typeface_t sk_typeface_t;
/**
   Various stream types
*/
typedef struct sk_stream_t sk_stream_t;
typedef struct sk_stream_filestream_t sk_stream_filestream_t;
typedef struct sk_stream_asset_t sk_stream_asset_t;
typedef struct sk_stream_memorystream_t sk_stream_memorystream_t;
typedef struct sk_stream_streamrewindable_t sk_stream_streamrewindable_t;

typedef enum {
	UTF8_ENCODING,
	UTF16_ENCODING,
	UTF32_ENCODING
} sk_encoding_t;
	
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

typedef enum {
    POINTS_SK_POINT_MODE,
    LINES_SK_POINT_MODE,
    POLYGON_SK_POINT_MODE
} sk_point_mode_t;

typedef enum {
    DECODEBOUNDS_SK_IMAGEDECODER_MODE,
    DECODEPIXELS_SK_IMAGEDECODER_MODE
} sk_imagedecoder_mode_t;

typedef enum {
    FAILURE_SK_IMAGEDECODER_RESULT,
    PARTIALSUCCESS_SK_IMAGEDECODER_RESULT,
    SUCCESS_SK_IMAGEDECODER_RESULT
} sk_imagedecoder_result_t;

typedef enum {
    UNKNOWN_SK_IMAGEDECODER_FORMAT,
    BMP_SK_IMAGEDECODER_FORMAT,
    GIF_SK_IMAGEDECODER_FORMAT,
    ICO_SK_IMAGEDECODER_FORMAT,
    JPEG_SK_IMAGEDECODER_FORMAT,
    PNG_SK_IMAGEDECODER_FORMAT,
    WBMP_SK_IMAGEDECODER_FORMAT,
    WEBP_SK_IMAGEDECODER_FORMAT,
    PKM_SK_IMAGEDECODER_FORMAT,
    KTX_SK_IMAGEDECODER_FORMAT,
    ASTC_SK_IMAGEDECODER_FORMAT
} sk_imagedecoder_format_t;

typedef enum {
    NORMAL_SK_BLUR_STYLE,   //!< fuzzy inside and outside
    SOLID_SK_BLUR_STYLE,    //!< solid inside, fuzzy outside
    OUTER_SK_BLUR_STYLE,    //!< nothing inside, fuzzy outside
    INNER_SK_BLUR_STYLE,    //!< fuzzy inside, nothing outside
} sk_blurstyle_t;

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
    CW_SK_PATH_DIRECTION,
    CCW_SK_PATH_DIRECTION,
} sk_path_direction_t;

typedef enum {
    CLAMP_SK_SHADER_TILEMODE,
    REPEAT_SK_SHADER_TILEMODE,
    MIRROR_SK_SHADER_TILEMODE,
} sk_shader_tilemode_t;

typedef enum {
	NORMAL_TYPEFACE_STYLE = 0,
	BOLD_TYPEFACE_STYLE = 1,
	ITALIC_TYPEFACE_STYLE = 2,
	BOLD_ITALIC_TYPEFACE_STYLE = 3
} sk_typeface_style_t;

//////////////////////////////////////////////////////////////////////////////////////////

SK_C_PLUS_PLUS_END_GUARD

#endif
