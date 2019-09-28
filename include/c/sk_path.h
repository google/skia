/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_path_DEFINED
#define sk_path_DEFINED

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

/* Path */
SK_C_API sk_path_t* sk_path_new(void);
SK_C_API void sk_path_delete(sk_path_t*);
SK_C_API void sk_path_move_to(sk_path_t*, float x, float y);
SK_C_API void sk_path_line_to(sk_path_t*, float x, float y);
SK_C_API void sk_path_quad_to(sk_path_t*, float x0, float y0, float x1, float y1);
SK_C_API void sk_path_conic_to(sk_path_t*, float x0, float y0, float x1, float y1, float w);
SK_C_API void sk_path_cubic_to(sk_path_t*, float x0, float y0, float x1, float y1, float x2, float y2);
SK_C_API void sk_path_arc_to(sk_path_t*, float rx, float ry, float xAxisRotate, sk_path_arc_size_t largeArc, sk_path_direction_t sweep, float x, float y);
SK_C_API void sk_path_rarc_to(sk_path_t*, float rx, float ry, float xAxisRotate, sk_path_arc_size_t largeArc, sk_path_direction_t sweep, float x, float y);
SK_C_API void sk_path_arc_to_with_oval(sk_path_t*, const sk_rect_t* oval, float startAngle, float sweepAngle, bool forceMoveTo);
SK_C_API void sk_path_arc_to_with_points(sk_path_t*, float x1, float y1, float x2, float y2, float radius);
SK_C_API void sk_path_close(sk_path_t*);
SK_C_API void sk_path_add_rect(sk_path_t*, const sk_rect_t*, sk_path_direction_t);
SK_C_API void sk_path_add_rrect(sk_path_t*, const sk_rrect_t*, sk_path_direction_t);
SK_C_API void sk_path_add_rrect_start(sk_path_t*, const sk_rrect_t*, sk_path_direction_t, uint32_t);
SK_C_API void sk_path_add_rounded_rect(sk_path_t*, const sk_rect_t*, float, float, sk_path_direction_t);
SK_C_API void sk_path_add_oval(sk_path_t*, const sk_rect_t*, sk_path_direction_t);
SK_C_API void sk_path_add_circle(sk_path_t*, float x, float y, float radius, sk_path_direction_t dir);
SK_C_API void sk_path_get_bounds(const sk_path_t*, sk_rect_t*);
SK_C_API void sk_path_compute_tight_bounds(const sk_path_t*, sk_rect_t*);
SK_C_API void sk_path_rmove_to(sk_path_t*, float dx, float dy);
SK_C_API void sk_path_rline_to(sk_path_t*, float dx, float yd);
SK_C_API void sk_path_rquad_to(sk_path_t*, float dx0, float dy0, float dx1, float dy1);
SK_C_API void sk_path_rconic_to(sk_path_t*, float dx0, float dy0, float dx1, float dy1, float w);
SK_C_API void sk_path_rcubic_to(sk_path_t*, float dx0, float dy0, float dx1, float dy1, float dx2, float dy2);
SK_C_API void sk_path_add_rect_start(sk_path_t* cpath, const sk_rect_t* crect, sk_path_direction_t cdir, uint32_t startIndex);
SK_C_API void sk_path_add_arc(sk_path_t* cpath, const sk_rect_t* crect, float startAngle, float sweepAngle);
SK_C_API sk_path_filltype_t sk_path_get_filltype(sk_path_t*);
SK_C_API void sk_path_set_filltype(sk_path_t*, sk_path_filltype_t);
SK_C_API void sk_path_transform(sk_path_t* cpath, const sk_matrix_t* cmatrix);
SK_C_API sk_path_t* sk_path_clone(const sk_path_t* cpath);
SK_C_API void sk_path_add_path_offset  (sk_path_t* cpath, sk_path_t* other, float dx, float dy, sk_path_add_mode_t add_mode);
SK_C_API void sk_path_add_path_matrix  (sk_path_t* cpath, sk_path_t* other, sk_matrix_t *matrix, sk_path_add_mode_t add_mode);
SK_C_API void sk_path_add_path         (sk_path_t* cpath, sk_path_t* other, sk_path_add_mode_t add_mode);
SK_C_API void sk_path_add_path_reverse (sk_path_t* cpath, sk_path_t* other);
SK_C_API void sk_path_reset (sk_path_t* cpath);
SK_C_API void sk_path_rewind (sk_path_t* cpath);
SK_C_API int sk_path_count_points (const sk_path_t* cpath);
SK_C_API int sk_path_count_verbs (const sk_path_t* cpath);
SK_C_API void sk_path_get_point (const sk_path_t* cpath, int index, sk_point_t* point);
SK_C_API int sk_path_get_points (const sk_path_t* cpath, sk_point_t* points, int max);
SK_C_API bool sk_path_contains (const sk_path_t* cpath, float x, float y);
SK_C_API sk_path_convexity_t sk_path_get_convexity (const sk_path_t* cpath);
SK_C_API void sk_path_set_convexity (sk_path_t* cpath, sk_path_convexity_t convexity);
SK_C_API bool sk_path_parse_svg_string (sk_path_t* cpath, const char* str);
SK_C_API void sk_path_to_svg_string (const sk_path_t* cpath, sk_string_t* str);
SK_C_API bool sk_path_get_last_point (const sk_path_t* cpath, sk_point_t* point);
SK_C_API int sk_path_convert_conic_to_quads(const sk_point_t* p0, const sk_point_t* p1, const sk_point_t* p2, float w, sk_point_t* pts, int pow2);
SK_C_API void sk_path_add_poly(sk_path_t* cpath, const sk_point_t* points, int count, bool close);
SK_C_API uint32_t sk_path_get_segment_masks(sk_path_t* cpath);
SK_C_API bool sk_path_is_oval(sk_path_t* cpath, sk_rect_t* bounds);
SK_C_API bool sk_path_is_rrect(sk_path_t* cpath, sk_rrect_t* bounds);
SK_C_API bool sk_path_is_line(sk_path_t* cpath, sk_point_t line [2]);
SK_C_API bool sk_path_is_rect(sk_path_t* cpath, sk_rect_t* rect, bool* isClosed, sk_path_direction_t* direction);

/* Iterators */
SK_C_API sk_path_iterator_t* sk_path_create_iter (sk_path_t *cpath, int forceClose);
SK_C_API sk_path_verb_t sk_path_iter_next (sk_path_iterator_t *iterator, sk_point_t points [4], int doConsumeDegenerates, int exact);
SK_C_API float sk_path_iter_conic_weight (sk_path_iterator_t *iterator);
SK_C_API int sk_path_iter_is_close_line (sk_path_iterator_t *iterator);
SK_C_API int sk_path_iter_is_closed_contour (sk_path_iterator_t *iterator);
SK_C_API void sk_path_iter_destroy (sk_path_iterator_t *iterator);

/* Raw iterators */
SK_C_API sk_path_rawiterator_t* sk_path_create_rawiter (sk_path_t *cpath);
SK_C_API sk_path_verb_t sk_path_rawiter_peek (sk_path_rawiterator_t *iterator);
SK_C_API sk_path_verb_t sk_path_rawiter_next (sk_path_rawiterator_t *iterator, sk_point_t points [4]);
SK_C_API float sk_path_rawiter_conic_weight (sk_path_rawiterator_t *iterator);
SK_C_API void sk_path_rawiter_destroy (sk_path_rawiterator_t *iterator);

/* Path Ops */
SK_C_API bool sk_pathop_op(const sk_path_t* one, const sk_path_t* two, sk_pathop_t op, sk_path_t* result);
SK_C_API bool sk_pathop_simplify(const sk_path_t* path, sk_path_t* result);
SK_C_API bool sk_pathop_tight_bounds(const sk_path_t* path, sk_rect_t* result);

/* Path Op Builder */
SK_C_API sk_opbuilder_t* sk_opbuilder_new(void);
SK_C_API void sk_opbuilder_destroy(sk_opbuilder_t* builder);
SK_C_API void sk_opbuilder_add(sk_opbuilder_t* builder, const sk_path_t* path, sk_pathop_t op);
SK_C_API bool sk_opbuilder_resolve(sk_opbuilder_t* builder, sk_path_t* result);

/* Path Measure */
SK_C_API sk_pathmeasure_t* sk_pathmeasure_new(void);
SK_C_API sk_pathmeasure_t* sk_pathmeasure_new_with_path(const sk_path_t* path, bool forceClosed, float resScale);
SK_C_API void sk_pathmeasure_destroy(sk_pathmeasure_t* pathMeasure);
SK_C_API void sk_pathmeasure_set_path(sk_pathmeasure_t* pathMeasure, const sk_path_t* path, bool forceClosed);
SK_C_API float sk_pathmeasure_get_length(sk_pathmeasure_t* pathMeasure);
SK_C_API bool sk_pathmeasure_get_pos_tan(sk_pathmeasure_t* pathMeasure, float distance, sk_point_t* position, sk_vector_t* tangent);
SK_C_API bool sk_pathmeasure_get_matrix(sk_pathmeasure_t* pathMeasure, float distance, sk_matrix_t* matrix, sk_pathmeasure_matrixflags_t flags);
SK_C_API bool sk_pathmeasure_get_segment(sk_pathmeasure_t* pathMeasure, float start, float stop, sk_path_t* dst, bool startWithMoveTo);
SK_C_API bool sk_pathmeasure_is_closed(sk_pathmeasure_t* pathMeasure);
SK_C_API bool sk_pathmeasure_next_contour(sk_pathmeasure_t* pathMeasure);

SK_C_PLUS_PLUS_END_GUARD

#endif
