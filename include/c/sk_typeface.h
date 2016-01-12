/*
 * Copyright 2015 Xamarin Inc.
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

typedef enum {
	NORMAL_TYPEFACE_STYLE = 0,
	BOLD_TYPEFACE_STYLE = 1,
	ITALIC_TYPEFACE_STYLE = 2,
	BOLD_ITALIC_TYPEFACE_STYLE = 3
} sk_typeface_style_t;

/**
*/
SK_API sk_typeface_t* sk_typeface_create_from_name(const char *familyName, sk_typeface_style_t style);
/**
*/
SK_API void sk_typeface_unref(sk_typeface_t*);

SK_API sk_typeface_t* sk_typeface_create_from_typeface(sk_typeface_t* typeface, sk_typeface_style_t sstyle);

SK_API sk_typeface_t* sk_typeface_create_from_file(const char* path, int index);

SK_API sk_typeface_t* sk_typeface_create_from_stream(sk_stream_asset_t* stream, int index);

SK_API int sk_typeface_chars_to_glyphs (sk_typeface_t* typeface, const char *chars, sk_encoding_t encoding, uint16_t glyphs [], int glyphCount);

SK_C_PLUS_PLUS_END_GUARD

#endif
