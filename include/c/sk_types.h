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

///////////////////////////////////////////////////////////////////////////////////////

SK_C_PLUS_PLUS_BEGIN_GUARD

typedef uint32_t sk_color_t;

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

sk_colortype_t sk_colortype_get_default_8888();

typedef struct {
    int32_t         width;
    int32_t         height;
    sk_colortype_t  colorType;
    sk_alphatype_t  alphaType;
} sk_imageinfo_t;

typedef struct {
    float   x;
    float   y;
} sk_point_t;

typedef struct {
    float   left;
    float   top;
    float   right;
    float   bottom;
} sk_rect_t;

typedef struct {
    float   mat[9];
} sk_matrix_t;

typedef struct sk_canvas_t sk_canvas_t;
typedef struct sk_data_t sk_data_t;
typedef struct sk_image_t sk_image_t;
typedef struct sk_maskfilter_t sk_maskfilter_t;
typedef struct sk_paint_t sk_paint_t;
typedef struct sk_path_t sk_path_t;
typedef struct sk_picture_t sk_picture_t;
typedef struct sk_picture_recorder_t sk_picture_recorder_t;
typedef struct sk_shader_t sk_shader_t;
typedef struct sk_surface_t sk_surface_t;

//////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
    class SkCanvas;
    void sk_test_capi(SkCanvas*);
#endif

SK_C_PLUS_PLUS_END_GUARD

#endif
