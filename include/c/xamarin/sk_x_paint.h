/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_x_paint_DEFINED
#define sk_x_paint_DEFINED

#include "sk_types.h"
#include "xamarin/sk_x_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

/**
 Return true iff the paint has dithering enabled.
 */
SK_API bool sk_paint_is_dither(const sk_paint_t*);
/**
 Set to true to enable dithering, false to disable it on this
 sk_paint_t.
 */
SK_API void sk_paint_set_dither(sk_paint_t*, bool);
/**
 Return true iff the paint has verticaltext enabled.
 */
SK_API bool sk_paint_is_verticaltext(const sk_paint_t*);
/**
 Set to true to enable verticaltext, false to disable it on this
 sk_paint_t.
 */
SK_API void sk_paint_set_verticaltext(sk_paint_t*, bool);
SK_API sk_shader_t* sk_paint_get_shader(sk_paint_t*);
SK_API sk_maskfilter_t* sk_paint_get_maskfilter(sk_paint_t*);
SK_API void sk_paint_set_colorfilter(sk_paint_t*, sk_colorfilter_t*);
SK_API sk_colorfilter_t* sk_paint_get_colorfilter(sk_paint_t*);
SK_API void sk_paint_set_imagefilter(sk_paint_t*, sk_imagefilter_t*);
SK_API sk_imagefilter_t* sk_paint_get_imagefilter(sk_paint_t*);
SK_API sk_xfermode_mode_t sk_paint_get_xfermode_mode(sk_paint_t*);
/**
 * Get or set the paint's filter quality.
*/
SK_API void sk_paint_set_filter_quality(sk_paint_t*, sk_filter_quality_t);
SK_API sk_filter_quality_t sk_paint_get_filter_quality(sk_paint_t *);
/**
 *  Get the paint's typeface
 */
SK_API sk_typeface_t* sk_paint_get_typeface(sk_paint_t*);
/**
 *  Set the paint's typeface
 */
SK_API void sk_paint_set_typeface(sk_paint_t*, sk_typeface_t*);
/**
 *  Get the paint's text sixe
 */
SK_API float sk_paint_get_textsize(sk_paint_t*);
/**
 *  Set the paint's text sixe
 */
SK_API void sk_paint_set_textsize(sk_paint_t*, float);
/**
 *  Get the paint's text alignment
 */
SK_API sk_text_align_t sk_paint_get_text_align(const sk_paint_t*);
/**
 *  Set the paint's text alignment
 */
SK_API void sk_paint_set_text_align(sk_paint_t*, sk_text_align_t);
/**
 *  Get the paint's text encoding
 */
SK_API sk_text_encoding_t sk_paint_get_text_encoding(const sk_paint_t*);
/**
 *  Set the paint's text encoding
 */
SK_API void sk_paint_set_text_encoding(sk_paint_t*, sk_text_encoding_t);
/**
 *  Set the paint's horizontal scale factor for text
 */
SK_API float sk_paint_get_text_scale_x(const sk_paint_t* cpaint);
/**
 *  Set the paint's horizontal scale factor for text
 */
SK_API void sk_paint_set_text_scale_x(sk_paint_t* cpaint, float scale);
/**
 *  Set the paint's horizontal skew factor for text
 */
SK_API float sk_paint_get_text_skew_x(const sk_paint_t* cpaint);
/**
 *  Set the paint's horizontal skew factor for text
 */
SK_API void sk_paint_set_text_skew_x(sk_paint_t* cpaint, float skew);
/**
 *  Return the number of bytes of text that were measured
 */
SK_API size_t sk_paint_break_text(const sk_paint_t* cpaint, const void* text, size_t length, float maxWidth, float* measuredWidth);
/**
 *  Return the width of the text
 */
SK_API float sk_paint_measure_text(const sk_paint_t* cpaint, const void* text, size_t length, sk_rect_t* cbounds);
/**
 *  Get the path outline of text.
 */
SK_API sk_path_t* sk_paint_get_text_path(sk_paint_t* cpaint, const void* text, size_t length, float x, float y);
/**
 *  Get the path outline of text with each glyph positioned.
 */
SK_API sk_path_t* sk_paint_get_pos_text_path(sk_paint_t* cpaint, const void* text, size_t length, const sk_point_t pos[]);
/**
 * Return the recommend spacing between lines (which will be fDescent - fAscent + fLeading). 
 * Also get the font metrics for the current typeface and type size if cfontmetrics is not null.
 */
SK_API float sk_paint_get_fontmetrics(sk_paint_t* cpaint, sk_fontmetrics_t* cfontmetrics, float scale);

SK_C_PLUS_PLUS_END_GUARD

#endif
