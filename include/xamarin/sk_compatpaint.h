/*
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_compatpaint_DEFINED
#define sk_compatpaint_DEFINED

#include "sk_xamarin.h"

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

typedef struct sk_compatpaint_t sk_compatpaint_t;

SK_X_API sk_compatpaint_t* sk_compatpaint_new(void);
SK_X_API sk_compatpaint_t* sk_compatpaint_new_with_font(const sk_font_t* font);
SK_X_API void sk_compatpaint_delete(sk_compatpaint_t* paint);
SK_X_API sk_compatpaint_t* sk_compatpaint_clone(const sk_compatpaint_t* paint);
SK_X_API void sk_compatpaint_reset(sk_compatpaint_t* paint);
SK_X_API sk_font_t* sk_compatpaint_make_font(sk_compatpaint_t* paint);
SK_X_API sk_font_t* sk_compatpaint_get_font(sk_compatpaint_t* paint);
SK_X_API void sk_compatpaint_set_text_align(sk_compatpaint_t* paint, sk_text_align_t align);
SK_X_API sk_text_align_t sk_compatpaint_get_text_align(const sk_compatpaint_t* paint);
SK_X_API void sk_compatpaint_set_text_encoding(sk_compatpaint_t* paint, sk_text_encoding_t encoding);
SK_X_API sk_text_encoding_t sk_compatpaint_get_text_encoding(const sk_compatpaint_t* paint);

SK_C_PLUS_PLUS_END_GUARD

#endif
