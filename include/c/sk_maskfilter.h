/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_maskfilter_DEFINED
#define sk_maskfilter_DEFINED

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_C_API void sk_maskfilter_ref(sk_maskfilter_t*);
SK_C_API void sk_maskfilter_unref(sk_maskfilter_t*);
SK_C_API sk_maskfilter_t* sk_maskfilter_new_blur(sk_blurstyle_t, float sigma);
SK_C_API sk_maskfilter_t* sk_maskfilter_new_blur_with_flags(sk_blurstyle_t, float sigma, const sk_rect_t* occluder, bool respectCTM);
SK_C_API sk_maskfilter_t* sk_maskfilter_new_table(const uint8_t table[256]);
SK_C_API sk_maskfilter_t* sk_maskfilter_new_gamma(float gamma);
SK_C_API sk_maskfilter_t* sk_maskfilter_new_clip(uint8_t min, uint8_t max);

SK_C_PLUS_PLUS_END_GUARD

#endif
