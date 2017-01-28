/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_maskfilter_DEFINED
#define sk_maskfilter_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

/**
    Increment the reference count on the given sk_maskfilter_t. Must be
    balanced by a call to sk_maskfilter_unref().
*/
SK_C_API void sk_maskfilter_ref(sk_maskfilter_t*);
/**
    Decrement the reference count. If the reference count is 1 before
    the decrement, then release both the memory holding the
    sk_maskfilter_t and any other associated resources.  New
    sk_maskfilter_t are created with a reference count of 1.
*/
SK_C_API void sk_maskfilter_unref(sk_maskfilter_t*);

/**
    Create a blur maskfilter.
    @param sk_blurstyle_t The SkBlurStyle to use
    @param sigma Standard deviation of the Gaussian blur to apply. Must be > 0.
*/
SK_C_API sk_maskfilter_t* sk_maskfilter_new_blur(sk_blurstyle_t, float sigma);

/** Create an emboss maskfilter
    @param blurSigma    standard deviation of the Gaussian blur to apply
                        before applying lighting (e.g. 3)
    @param direction    array of 3 scalars [x, y, z] specifying the direction of the light source
    @param ambient      0...1 amount of ambient light
    @param specular     coefficient for specular highlights (e.g. 8)
    @return the emboss maskfilter
*/
SK_C_API sk_maskfilter_t* sk_maskfilter_new_emboss(
    float blurSigma,
    const float direction[3],
    float ambient, 
    float specular);

SK_C_API sk_maskfilter_t* sk_maskfilter_new_table(
    const uint8_t table[256]);

SK_C_API sk_maskfilter_t* sk_maskfilter_new_gamma(
    float gamma);

SK_C_API sk_maskfilter_t* sk_maskfilter_new_clip(
    uint8_t min,
    uint8_t max);

SK_C_API sk_maskfilter_t* sk_maskfilter_new_shadow(float occluderHeight, const sk_point3_t* lightPos, float lightRadius, float ambientAlpha, float spotAlpha, sk_shadowmaskfilter_shadowflags_t flags);

SK_C_PLUS_PLUS_END_GUARD

#endif
