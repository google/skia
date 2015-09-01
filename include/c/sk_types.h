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

SK_C_PLUS_PLUS_END_GUARD

#endif
