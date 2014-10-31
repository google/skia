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

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

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

sk_colortype_t sk_colortype_get_default_8888();

typedef struct {
    int32_t         width;
    int32_t         height;
    sk_colortype_t  colorType;
    sk_alphatype_t  alphaType;
} sk_imageinfo_t;

typedef struct {
    float   left;
    float   top;
    float   right;
    float   bottom;
} sk_rect_t;

typedef struct sk_path_t sk_path_t;

sk_path_t* sk_path_new();
void sk_path_delete(sk_path_t*);
void sk_path_move_to(sk_path_t*, float x, float y);
void sk_path_line_to(sk_path_t*, float x, float y);
void sk_path_quad_to(sk_path_t*, float x0, float y0, float x1, float y1);
void sk_path_close(sk_path_t*);

typedef struct sk_paint_t sk_paint_t;

sk_paint_t* sk_paint_new();
void sk_paint_delete(sk_paint_t*);
bool sk_paint_is_antialias(const sk_paint_t*);
void sk_paint_set_antialias(sk_paint_t*, bool);
sk_color_t sk_paint_get_color(const sk_paint_t*);
void sk_paint_set_color(sk_paint_t*, sk_color_t);

typedef struct sk_canvas_t sk_canvas_t;
typedef struct sk_image_t sk_image_t;

void sk_canvas_save(sk_canvas_t*);
void sk_canvas_save_layer(sk_canvas_t*, const sk_rect_t*, const sk_paint_t*);
void sk_canvas_restore(sk_canvas_t*);

void sk_canvas_translate(sk_canvas_t*, float dx, float dy);
void sk_canvas_scale(sk_canvas_t*, float sx, float sy);

void sk_canvas_draw_paint(sk_canvas_t*, const sk_paint_t*);
void sk_canvas_draw_rect(sk_canvas_t*, const sk_rect_t*, const sk_paint_t*);
void sk_canvas_draw_oval(sk_canvas_t*, const sk_rect_t*, const sk_paint_t*);
void sk_canvas_draw_path(sk_canvas_t*, const sk_path_t*, const sk_paint_t*);
void sk_canvas_draw_image(sk_canvas_t*, const sk_image_t*, float x, float y, const sk_paint_t*);

/**
 *  Return a new image that has made a copy of the provided pixels, or NULL on failure.
 *  Balance with a call to sk_image_unref().
 */
sk_image_t* sk_image_new_raster_copy(const sk_imageinfo_t*, const void* pixels, size_t rowBytes);
void sk_image_ref(const sk_image_t*);
void sk_image_unref(const sk_image_t*);
int sk_image_get_width(const sk_image_t*);
int sk_image_get_height(const sk_image_t*);
uint32_t sk_image_get_unique_id(const sk_image_t*);

typedef struct sk_surface_t sk_surface_t;

sk_surface_t* sk_surface_new_raster(const sk_imageinfo_t*);
sk_surface_t* sk_surface_new_raster_direct(const sk_imageinfo_t*, void* pixels, size_t rowBytes);
void sk_surface_delete(sk_surface_t*);

/**
 *  Return the canvas associated with this surface. Note: the canvas is owned by the surface,
 *  so the returned object is only valid while the owning surface is valid.
 */
sk_canvas_t* sk_surface_get_canvas(sk_surface_t*);
sk_image_t* sk_surface_new_image_snapshot(sk_surface_t*);

#ifdef __cplusplus
    class SkCanvas;
    void sk_test_capi(SkCanvas*);
}
#endif

#endif
