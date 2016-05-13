/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_x_canvas_DEFINED
#define sk_x_canvas_DEFINED

#include "sk_types.h"
#include "xamarin/sk_x_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_API void sk_canvas_clear(sk_canvas_t*, sk_color_t);
/**
   This makes the contents of the canvas undefined. Subsequent calls that
   require reading the canvas contents will produce undefined results. Examples
   include blending and readPixels. The actual implementation is backend-
   dependent and one legal implementation is to do nothing. Like clear(), this
   ignores the clip.
   
   This function should only be called if the caller intends to subsequently
   draw to the canvas. The canvas may do real work at discard() time in order
   to optimize performance on subsequent draws. Thus, if you call this and then
   never draw to the canvas subsequently you may pay a perfomance penalty.
*/
SK_API void sk_canvas_discard(sk_canvas_t*);
/**
    Returns the number of matrix/clip states on the SkCanvas' private stack.
    This will equal # save() calls - # restore() calls + 1. The save count on
    a new canvas is 1.
*/
SK_API int sk_canvas_get_save_count(sk_canvas_t*);
/**
    Efficient way to pop any calls to sk_canvas_save() that happened after the save
    count reached saveCount. It is an error for saveCount to be greater than
    getSaveCount(). To pop all the way back to the initial matrix/clip context
    pass saveCount == 1.
*/
SK_API void sk_canvas_restore_to_count(sk_canvas_t*, int saveCount);
/**
    Draws with the specified color and mode.
**/
SK_API void sk_canvas_draw_color(sk_canvas_t* ccanvas, sk_color_t color, sk_xfermode_mode_t mode);
/**
   Draw a series of points, interpreted based on the sk_point_mode_t mode. For
   all modes, the count parameter is interpreted as the total number of
   points. For LINES_SK_POINT_MODE mode, count/2 line segments are drawn.
   For POINTS_SK_POINT_MODE mode, each point is drawn centered at its coordinate, and its
   size is specified by the paint's stroke-width. It draws as a square,
   unless the paint's cap-type is round, in which the points are drawn as
   circles.
   For LINES_SK_POINT_MODE mode, each pair of points is drawn as a line segment,
   respecting the paint's settings for cap/join/width.
   For POLYGON_SK_POINT_MODE mode, the entire array is drawn as a series of connected
   line segments.
   Note that, while similar, LINES_SK_POINT_MODE and POLYGON_SK_POINT_MODE modes draw slightly
   differently than the equivalent path built with a series of moveto,
   lineto calls, in that the path will draw all of its contours at once,
   with no interactions if contours intersect each other (think XOR
   xfermode). sk_canvas_draw_paint always draws each element one at a time.
*/
SK_API void sk_canvas_draw_points(sk_canvas_t*, sk_point_mode_t, size_t, const sk_point_t[], const sk_paint_t*);
/**
   Draws a single point with the specified paint
*/
SK_API void sk_canvas_draw_point(sk_canvas_t*, float, float, const sk_paint_t*);
/**
   Draws a single point with the specified paint
*/
SK_API void sk_canvas_draw_point_color(sk_canvas_t*, float, float, sk_color_t);
/**
   Draws a line from x0,y0 to x1,y1
*/
SK_API void sk_canvas_draw_line(sk_canvas_t* ccanvas, float x0, float y0, float x1, float y1, sk_paint_t* cpaint);
/**
   Draw the text, with origin at (x,y), using the specified paint.
   The origin is interpreted based on the Align setting in the paint.
    
   @param text The text to be drawn
   @param byteLength   The number of bytes to read from the text parameter
   @param x        The x-coordinate of the origin of the text being drawn
   @param y        The y-coordinate of the origin of the text being drawn
   @param paint    The paint used for the text (e.g. color, size, style)
*/
SK_API void sk_canvas_draw_text (sk_canvas_t*, const char *text, size_t byteLength, float x, float y, const sk_paint_t* paint);
/**
    Draw the text, with each character/glyph origin specified by the pos[]
    array. The origin is interpreted by the Align setting in the paint.
    @param text The text to be drawn
    @param byteLength   The number of bytes to read from the text parameter
    @param pos      Array of positions, used to position each character
    @param paint    The paint used for the text (e.g. color, size, style)
*/
SK_API void sk_canvas_draw_pos_text (sk_canvas_t*, const char *text, size_t byteLength, const sk_point_t[], const sk_paint_t* paint);
SK_API void sk_canvas_draw_text_on_path (sk_canvas_t*, const char *text, size_t byteLength, const sk_path_t*path, float hOffset, float vOffset, const sk_paint_t* paint);
SK_API void sk_canvas_draw_bitmap(sk_canvas_t* ccanvas, const sk_bitmap_t* cbitmap, float x, float y, const sk_paint_t* cpaint);
SK_API void sk_canvas_draw_bitmap_rect(sk_canvas_t* ccanvas, const sk_bitmap_t* cbitmap, const sk_rect_t* csrcR, const sk_rect_t* cdstR, const sk_paint_t* cpaint);
SK_API void sk_canvas_reset_matrix(sk_canvas_t* ccanvas);
SK_API void sk_canvas_set_matrix(sk_canvas_t* ccanvas, const sk_matrix_t* cmatrix);
SK_API void sk_canvas_get_total_matrix(sk_canvas_t* ccanvas, sk_matrix_t* cmatrix);
/**
    Draw the specified rounded rectangle using the specified paint. The
    rectangle will be filled or stroked based on the style in the
    paint.
*/
SK_API void sk_canvas_draw_round_rect(sk_canvas_t*, const sk_rect_t*, float rx, float ry, const sk_paint_t*);
/**
    Modify the current clip with the specified rectangle.
*/
SK_API void sk_canvas_clip_rect_with_operation(sk_canvas_t* t, const sk_rect_t* crect, sk_region_op_t op, bool doAA);
/**
    Modify the current clip with the specified path.
*/
SK_API void sk_canvas_clip_path_with_operation(sk_canvas_t* t, const sk_path_t* crect, sk_region_op_t op, bool doAA);

/** Return the bounds of the current clip (in local coordinates) in the
    bounds parameter, and return true if it is non-empty. This can be useful
    in a way similar to quickReject, in that it tells you that drawing
    outside of these bounds will be clipped out.
*/
SK_API bool sk_canvas_get_clip_bounds(sk_canvas_t* t, sk_rect_t* cbounds);
/** Return the bounds of the current clip, in device coordinates; returns
    true if non-empty. Maybe faster than getting the clip explicitly and
    then taking its bounds.
*/
SK_API bool sk_canvas_get_clip_device_bounds(sk_canvas_t* t, sk_irect_t* cbounds);

SK_C_PLUS_PLUS_END_GUARD

#endif
