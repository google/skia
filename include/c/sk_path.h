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
SK_API sk_path_t* sk_path_new();
/** Release the memory used by a sk_path_t. */
SK_API void sk_path_delete(sk_path_t*);

/** Set the beginning of the next contour to the point (x,y). */
SK_API void sk_path_move_to(sk_path_t*, float x, float y);
/** Set the beginning of the next contour relative to the last point on the
        previous contour. If there is no previous contour, this is treated the
        same as sk_path_move_to. 
*/
SK_API void sk_path_rmove_to(sk_path_t*, float dx, float dy);
/**
    Add a line from the last point to the specified point (x,y). If no
    sk_path_move_to() call has been made for this contour, the first
    point is automatically set to (0,0).
*/
SK_API void sk_path_line_to(sk_path_t*, float x, float y);
/**
    Same as sk_path_line_to, but the coordinates are considered relative to the last
    point on this contour. If there is no previous point, then a sk_path_move_to(0,0)
    is inserted automatically.
*/
SK_API void sk_path_rline_to(sk_path_t*, float dx, float yd);
/**
    Add a quadratic bezier from the last point, approaching control
    point (x0,y0), and ending at (x1,y1). If no sk_path_move_to() call
    has been made for this contour, the first point is automatically
    set to (0,0).
*/
SK_API void sk_path_quad_to(sk_path_t*, float x0, float y0, float x1, float y1);
/**
    Same as sk_path_quad_to, but the coordinates are considered relative to the last
    point on this contour. If there is no previous point, then a sk_path_move_to(0,0)
    is inserted automatically.
*/
SK_API void sk_path_rquad_to(sk_path_t*, float dx0, float dy0, float dx1, float dy1);
/**
    Add a conic curve from the last point, approaching control point
    (x0,y01), and ending at (x1,y1) with weight w.  If no
    sk_path_move_to() call has been made for this contour, the first
    point is automatically set to (0,0).
*/
SK_API void sk_path_conic_to(sk_path_t*, float x0, float y0, float x1, float y1, float w);
/**
    Same as sk_path_conic_to, but the coordinates are considered relative to the last
    point on this contour. If there is no previous point, then a sk_path_move_to(0,0)
    is inserted automatically.
*/
SK_API void sk_path_rconic_to(sk_path_t*, float dx0, float dy0, float dx1, float dy1, float w);
/**
    Add a cubic bezier from the last point, approaching control points
    (x0,y0) and (x1,y1), and ending at (x2,y2). If no
    sk_path_move_to() call has been made for this contour, the first
    point is automatically set to (0,0).
*/
SK_API void sk_path_cubic_to(sk_path_t*,
                             float x0, float y0,
                             float x1, float y1,
                             float x2, float y2);
/**
    Same as sk_path_cubic_to, but the coordinates are considered relative to the last
    point on this contour. If there is no previous point, then a sk_path_move_to(0,0)
    is inserted automatically.
*/
SK_API void sk_path_rcubic_to(sk_path_t*,
                             float dx0, float dy0,
                             float dx1, float dy1,
                             float dx2, float dy2);
/**
   Close the current contour. If the current point is not equal to the
   first point of the contour, a line segment is automatically added.
*/
SK_API void sk_path_close(sk_path_t*);

/**
    Add a closed rectangle contour to the path.
*/
SK_API void sk_path_add_rect(sk_path_t*, const sk_rect_t*, sk_path_direction_t);
/**
   Add a closed rectangle contour to the path with an initial point of the contour
   (startIndex) expressed as a corner index (0-3)
 */
SK_API void sk_path_add_rect_start(sk_path_t* cpath, const sk_rect_t* crect, sk_path_direction_t cdir, uint32_t startIndex);
/**
    Add a closed oval contour to the path
*/
SK_API void sk_path_add_oval(sk_path_t*, const sk_rect_t*, sk_path_direction_t);
/**
 Add the specified arc to the path as a new contour.
 */
SK_API void sk_path_add_arc(sk_path_t* cpath, const sk_rect_t* crect, float startAngle, float sweepAngle);

/**
    Get the fill type of the path.
*/
SK_API sk_path_filltype_t sk_path_get_filltype(sk_path_t*);

/**
    Set the fill type of the path.
*/
SK_API void sk_path_set_filltype(sk_path_t*, sk_path_filltype_t);

/**
 *  If the path is empty, return false and set the rect parameter to [0, 0, 0, 0].
 *  else return true and set the rect parameter to the bounds of the control-points
 *  of the path.
 */
SK_API bool sk_path_get_bounds(const sk_path_t*, sk_rect_t*);

SK_C_PLUS_PLUS_END_GUARD

#endif
