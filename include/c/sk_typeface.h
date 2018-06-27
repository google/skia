/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_typeface_DEFINED
#define sk_typeface_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_C_API sk_typeface_t* sk_typeface_create_from_name(const char *familyName, sk_typeface_style_t style);
SK_C_API sk_typeface_t* sk_typeface_create_from_name_with_font_style(const char *familyName, int weight, int width, sk_font_style_slant_t slant);
SK_C_API void sk_typeface_unref(sk_typeface_t*);
SK_C_API sk_typeface_t* sk_typeface_create_from_typeface(sk_typeface_t* typeface, sk_typeface_style_t sstyle);
SK_C_API sk_typeface_t* sk_typeface_create_from_file(const char* path, int index);
SK_C_API sk_typeface_t* sk_typeface_create_from_stream(sk_stream_asset_t* stream, int index);
SK_C_API int sk_typeface_chars_to_glyphs(sk_typeface_t* typeface, const char *chars, sk_encoding_t encoding, uint16_t glyphs[], int glyphCount);
SK_C_API sk_stream_asset_t* sk_typeface_open_stream(sk_typeface_t* typeface, int* ttcIndex);
SK_C_API int sk_typeface_get_units_per_em(sk_typeface_t* typeface);

SK_C_API sk_string_t* sk_typeface_get_family_name(sk_typeface_t* typeface);
SK_C_API int sk_typeface_get_font_weight(sk_typeface_t* typeface);
SK_C_API int sk_typeface_get_font_width(sk_typeface_t* typeface);
SK_C_API sk_font_style_slant_t sk_typeface_get_font_slant(sk_typeface_t* typeface);
SK_C_API sk_typeface_style_t sk_typeface_get_style(sk_typeface_t* typeface);

SK_C_API int sk_typeface_count_tables(sk_typeface_t* typeface);
SK_C_API int sk_typeface_get_table_tags(sk_typeface_t* typeface, sk_font_table_tag_t tags[]);
SK_C_API size_t sk_typeface_get_table_size(sk_typeface_t* typeface, sk_font_table_tag_t tag);
SK_C_API size_t sk_typeface_get_table_data(sk_typeface_t* typeface, sk_font_table_tag_t tag, size_t offset, size_t length, void* data);

SK_C_API sk_fontmgr_t* sk_fontmgr_ref_default(void);
SK_C_API void sk_fontmgr_unref(sk_fontmgr_t*);
SK_C_API int sk_fontmgr_count_families(sk_fontmgr_t*);
SK_C_API void sk_fontmgr_get_family_name(sk_fontmgr_t*, int index, sk_string_t* familyName);
SK_C_API sk_typeface_t* sk_fontmgr_match_family_style_character(sk_fontmgr_t*, const char* familyName, int weight, int width, sk_font_style_slant_t slant, const char** bcp47, int bcp47Count, int32_t character);

SK_C_PLUS_PLUS_END_GUARD

#endif
