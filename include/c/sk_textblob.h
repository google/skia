/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_textblob_DEFINED
#define sk_textblob_DEFINED

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_C_API void sk_textblob_ref(const sk_textblob_t* blob);
SK_C_API void sk_textblob_unref(const sk_textblob_t* blob);
SK_C_API uint32_t sk_textblob_get_unique_id(const sk_textblob_t* blob);
SK_C_API void sk_textblob_get_bounds(const sk_textblob_t* blob, sk_rect_t* bounds);

SK_C_API sk_textblob_builder_t* sk_textblob_builder_new();
SK_C_API void sk_textblob_builder_delete(sk_textblob_builder_t* builder);
SK_C_API sk_textblob_t* sk_textblob_builder_make(sk_textblob_builder_t* builder);
SK_C_API void sk_textblob_builder_alloc_run(sk_textblob_builder_t* builder, const sk_font_t* font, int count, float x, float y, const sk_rect_t* bounds, sk_textblob_builder_runbuffer_t* runbuffer);
SK_C_API void sk_textblob_builder_alloc_run_pos_h(sk_textblob_builder_t* builder, const sk_font_t* font, int count, float y, const sk_rect_t* bounds, sk_textblob_builder_runbuffer_t* runbuffer);
SK_C_API void sk_textblob_builder_alloc_run_pos(sk_textblob_builder_t* builder, const sk_font_t* font, int count, const sk_rect_t* bounds, sk_textblob_builder_runbuffer_t* runbuffer);

SK_C_PLUS_PLUS_END_GUARD

#endif
