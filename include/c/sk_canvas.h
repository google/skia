/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_canvas_DEFINED
#define sk_canvas_DEFINED

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_C_API void sk_canvas_destroy(sk_canvas_t*);
SK_C_API int sk_canvas_save(sk_canvas_t*);
SK_C_API int sk_canvas_save_layer(sk_canvas_t*, const sk_rect_t*, const sk_paint_t*);
SK_C_API void sk_canvas_restore(sk_canvas_t*);
SK_C_API void sk_canvas_translate(sk_canvas_t*, float dx, float dy);
SK_C_API void sk_canvas_scale(sk_canvas_t*, float sx, float sy);
SK_C_API void sk_canvas_rotate_degrees(sk_canvas_t*, float degrees);
SK_C_API void sk_canvas_rotate_radians(sk_canvas_t*, float radians);
SK_C_API void sk_canvas_skew(sk_canvas_t*, float sx, float sy);
SK_C_API void sk_canvas_concat(sk_canvas_t*, const sk_matrix_t*);
SK_C_API bool sk_canvas_quick_reject(sk_canvas_t*, const sk_rect_t*);
SK_C_API void sk_canvas_clip_region(sk_canvas_t* canvas, const sk_region_t* region, sk_clipop_t op);
SK_C_API void sk_canvas_draw_paint(sk_canvas_t*, const sk_paint_t*);
SK_C_API void sk_canvas_draw_rect(sk_canvas_t*, const sk_rect_t*, const sk_paint_t*);
SK_C_API void sk_canvas_draw_rrect(sk_canvas_t*, const sk_rrect_t*, const sk_paint_t*);
SK_C_API void sk_canvas_draw_region(sk_canvas_t*, const sk_region_t*, const sk_paint_t*);
SK_C_API void sk_canvas_draw_circle(sk_canvas_t*, float cx, float cy, float rad, const sk_paint_t*);
SK_C_API void sk_canvas_draw_oval(sk_canvas_t*, const sk_rect_t*, const sk_paint_t*);
SK_C_API void sk_canvas_draw_path(sk_canvas_t*, const sk_path_t*, const sk_paint_t*);
SK_C_API void sk_canvas_draw_image(sk_canvas_t*, const sk_image_t*, float x, float y, const sk_paint_t*);
SK_C_API void sk_canvas_draw_image_rect(sk_canvas_t*, const sk_image_t*, const sk_rect_t* src, const sk_rect_t* dst, const sk_paint_t*);
SK_C_API void sk_canvas_draw_picture(sk_canvas_t*, const sk_picture_t*, const sk_matrix_t*, const sk_paint_t*);
SK_C_API void sk_canvas_draw_drawable(sk_canvas_t*, sk_drawable_t*, const sk_matrix_t*);
SK_C_API void sk_canvas_clear(sk_canvas_t*, sk_color_t);
SK_C_API void sk_canvas_discard(sk_canvas_t*);
SK_C_API int sk_canvas_get_save_count(sk_canvas_t*);
SK_C_API void sk_canvas_restore_to_count(sk_canvas_t*, int saveCount);
SK_C_API void sk_canvas_draw_color(sk_canvas_t* ccanvas, sk_color_t color, sk_blendmode_t mode);
SK_C_API void sk_canvas_draw_points(sk_canvas_t*, sk_point_mode_t, size_t, const sk_point_t[], const sk_paint_t*);
SK_C_API void sk_canvas_draw_point(sk_canvas_t*, float, float, const sk_paint_t*);
SK_C_API void sk_canvas_draw_line(sk_canvas_t* ccanvas, float x0, float y0, float x1, float y1, sk_paint_t* cpaint);
SK_C_API void sk_canvas_draw_text_blob (sk_canvas_t*, sk_textblob_t* text, float x, float y, const sk_paint_t* paint);
SK_C_API void sk_canvas_draw_bitmap(sk_canvas_t* ccanvas, const sk_bitmap_t* bitmap, float left, float top, const sk_paint_t* paint);
SK_C_API void sk_canvas_draw_bitmap_rect(sk_canvas_t* ccanvas, const sk_bitmap_t* bitmap, const sk_rect_t* src, const sk_rect_t* dst, const sk_paint_t* paint);
SK_C_API void sk_canvas_reset_matrix(sk_canvas_t* ccanvas);
SK_C_API void sk_canvas_set_matrix(sk_canvas_t* ccanvas, const sk_matrix_t* matrix);
SK_C_API void sk_canvas_get_total_matrix(sk_canvas_t* ccanvas, sk_matrix_t* matrix);
SK_C_API void sk_canvas_draw_round_rect(sk_canvas_t*, const sk_rect_t*, float rx, float ry, const sk_paint_t*);
SK_C_API void sk_canvas_clip_rect_with_operation(sk_canvas_t* t, const sk_rect_t* crect, sk_clipop_t op, bool doAA);
SK_C_API void sk_canvas_clip_path_with_operation(sk_canvas_t* t, const sk_path_t* crect, sk_clipop_t op, bool doAA);
SK_C_API void sk_canvas_clip_rrect_with_operation(sk_canvas_t* t, const sk_rrect_t* crect, sk_clipop_t op, bool doAA);
SK_C_API bool sk_canvas_get_local_clip_bounds(sk_canvas_t* t, sk_rect_t* cbounds);
SK_C_API bool sk_canvas_get_device_clip_bounds(sk_canvas_t* t, sk_irect_t* cbounds);
SK_C_API void sk_canvas_flush(sk_canvas_t* ccanvas);
SK_C_API sk_canvas_t* sk_canvas_new_from_bitmap(const sk_bitmap_t* bitmap);
SK_C_API void sk_canvas_draw_annotation(sk_canvas_t* t, const sk_rect_t* rect, const char* key, sk_data_t* value);
SK_C_API void sk_canvas_draw_url_annotation(sk_canvas_t* t, const sk_rect_t* rect, sk_data_t* value);
SK_C_API void sk_canvas_draw_named_destination_annotation(sk_canvas_t* t, const sk_point_t* point, sk_data_t* value);
SK_C_API void sk_canvas_draw_link_destination_annotation(sk_canvas_t* t, const sk_rect_t* rect, sk_data_t* value);
SK_C_API void sk_canvas_draw_bitmap_lattice(sk_canvas_t* t, const sk_bitmap_t* bitmap, const sk_lattice_t* lattice, const sk_rect_t* dst, const sk_paint_t* paint);
SK_C_API void sk_canvas_draw_image_lattice(sk_canvas_t* t, const sk_image_t* image, const sk_lattice_t* lattice, const sk_rect_t* dst, const sk_paint_t* paint);
SK_C_API void sk_canvas_draw_bitmap_nine(sk_canvas_t* t, const sk_bitmap_t* bitmap, const sk_irect_t* center, const sk_rect_t* dst, const sk_paint_t* paint);
SK_C_API void sk_canvas_draw_image_nine(sk_canvas_t* t, const sk_image_t* image, const sk_irect_t* center, const sk_rect_t* dst, const sk_paint_t* paint);
SK_C_API void sk_canvas_draw_vertices(sk_canvas_t* ccanvas, sk_vertices_t* vertices, sk_blendmode_t mode, const sk_paint_t* paint);

SK_C_API sk_nodraw_canvas_t* sk_nodraw_canvas_new(int width, int height);
SK_C_API void sk_nodraw_canvas_destroy(sk_nodraw_canvas_t*);

SK_C_API sk_nway_canvas_t* sk_nway_canvas_new(int width, int height);
SK_C_API void sk_nway_canvas_destroy(sk_nway_canvas_t*);
SK_C_API void sk_nway_canvas_add_canvas(sk_nway_canvas_t*, sk_canvas_t* canvas);
SK_C_API void sk_nway_canvas_remove_canvas(sk_nway_canvas_t*, sk_canvas_t* canvas);
SK_C_API void sk_nway_canvas_remove_all(sk_nway_canvas_t*);

SK_C_API sk_overdraw_canvas_t* sk_overdraw_canvas_new(sk_canvas_t* canvas);
SK_C_API void sk_overdraw_canvas_destroy(sk_overdraw_canvas_t* canvas);

SK_C_PLUS_PLUS_END_GUARD

#endif
