/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_typeface_DEFINED
#define sk_typeface_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_API sk_typeface_t* sk_typeface_create_from_name(const char *familyName, sk_typeface_style_t style);
SK_API sk_typeface_t* sk_typeface_create_from_name_with_font_style(const char *familyName, int weight, int width, sk_font_style_slant_t slant);
SK_API void sk_typeface_unref(sk_typeface_t*);
SK_API sk_typeface_t* sk_typeface_create_from_typeface(sk_typeface_t* typeface, sk_typeface_style_t sstyle);
SK_API sk_typeface_t* sk_typeface_create_from_file(const char* path, int index);
SK_API sk_typeface_t* sk_typeface_create_from_stream(sk_stream_asset_t* stream, int index);
SK_API int sk_typeface_chars_to_glyphs(sk_typeface_t* typeface, const char *chars, sk_encoding_t encoding, uint16_t glyphs[], int glyphCount);

SK_API sk_string_t* sk_typeface_get_family_name(sk_typeface_t* typeface);

SK_API int sk_typeface_count_tables(sk_typeface_t* typeface);
SK_API int sk_typeface_get_table_tags(sk_typeface_t* typeface, sk_font_table_tag_t tags[]);
SK_API size_t sk_typeface_get_table_size(sk_typeface_t* typeface, sk_font_table_tag_t tag);
SK_API size_t sk_typeface_get_table_data(sk_typeface_t* typeface, sk_font_table_tag_t tag, size_t offset, size_t length, void* data);

SK_C_PLUS_PLUS_END_GUARD

#endif
