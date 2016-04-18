/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_x_maskfilter_DEFINED
#define sk_x_maskfilter_DEFINED

#include "sk_types.h"
#include "xamarin\sk_x_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_API sk_maskfilter_t* sk_maskfilter_new_emboss(
    float blurSigma,
    const float direction[3],
    float ambient, 
    float specular);
SK_API sk_maskfilter_t* sk_maskfilter_new_table(
    const uint8_t table[256]);
SK_API sk_maskfilter_t* sk_maskfilter_new_gamma(
    float gamma);
SK_API sk_maskfilter_t* sk_maskfilter_new_clip(
    uint8_t min,
    uint8_t max);

SK_C_PLUS_PLUS_END_GUARD

#endif
