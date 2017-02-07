/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_path_DEFINED
#define sk_path_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

/** Create a new, empty path. */
SK_C_API sk_path_t* sk_path_new();
/** Release the memory used by a sk_path_t. */
SK_C_API void sk_path_delete(sk_path_t*);

/** Set the beginning of the next contour to the point (x,y). */
SK_C_API void sk_path_move_to(sk_path_t*, float x, float y);
/**
    Add a line from the last point to the specified point (x,y). If no
    sk_path_move_to() call has been made for this contour, the first
    point is automatically set to (0,0).
*/
SK_C_API void sk_path_line_to(sk_path_t*, float x, float y);
/**
    Add a quadratic bezier from the last point, approaching control
    point (x0,y0), and ending at (x1,y1). If no sk_path_move_to() call
    has been made for this contour, the first point is automatically
    set to (0,0).
*/
SK_C_API void sk_path_quad_to(sk_path_t*, float x0, float y0, float x1, float y1);
/**
    Add a conic curve from the last point, approaching control point
    (x0,y01), and ending at (x1,y1) with weight w.  If no
    sk_path_move_to() call has been made for this contour, the first
    point is automatically set to (0,0).
*/
SK_C_API void sk_path_conic_to(sk_path_t*, float x0, float y0, float x1, float y1, float w);
/**
    Add a cubic bezier from the last point, approaching control points
    (x0,y0) and (x1,y1), and ending at (x2,y2). If no
    sk_path_move_to() call has been made for this contour, the first
    point is automatically set to (0,0).
*/
SK_C_API void sk_path_cubic_to(sk_path_t*,
                             float x0, float y0,
                             float x1, float y1,
                             float x2, float y2);
/**
 *  Append an elliptical arc from the current point in the format used by SVG.
 *  The center of the ellipse is computed to satisfy the constraints below.
 */
SK_C_API void sk_path_arc_to(sk_path_t*, float rx, float ry, float xAxisRotate, sk_path_arc_size_t largeArc, sk_path_direction_t sweep, float x, float y);
/**
 *  Same as arcTo format used by SVG, but the destination coordinate is relative to the
 *  last point on this contour. If there is no previous point, then a
 *  moveTo(0,0) is inserted automatically.
 */
SK_C_API void sk_path_rarc_to(sk_path_t*, float rx, float ry, float xAxisRotate, sk_path_arc_size_t largeArc, sk_path_direction_t sweep, float x, float y);

SK_C_API void sk_path_arc_to_with_oval(sk_path_t*, const sk_rect_t* oval, float startAngle, float sweepAngle, bool forceMoveTo);

SK_C_API void sk_path_arc_to_with_points(sk_path_t*, float x1, float y1, float x2, float y2, float radius);

/**
   Close the current contour. If the current point is not equal to the
   first point of the contour, a line segment is automatically added.
*/
SK_C_API void sk_path_close(sk_path_t*);

/**
    Add a closed rectangle contour to the path.
*/
SK_C_API void sk_path_add_rect(sk_path_t*, const sk_rect_t*, sk_path_direction_t);
/**
 *  Add a closed rounded rectangle contour to the path.
 */
SK_C_API void sk_path_add_rounded_rect(sk_path_t*, const sk_rect_t*, float, float, sk_path_direction_t);
/**
    Add a closed oval contour to the path
*/
SK_C_API void sk_path_add_oval(sk_path_t*, const sk_rect_t*, sk_path_direction_t);
/**
 *  Add a closed circle contour to the path. The circle contour begins at
 *  the right-most point (as though 1 were passed to addOval's 'start' param).
 */
SK_C_API void sk_path_add_circle(sk_path_t*, float x, float y, float radius, sk_path_direction_t dir);

/**
 *  If the path is empty, return false and set the rect parameter to [0, 0, 0, 0].
 *  else return true and set the rect parameter to the bounds of the control-points
 *  of the path.
 */
SK_C_API bool sk_path_get_bounds(const sk_path_t*, sk_rect_t*);

/** Set the beginning of the next contour relative to the last point on the
        previous contour. If there is no previous contour, this is treated the
        same as sk_path_move_to. 
*/
SK_C_API void sk_path_rmove_to(sk_path_t*, float dx, float dy);
/**
    Same as sk_path_line_to, but the coordinates are considered relative to the last
    point on this contour. If there is no previous point, then a sk_path_move_to(0,0)
    is inserted automatically.
*/
SK_C_API void sk_path_rline_to(sk_path_t*, float dx, float yd);
/**
    Same as sk_path_quad_to, but the coordinates are considered relative to the last
    point on this contour. If there is no previous point, then a sk_path_move_to(0,0)
    is inserted automatically.
*/
SK_C_API void sk_path_rquad_to(sk_path_t*, float dx0, float dy0, float dx1, float dy1);
/**
    Same as sk_path_conic_to, but the coordinates are considered relative to the last
    point on this contour. If there is no previous point, then a sk_path_move_to(0,0)
    is inserted automatically.
*/
SK_C_API void sk_path_rconic_to(sk_path_t*, float dx0, float dy0, float dx1, float dy1, float w);
/**
    Same as sk_path_cubic_to, but the coordinates are considered relative to the last
    point on this contour. If there is no previous point, then a sk_path_move_to(0,0)
    is inserted automatically.
*/
SK_C_API void sk_path_rcubic_to(sk_path_t*,
                             float dx0, float dy0,
                             float dx1, float dy1,
                             float dx2, float dy2);
/**
   Add a closed rectangle contour to the path with an initial point of the contour
   (startIndex) expressed as a corner index (0-3)
 */
SK_C_API void sk_path_add_rect_start(sk_path_t* cpath, const sk_rect_t* crect, sk_path_direction_t cdir, uint32_t startIndex);
/**
 Add the specified arc to the path as a new contour.
 */
SK_C_API void sk_path_add_arc(sk_path_t* cpath, const sk_rect_t* crect, float startAngle, float sweepAngle);
/**
    Get the fill type of the path.
*/
SK_C_API sk_path_filltype_t sk_path_get_filltype(sk_path_t*);
/**
    Set the fill type of the path.
*/
SK_C_API void sk_path_set_filltype(sk_path_t*, sk_path_filltype_t);
/**
    Transform the points in this path by matrix, and write the answer back into the path
*/
SK_C_API void sk_path_transform(sk_path_t* cpath, const sk_matrix_t* cmatrix);
/**
    Creates a copy of the path
*/
SK_C_API sk_path_t* sk_path_clone(const sk_path_t* cpath);

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

/* Paths */

/**
   Adds the @other path to the @cpath by appending a @dx, @dy offset to each node, using the specified adding mode in @add_mode
 */ 
SK_C_API void sk_path_add_path_offset  (sk_path_t* cpath, sk_path_t* other, float dx, float dy, sk_path_add_mode_t add_mode);
/**
   Adds the @other path to the @cpath by applying the @matrix transformation on the @other, using the specified adding mode in @add_mode
 */ 
SK_C_API void sk_path_add_path_matrix  (sk_path_t* cpath, sk_path_t* other, sk_matrix_t *matrix, sk_path_add_mode_t add_mode);
/**
   Adds the @other path to the @cpath using the specified adding mode in @add_mode
 */ 
SK_C_API void sk_path_add_path         (sk_path_t* cpath, sk_path_t* other, sk_path_add_mode_t add_mode);
SK_C_API void sk_path_add_path_reverse (sk_path_t* cpath, sk_path_t* other);

/**
   Clear any lines and curves from the path, making it empty. This frees up
   internal storage associated with those segments.
   On Android, does not change fSourcePath.
 */
SK_C_API void sk_path_reset (sk_path_t* cpath);

/**
   Similar to sk_path_reset (), in that all lines and curves are removed from the
   path. However, any internal storage for those lines/curves is retained,
   making reuse of the path potentially faster.
   On Android, does not change fSourcePath.   
 */
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

SK_C_API bool sk_pathop_op(const sk_path_t* one, const sk_path_t* two, sk_pathop_t op, sk_path_t* result);

SK_C_API bool sk_pathop_simplify(const sk_path_t* path, sk_path_t* result);

SK_C_API bool sk_pathop_tight_bounds(const sk_path_t* path, sk_rect_t* result);

SK_C_API sk_opbuilder_t* sk_opbuilder_new();

SK_C_API void sk_opbuilder_destroy(sk_opbuilder_t* builder);

SK_C_API void sk_opbuilder_add(sk_opbuilder_t* builder, const sk_path_t* path, sk_pathop_t op);

SK_C_API bool sk_opbuilder_resolve(sk_opbuilder_t* builder, sk_path_t* result);

SK_C_API int sk_path_convert_conic_to_quads(const sk_point_t* p0, const sk_point_t* p1, const sk_point_t* p2, float w, sk_point_t* pts, int pow2);

SK_C_API sk_pathmeasure_t* sk_pathmeasure_new();

SK_C_API sk_pathmeasure_t* sk_pathmeasure_new_with_path(const sk_path_t* path, bool forceClosed, float resScale);

SK_C_API void sk_pathmeasure_destroy(sk_pathmeasure_t* pathMeasure);

SK_C_API void sk_pathmeasure_set_path(sk_pathmeasure_t* pathMeasure, const sk_path_t* path, bool forceClosed);

SK_C_API float sk_pathmeasure_get_length(sk_pathmeasure_t* pathMeasure);

SK_C_API bool sk_pathmeasure_get_pos_tan(sk_pathmeasure_t* pathMeasure, float distance, sk_point_t* position, sk_vector_t* tangent);

SK_C_API bool sk_pathmeasure_get_matrix(sk_pathmeasure_t* pathMeasure, float distance, sk_matrix_t* matrix, sk_pathmeasure_matrixflags_t flags);

SK_C_API bool sk_pathmeasure_get_segment(sk_pathmeasure_t* pathMeasure, float start, float stop, sk_path_t* dst, bool startWithMoveTo);

SK_C_API bool sk_pathmeasure_is_closed(sk_pathmeasure_t* pathMeasure);

SK_C_API bool sk_pathmeasure_next_contour(sk_pathmeasure_t* pathMeasure);

SK_C_API void sk_path_add_poly(sk_path_t* cpath, const sk_point_t* points, int count, bool close);

SK_C_API uint32_t sk_path_get_segment_masks(sk_path_t* cpath);

SK_C_PLUS_PLUS_END_GUARD

#endif
