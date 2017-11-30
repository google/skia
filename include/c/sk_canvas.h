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

SK_C_API void sk_canvas_destroy(sk_canvas_t*);

/**
    Save the current matrix and clip on the canvas.  When the
    balancing call to sk_canvas_restore() is made, the previous matrix
    and clip are restored.
*/
SK_C_API int sk_canvas_save(sk_canvas_t*);
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
SK_C_API int sk_canvas_save_layer(sk_canvas_t*, const sk_rect_t*, const sk_paint_t*);
/**
    This call balances a previous call to sk_canvas_save() or
    sk_canvas_save_layer(), and is used to remove all modifications to
    the matrix and clip state since the last save call.  It is an
    error to call sk_canvas_restore() more times than save and
    save_layer were called.
*/
SK_C_API void sk_canvas_restore(sk_canvas_t*);
/**
    Preconcat the current coordinate transformation matrix with the
    specified translation.
*/
SK_C_API void sk_canvas_translate(sk_canvas_t*, float dx, float dy);
/**
    Preconcat the current coordinate transformation matrix with the
    specified scale.
*/
SK_C_API void sk_canvas_scale(sk_canvas_t*, float sx, float sy);
/**
    Preconcat the current coordinate transformation matrix with the
    specified rotation in degrees.
*/
SK_C_API void sk_canvas_rotate_degrees(sk_canvas_t*, float degrees);
/**
    Preconcat the current coordinate transformation matrix with the
    specified rotation in radians.
*/
SK_C_API void sk_canvas_rotate_radians(sk_canvas_t*, float radians);
/**
    Preconcat the current coordinate transformation matrix with the
    specified skew.
*/
SK_C_API void sk_canvas_skew(sk_canvas_t*, float sx, float sy);
/**
    Preconcat the current coordinate transformation matrix with the
    specified matrix.
*/
SK_C_API void sk_canvas_concat(sk_canvas_t*, const sk_matrix_t*);

/**
    Modify the current clip with the specified rectangle.  The new
    current clip will be the intersection of the old clip and the
    rectange.
*/
SK_C_API bool sk_canvas_quick_reject(sk_canvas_t*, const sk_rect_t*);
SK_C_API void sk_canvas_clip_rect(sk_canvas_t*, const sk_rect_t*);
SK_C_API void sk_canvas_clip_region(sk_canvas_t* canvas, const sk_region_t* region, sk_clipop_t op);
/**
    Modify the current clip with the specified path.  The new
    current clip will be the intersection of the old clip and the
    path.
*/
SK_C_API void sk_canvas_clip_path(sk_canvas_t*, const sk_path_t*);

/**
    Fill the entire canvas (restricted to the current clip) with the
    specified paint.
*/
SK_C_API void sk_canvas_draw_paint(sk_canvas_t*, const sk_paint_t*);
/**
    Draw the specified rectangle using the specified paint. The
    rectangle will be filled or stroked based on the style in the
    paint.
*/
SK_C_API void sk_canvas_draw_rect(sk_canvas_t*, const sk_rect_t*, const sk_paint_t*);
SK_C_API void sk_canvas_draw_region(sk_canvas_t*, const sk_region_t*, const sk_paint_t*);
/**
 *  Draw the circle centered at (cx, cy) with radius rad using the specified paint.
 *  The circle will be filled or framed based on the style in the paint
 */
SK_C_API void sk_canvas_draw_circle(sk_canvas_t*, float cx, float cy, float rad, const sk_paint_t*);
/**
    Draw the specified oval using the specified paint. The oval will be
    filled or framed based on the style in the paint
*/
SK_C_API void sk_canvas_draw_oval(sk_canvas_t*, const sk_rect_t*, const sk_paint_t*);
/**
    Draw the specified path using the specified paint. The path will be
    filled or framed based on the style in the paint
*/
SK_C_API void sk_canvas_draw_path(sk_canvas_t*, const sk_path_t*, const sk_paint_t*);
/**
    Draw the specified image, with its top/left corner at (x,y), using
    the specified paint, transformed by the current matrix.

    @param sk_paint_t* (may be NULL) the paint used to draw the image.
*/
SK_C_API void sk_canvas_draw_image(sk_canvas_t*, const sk_image_t*,
                                 float x, float y, const sk_paint_t*);
/**
    Draw the specified image, scaling and translating so that it fills
    the specified dst rect. If the src rect is non-null, only that
    subset of the image is transformed and drawn.

    @param sk_paint_t* (may be NULL) The paint used to draw the image.
*/
SK_C_API void sk_canvas_draw_image_rect(sk_canvas_t*, const sk_image_t*,
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
SK_C_API void sk_canvas_draw_picture(sk_canvas_t*, const sk_picture_t*,
                                   const sk_matrix_t*, const sk_paint_t*);
/**
   Helper method for drawing a color in SRC mode, completely replacing all the pixels
   in the current clip with this color.
 */
SK_C_API void sk_canvas_clear(sk_canvas_t*, sk_color_t);
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
SK_C_API void sk_canvas_discard(sk_canvas_t*);
/**
    Returns the number of matrix/clip states on the SkCanvas' private stack.
    This will equal # save() calls - # restore() calls + 1. The save count on
    a new canvas is 1.
*/
SK_C_API int sk_canvas_get_save_count(sk_canvas_t*);
/**
    Efficient way to pop any calls to sk_canvas_save() that happened after the save
    count reached saveCount. It is an error for saveCount to be greater than
    getSaveCount(). To pop all the way back to the initial matrix/clip context
    pass saveCount == 1.
*/
SK_C_API void sk_canvas_restore_to_count(sk_canvas_t*, int saveCount);
/**
    Draws with the specified color and mode.
**/
SK_C_API void sk_canvas_draw_color(sk_canvas_t* ccanvas, sk_color_t color, sk_blendmode_t mode);
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
SK_C_API void sk_canvas_draw_points(sk_canvas_t*, sk_point_mode_t, size_t, const sk_point_t[], const sk_paint_t*);
/**
   Draws a single point with the specified paint
*/
SK_C_API void sk_canvas_draw_point(sk_canvas_t*, float, float, const sk_paint_t*);
/**
   Draws a line from x0,y0 to x1,y1
*/
SK_C_API void sk_canvas_draw_line(sk_canvas_t* ccanvas, float x0, float y0, float x1, float y1, sk_paint_t* cpaint);
/**
    Draw the text, with origin at (x,y), using the specified paint.
    The origin is interpreted based on the Align setting in the paint.

    @param text The text to be drawn
    @param byteLength   The number of bytes to read from the text parameter
    @param x        The x-coordinate of the origin of the text being drawn
    @param y        The y-coordinate of the origin of the text being drawn
    @param paint    The paint used for the text (e.g. color, size, style)
*/
SK_C_API void sk_canvas_draw_text (sk_canvas_t*, const char *text, size_t byteLength, float x, float y, const sk_paint_t* paint);
/**
    Draw the text, with each character/glyph origin specified by the pos[]
    array. The origin is interpreted by the Align setting in the paint.

    @param text The text to be drawn
    @param byteLength   The number of bytes to read from the text parameter
    @param pos      Array of positions, used to position each character
    @param paint    The paint used for the text (e.g. color, size, style)
*/
SK_C_API void sk_canvas_draw_pos_text (sk_canvas_t*, const char *text, size_t byteLength, const sk_point_t[], const sk_paint_t* paint);
/**
    Draw the text, with origin at (x,y), using the specified paint, along
    the specified path. The paint's Align setting determins where along the
    path to start the text.

    @param text The text to be drawn
    @param byteLength   The number of bytes to read from the text parameter
    @param path         The path the text should follow for its baseline
    @param hOffset      The distance along the path to add to the text's
                        starting position
    @param vOffset      The distance above(-) or below(+) the path to
                        position the text
    @param paint        The paint used for the text
*/
SK_C_API void sk_canvas_draw_text_on_path (sk_canvas_t*, const char *text, size_t byteLength, const sk_path_t*path, float hOffset, float vOffset, const sk_paint_t* paint);
/** 
    Draw the specified bitmap, with its top/left corner at (x,y), using the
    specified paint, transformed by the current matrix. Note: if the paint
    contains a maskfilter that generates a mask which extends beyond the
    bitmap's original width/height, then the bitmap will be drawn as if it
    were in a Shader with CLAMP mode. Thus the color outside of the original
    width/height will be the edge color replicated.

    If a shader is present on the paint it will be ignored, except in the
    case where the bitmap is kAlpha_8_SkColorType. In that case, the color is
    generated by the shader.

    @param bitmap   The bitmap to be drawn
    @param left     The position of the left side of the bitmap being drawn
    @param top      The position of the top side of the bitmap being drawn
    @param paint    The paint used to draw the bitmap, or NULL
*/
SK_C_API void sk_canvas_draw_bitmap(sk_canvas_t* ccanvas, const sk_bitmap_t* bitmap, float left, float top, const sk_paint_t* paint);
/** Draw the specified bitmap, scaling and translating so that it fills the specified
    dst rect. If the src rect is non-null, only that subset of the bitmap is transformed
    and drawn.
  
    @param bitmap     The bitmap to be drawn
    @param src        Optional: specify the subset of the bitmap to be drawn
    @param dst        The destination rectangle where the scaled/translated
                      bitmap will be drawn
    @param paint      The paint used to draw the bitmap, or NULL
*/
SK_C_API void sk_canvas_draw_bitmap_rect(sk_canvas_t* ccanvas, const sk_bitmap_t* bitmap, const sk_rect_t* src, const sk_rect_t* dst, const sk_paint_t* paint);
/**
    Helper for setMatrix(identity). Sets the current matrix to identity.
*/
SK_C_API void sk_canvas_reset_matrix(sk_canvas_t* ccanvas);
/**
    Replace the current matrix with a copy of the specified matrix.

    @param matrix The matrix that will be copied into the current matrix.
*/
SK_C_API void sk_canvas_set_matrix(sk_canvas_t* ccanvas, const sk_matrix_t* matrix);
/**
    Return the current matrix on the canvas.
    This does not account for the translate in any of the devices.

    @param matrix The current matrix on the canvas.
*/
SK_C_API void sk_canvas_get_total_matrix(sk_canvas_t* ccanvas, sk_matrix_t* matrix);
/**
    Draw the specified rounded rectangle using the specified paint. The
    rectangle will be filled or stroked based on the style in the
    paint.
*/
SK_C_API void sk_canvas_draw_round_rect(sk_canvas_t*, const sk_rect_t*, float rx, float ry, const sk_paint_t*);
/**
    Modify the current clip with the specified rectangle.
*/
SK_C_API void sk_canvas_clip_rect_with_operation(sk_canvas_t* t, const sk_rect_t* crect, sk_clipop_t op, bool doAA);
/**
    Modify the current clip with the specified path.
*/
SK_C_API void sk_canvas_clip_path_with_operation(sk_canvas_t* t, const sk_path_t* crect, sk_clipop_t op, bool doAA);

/**
    Return the bounds of the current clip (in local coordinates) in the
    bounds parameter, and return true if it is non-empty. This can be useful
    in a way similar to quickReject, in that it tells you that drawing
    outside of these bounds will be clipped out.
*/
SK_C_API bool sk_canvas_get_local_clip_bounds(sk_canvas_t* t, sk_rect_t* cbounds);
/**
    Return the bounds of the current clip, in device coordinates; returns
    true if non-empty. Maybe faster than getting the clip explicitly and
    then taking its bounds.
*/
SK_C_API bool sk_canvas_get_device_clip_bounds(sk_canvas_t* t, sk_irect_t* cbounds);

/**
    Trigger the immediate execution of all pending draw operations. For the GPU
    backend this will resolve all rendering to the GPU surface backing the
    SkSurface that owns this canvas.
*/
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

SK_C_PLUS_PLUS_END_GUARD

#endif
