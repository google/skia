/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_font_DEFINED
#define sk_font_DEFINED

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_C_API sk_font_t* sk_font_new();
SK_C_API sk_font_t* sk_font_new_with_values(sk_typeface_t* typeface, float size, float scaleX, float skewX);
SK_C_API void sk_font_delete(sk_font_t* font);
SK_C_API bool sk_font_is_force_auto_hinting(const sk_font_t* font);
SK_C_API void sk_font_set_force_auto_hinting(sk_font_t* font, bool value);
SK_C_API bool sk_font_is_embedded_bitmaps(const sk_font_t* font);
SK_C_API void sk_font_set_embedded_bitmaps(sk_font_t* font, bool value);
SK_C_API bool sk_font_is_subpixel(const sk_font_t* font);
SK_C_API void sk_font_set_subpixel(sk_font_t* font, bool value);
SK_C_API bool sk_font_is_linear_metrics(const sk_font_t* font);
SK_C_API void sk_font_set_linear_metrics(sk_font_t* font, bool value);
SK_C_API bool sk_font_is_embolden(const sk_font_t* font);
SK_C_API void sk_font_set_embolden(sk_font_t* font, bool value);
SK_C_API bool sk_font_is_baseline_snap(const sk_font_t* font);
SK_C_API void sk_font_set_baseline_snap(sk_font_t* font, bool value);
SK_C_API sk_font_edging_t sk_font_get_edging(const sk_font_t* font);
SK_C_API void sk_font_set_edging(sk_font_t* font, sk_font_edging_t value);
SK_C_API sk_font_hinting_t sk_font_get_hinting(const sk_font_t* font);
SK_C_API void sk_font_set_hinting(sk_font_t* font, sk_font_hinting_t value);
SK_C_API sk_typeface_t* sk_font_get_typeface(const sk_font_t* font);
SK_C_API void sk_font_set_typeface(sk_font_t* font, sk_typeface_t* value);
SK_C_API float sk_font_get_size(const sk_font_t* font);
SK_C_API void sk_font_set_size(sk_font_t* font, float value);
SK_C_API float sk_font_get_scale_x(const sk_font_t* font);
SK_C_API void sk_font_set_scale_x(sk_font_t* font, float value);
SK_C_API float sk_font_get_skew_x(const sk_font_t* font);
SK_C_API void sk_font_set_skew_x(sk_font_t* font, float value);
SK_C_API int sk_font_text_to_glyphs(const sk_font_t* font, const void* text, size_t byteLength, sk_text_encoding_t encoding, uint16_t glyphs[], int maxGlyphCount);
SK_C_API uint16_t sk_font_unichar_to_glyph(const sk_font_t* font, int32_t uni);
SK_C_API void sk_font_unichars_to_glyphs(const sk_font_t* font, const int32_t uni[], int count, uint16_t glyphs[]);
SK_C_API float sk_font_measure_text(const sk_font_t* font, const void* text, size_t byteLength, sk_text_encoding_t encoding, sk_rect_t* bounds, const sk_paint_t* paint);
SK_C_API void sk_font_get_widths_bounds(const sk_font_t* font, const uint16_t glyphs[], int count, float widths[], sk_rect_t bounds[], const sk_paint_t* paint);
SK_C_API float sk_font_get_metrics(const sk_font_t* font, sk_fontmetrics_t* metrics);

SK_C_PLUS_PLUS_END_GUARD

#endif
