/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_paint_DEFINED
#define sk_paint_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

/**
    Create a new paint with default settings:
        antialias : false
        stroke : false
        stroke width : 0.0f (hairline)
        stroke miter : 4.0f
        stroke cap : BUTT_SK_STROKE_CAP
        stroke join : MITER_SK_STROKE_JOIN
        color : opaque black
        shader : NULL
        maskfilter : NULL
        xfermode_mode : SRCOVER_SK_XFERMODE_MODE
*/
SK_C_API sk_paint_t* sk_paint_new(void);
SK_C_API sk_paint_t* sk_paint_clone(sk_paint_t*);
/**
    Release the memory storing the sk_paint_t and unref() all
    associated objects.
*/
SK_C_API void sk_paint_delete(sk_paint_t*);
/**
    Return true iff the paint has antialiasing enabled.
*/
SK_C_API bool sk_paint_is_antialias(const sk_paint_t*);
/**
    Set to true to enable antialiasing, false to disable it on this
    sk_paint_t.
*/
SK_C_API void sk_paint_set_antialias(sk_paint_t*, bool);
/**
    Return the paint's curent drawing color.
*/
SK_C_API sk_color_t sk_paint_get_color(const sk_paint_t*);
/**
    Set the paint's curent drawing color.
*/
SK_C_API void sk_paint_set_color(sk_paint_t*, sk_color_t);
/**
    Return true iff stroking is enabled rather than filling on this
    sk_paint_t.
*/
SK_C_API sk_paint_style_t sk_paint_get_style(const sk_paint_t*);
/**
    Set to true to enable stroking rather than filling with this
    sk_paint_t.
*/
SK_C_API void sk_paint_set_style(sk_paint_t*, sk_paint_style_t);
/**
    Return the width for stroking.  A value of 0 strokes in hairline mode.
 */
SK_C_API float sk_paint_get_stroke_width(const sk_paint_t*);
/**
   Set the width for stroking.  A value of 0 strokes in hairline mode
   (always draw 1-pixel wide, regardless of the matrix).
 */
SK_C_API void sk_paint_set_stroke_width(sk_paint_t*, float width);
/**
    Return the paint's stroke miter value. This is used to control the
    behavior of miter joins when the joins angle is sharp.
*/
SK_C_API float sk_paint_get_stroke_miter(const sk_paint_t*);
/**
   Set the paint's stroke miter value. This is used to control the
   behavior of miter joins when the joins angle is sharp. This value
   must be >= 0.
*/
SK_C_API void sk_paint_set_stroke_miter(sk_paint_t*, float miter);
/**
    Return the paint's stroke cap type, controlling how the start and
    end of stroked lines and paths are treated.
*/
SK_C_API sk_stroke_cap_t sk_paint_get_stroke_cap(const sk_paint_t*);
/**
    Set the paint's stroke cap type, controlling how the start and
    end of stroked lines and paths are treated.
*/
SK_C_API void sk_paint_set_stroke_cap(sk_paint_t*, sk_stroke_cap_t);
/**
    Return the paint's stroke join type, specifies the treatment that
    is applied to corners in paths and rectangles
 */
SK_C_API sk_stroke_join_t sk_paint_get_stroke_join(const sk_paint_t*);
/**
    Set the paint's stroke join type, specifies the treatment that
    is applied to corners in paths and rectangles
 */
SK_C_API void sk_paint_set_stroke_join(sk_paint_t*, sk_stroke_join_t);
/**
 *  Set the paint's shader to the specified parameter. This will automatically call unref() on
 *  any previous value, and call ref() on the new value.
 */
SK_C_API void sk_paint_set_shader(sk_paint_t*, sk_shader_t*);
/**
 *  Set the paint's maskfilter to the specified parameter. This will automatically call unref() on
 *  any previous value, and call ref() on the new value.
 */
SK_C_API void sk_paint_set_maskfilter(sk_paint_t*, sk_maskfilter_t*);
/**
 *  Set the paint's blend mode to the specified parameter.
 */
SK_C_API void sk_paint_set_blendmode(sk_paint_t*, sk_blendmode_t);
/**
 *  Return true iff the paint has dithering enabled.
 */
SK_C_API bool sk_paint_is_dither(const sk_paint_t*);
/**
 *  Set to true to enable dithering, false to disable it on this
 *  sk_paint_t.
 */
SK_C_API void sk_paint_set_dither(sk_paint_t*, bool);
/**
 *  Return true iff the paint has verticaltext enabled.
 */
SK_C_API bool sk_paint_is_verticaltext(const sk_paint_t*);
/**
 *  Set to true to enable verticaltext, false to disable it on this
 *  sk_paint_t.
 */
SK_C_API void sk_paint_set_verticaltext(sk_paint_t*, bool);
/**
 *  Get the paint's shader object.
 */
SK_C_API sk_shader_t* sk_paint_get_shader(sk_paint_t*);
/**
 *  Get the paint's mask filter object.
 */
SK_C_API sk_maskfilter_t* sk_paint_get_maskfilter(sk_paint_t*);
/**
 *  Set or clear the paint's color filter.
 */
SK_C_API void sk_paint_set_colorfilter(sk_paint_t*, sk_colorfilter_t*);
/**
 *  Get the paint's color filter object.
 */
SK_C_API sk_colorfilter_t* sk_paint_get_colorfilter(sk_paint_t*);
/**
 *  Set or clear the paint's image filter.
 */
SK_C_API void sk_paint_set_imagefilter(sk_paint_t*, sk_imagefilter_t*);
/**
 *  Get the paint's image filter object.
 */
SK_C_API sk_imagefilter_t* sk_paint_get_imagefilter(sk_paint_t*);
/**
 *  Get the paint's blend mode.
 */
SK_C_API sk_blendmode_t sk_paint_get_blendmode(sk_paint_t*);
/**
 *  Set the paint's filter quality.
 */
SK_C_API void sk_paint_set_filter_quality(sk_paint_t*, sk_filter_quality_t);
/**
 *  Get the paint's filter quality.
 */
SK_C_API sk_filter_quality_t sk_paint_get_filter_quality(sk_paint_t *);
/**
 *  Get the paint's typeface
 */
SK_C_API sk_typeface_t* sk_paint_get_typeface(sk_paint_t*);
/**
 *  Set the paint's typeface
 */
SK_C_API void sk_paint_set_typeface(sk_paint_t*, sk_typeface_t*);
/**
 *  Get the paint's text sixe
 */
SK_C_API float sk_paint_get_textsize(sk_paint_t*);
/**
 *  Set the paint's text sixe
 */
SK_C_API void sk_paint_set_textsize(sk_paint_t*, float);
/**
 *  Get the paint's text alignment
 */
SK_C_API sk_text_align_t sk_paint_get_text_align(const sk_paint_t*);
/**
 *  Set the paint's text alignment
 */
SK_C_API void sk_paint_set_text_align(sk_paint_t*, sk_text_align_t);
/**
 *  Get the paint's text encoding
 */
SK_C_API sk_text_encoding_t sk_paint_get_text_encoding(const sk_paint_t*);
/**
 *  Set the paint's text encoding
 */
SK_C_API void sk_paint_set_text_encoding(sk_paint_t*, sk_text_encoding_t);
/**
 *  Set the paint's horizontal scale factor for text
 */
SK_C_API float sk_paint_get_text_scale_x(const sk_paint_t* cpaint);
/**
 *  Set the paint's horizontal scale factor for text
 */
SK_C_API void sk_paint_set_text_scale_x(sk_paint_t* cpaint, float scale);
/**
 *  Set the paint's horizontal skew factor for text
 */
SK_C_API float sk_paint_get_text_skew_x(const sk_paint_t* cpaint);
/**
 *  Set the paint's horizontal skew factor for text
 */
SK_C_API void sk_paint_set_text_skew_x(sk_paint_t* cpaint, float skew);
/**
 *  Return the number of bytes of text that were measured
 */
SK_C_API size_t sk_paint_break_text(const sk_paint_t* cpaint, const void* text, size_t length, float maxWidth, float* measuredWidth);
/**
 *  Return the width of the text
 */
SK_C_API float sk_paint_measure_text(const sk_paint_t* cpaint, const void* text, size_t length, sk_rect_t* cbounds);
/**
 *  Get the path outline of text.
 */
SK_C_API sk_path_t* sk_paint_get_text_path(sk_paint_t* cpaint, const void* text, size_t length, float x, float y);
/**
 *  Get the path outline of text with each glyph positioned.
 */
SK_C_API sk_path_t* sk_paint_get_pos_text_path(sk_paint_t* cpaint, const void* text, size_t length, const sk_point_t pos[]);
/**
 * Return the recommend spacing between lines (which will be fDescent - fAscent + fLeading). 
 * Also get the font metrics for the current typeface and type size if cfontmetrics is not null.
 */
SK_C_API float sk_paint_get_fontmetrics(sk_paint_t* cpaint, sk_fontmetrics_t* cfontmetrics, float scale);
/**
 *  Return the paint's patheffect object  
 */  
SK_C_API sk_path_effect_t* sk_paint_get_path_effect(sk_paint_t* cpaint);
/**
 *  Sets the paint's patheffect object  
 */  
SK_C_API void sk_paint_set_path_effect(sk_paint_t* cpaint, sk_path_effect_t* effect);  

SK_C_API bool sk_paint_is_linear_text(const sk_paint_t*);
SK_C_API void sk_paint_set_linear_text(sk_paint_t*, bool);

SK_C_API bool sk_paint_is_subpixel_text(const sk_paint_t*);
SK_C_API void sk_paint_set_subpixel_text(sk_paint_t*, bool);

SK_C_API bool sk_paint_is_lcd_render_text(const sk_paint_t*);
SK_C_API void sk_paint_set_lcd_render_text(sk_paint_t*, bool);

SK_C_API bool sk_paint_is_embedded_bitmap_text(const sk_paint_t*);
SK_C_API void sk_paint_set_embedded_bitmap_text(sk_paint_t*, bool);

SK_C_API sk_paint_hinting_t sk_paint_get_hinting(const sk_paint_t*);
SK_C_API void sk_paint_set_hinting(sk_paint_t*, sk_paint_hinting_t);

SK_C_API bool sk_paint_is_autohinted(const sk_paint_t*);
SK_C_API void sk_paint_set_autohinted(sk_paint_t*, bool);

SK_C_API bool sk_paint_is_fake_bold_text(const sk_paint_t*);
SK_C_API void sk_paint_set_fake_bold_text(sk_paint_t*, bool);

SK_C_API bool sk_paint_is_dev_kern_text(const sk_paint_t*);
SK_C_API void sk_paint_set_dev_kern_text(sk_paint_t*, bool);

SK_C_API bool sk_paint_get_fill_path(const sk_paint_t*, const sk_path_t* src, sk_path_t* dst, const sk_rect_t* cullRect, float resScale);

SK_C_PLUS_PLUS_END_GUARD

#endif
