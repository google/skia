/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../../include/effects/SkBlurMaskFilter.h"
#include "../../include/effects/SkTableMaskFilter.h"

#include "xamarin/sk_x_maskfilter.h"

#include "../sk_types_priv.h"
#include "sk_x_types_priv.h"

sk_maskfilter_t* sk_maskfilter_new_emboss(
    float blurSigma, 
    const float direction[3],
    float ambient, 
    float specular) {
    return ToMaskFilter(SkBlurMaskFilter::CreateEmboss(blurSigma, direction, ambient, specular));
}

sk_maskfilter_t* sk_maskfilter_new_table(const uint8_t table[256]) {
    return ToMaskFilter(SkTableMaskFilter::Create(table));
}

sk_maskfilter_t* sk_maskfilter_new_gamma(float gamma) {
    return ToMaskFilter(SkTableMaskFilter::CreateGamma(gamma));
}

sk_maskfilter_t* sk_maskfilter_new_clip(uint8_t min, uint8_t max) {
    return ToMaskFilter(SkTableMaskFilter::CreateClip(min, max));
}
