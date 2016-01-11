/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_canvas_DEFINED
#define sk_canvas_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

/**
    Save the current matrix and clip on the canvas.  When the
    balancing call to sk_canvas_restore() is made, the previous matrix
    and clip are restored.
*/
SK_API void sk_canvas_clear(sk_canvas_t*);
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
    Save the current matrix and clip on the canvas.  When the
    balancing call to sk_canvas_restore() is made, the previous matrix
    and clip are restored.
*/
SK_API void sk_canvas_save(sk_canvas_t*);
/**
    This behaves the same as sk_canvas_save(), but in addition it
    allocates an offscreen surface. All drawing calls are directed
    there, and only when the balancing call to sk_canvas_restore() is
    made is that offscreen transfered to the canvas (or the previous
    layer).

    @param sk_rect_t* (may be null) This rect, if non-null, is used as
                      a hint to limit the size of the offscreen, and
                      thus drawing may be clipped to it, though that
                      clipping is not guaranteed to happen. If exact
                      clipping is desired, use sk_canvas_clip_rect().
    @param sk_paint_t* (may be null) The paint is copied, and is applied
                       to the offscreen when sk_canvas_restore() is
                       called.
*/
SK_API void sk_canvas_save_layer(sk_canvas_t*, const sk_rect_t*, const sk_paint_t*);
/**
    This call balances a previous call to sk_canvas_save() or
    sk_canvas_save_layer(), and is used to remove all modifications to
    the matrix and clip state since the last save call.  It is an
    error to call sk_canvas_restore() more times than save and
    save_layer were called.
*/
SK_API void sk_canvas_restore(sk_canvas_t*);
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
    Preconcat the current coordinate transformation matrix with the
    specified translation.
*/
SK_API void sk_canvas_translate(sk_canvas_t*, float dx, float dy);
/**
    Preconcat the current coordinate transformation matrix with the
    specified scale.
*/
SK_API void sk_canvas_scale(sk_canvas_t*, float sx, float sy);
/**
    Preconcat the current coordinate transformation matrix with the
    specified rotation in degrees.
*/
SK_API void sk_canvas_rotate_degrees(sk_canvas_t*, float degrees);
/**
    Preconcat the current coordinate transformation matrix with the
    specified rotation in radians.
*/
SK_API void sk_canvas_rotate_radians(sk_canvas_t*, float radians);
/**
    Preconcat the current coordinate transformation matrix with the
    specified skew.
*/
SK_API void sk_canvas_skew(sk_canvas_t*, float sx, float sy);
/**
    Preconcat the current coordinate transformation matrix with the
    specified matrix.
*/
SK_API void sk_canvas_concat(sk_canvas_t*, const sk_matrix_t*);

/**
    Modify the current clip with the specified rectangle.  The new
    current clip will be the intersection of the old clip and the
    rectange.
*/
SK_API void sk_canvas_clip_rect(sk_canvas_t*, const sk_rect_t*);
/**
    Modify the current clip with the specified path.  The new
    current clip will be the intersection of the old clip and the
    path.
*/
SK_API void sk_canvas_clip_path(sk_canvas_t*, const sk_path_t*);

/**
    Fill the entire canvas (restricted to the current clip) with the
    specified paint.
*/
SK_API void sk_canvas_draw_paint(sk_canvas_t*, const sk_paint_t*);
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
    Draw the specified rectangle using the specified paint. The
    rectangle will be filled or stroked based on the style in the
    paint.
*/
SK_API void sk_canvas_draw_rect(sk_canvas_t*, const sk_rect_t*, const sk_paint_t*);
/**
    Draw the specified oval using the specified paint. The oval will be
    filled or framed based on the style in the paint
*/
SK_API void sk_canvas_draw_oval(sk_canvas_t*, const sk_rect_t*, const sk_paint_t*);
/**
    Draw the specified path using the specified paint. The path will be
    filled or framed based on the style in the paint
*/
SK_API void sk_canvas_draw_path(sk_canvas_t*, const sk_path_t*, const sk_paint_t*);
/**
    Draw the specified image, with its top/left corner at (x,y), using
    the specified paint, transformed by the current matrix.

    @param sk_paint_t* (may be NULL) the paint used to draw the image.
*/
SK_API void sk_canvas_draw_image(sk_canvas_t*, const sk_image_t*,
                                 float x, float y, const sk_paint_t*);
/**
    Draw the specified image, scaling and translating so that it fills
    the specified dst rect. If the src rect is non-null, only that
    subset of the image is transformed and drawn.

    @param sk_paint_t* (may be NULL) The paint used to draw the image.
*/
SK_API void sk_canvas_draw_image_rect(sk_canvas_t*, const sk_image_t*,
                                      const sk_rect_t* src,
                                      const sk_rect_t* dst, const sk_paint_t*);

/**
    Draw the picture into this canvas (replay the pciture's drawing commands).

    @param sk_matrix_t* If non-null, apply that matrix to the CTM when
                        drawing this picture. This is logically
                        equivalent to: save, concat, draw_picture,
                        restore.

    @param sk_paint_t* If non-null, draw the picture into a temporary
                       buffer, and then apply the paint's alpha,
                       colorfilter, imagefilter, and xfermode to that
                       buffer as it is drawn to the canvas.  This is
                       logically equivalent to save_layer(paint),
                       draw_picture, restore.
*/
SK_API void sk_canvas_draw_picture(sk_canvas_t*, const sk_picture_t*,
                                   const sk_matrix_t*, const sk_paint_t*);

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

SK_C_PLUS_PLUS_END_GUARD

#endif
