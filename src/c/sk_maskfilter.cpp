/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlurMaskFilter.h"
#include "SkTableMaskFilter.h"
#include "SkShadowMaskFilter.h"

#include "sk_maskfilter.h"

#include "sk_types_priv.h"

sk_maskfilter_t* sk_maskfilter_new_emboss(
    float blurSigma, 
    const float direction[3],
    float ambient, 
    float specular) {
    return ToMaskFilter(SkBlurMaskFilter::MakeEmboss(blurSigma, direction, ambient, specular).release());
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

void sk_maskfilter_ref(sk_maskfilter_t* cfilter) {
    SkSafeRef(AsMaskFilter(cfilter));
}

void sk_maskfilter_unref(sk_maskfilter_t* cfilter) {
    SkSafeUnref(AsMaskFilter(cfilter));
}

sk_maskfilter_t* sk_maskfilter_new_blur(sk_blurstyle_t cstyle, float sigma) {
    return ToMaskFilter(SkBlurMaskFilter::Make((SkBlurStyle)cstyle, sigma).release());
}

sk_maskfilter_t* sk_maskfilter_new_shadow(float occluderHeight, const sk_point3_t* lightPos, float lightRadius, float ambientAlpha, float spotAlpha, sk_shadowmaskfilter_shadowflags_t flags) {
    return ToMaskFilter(SkShadowMaskFilter::Make(occluderHeight, AsPoint3(*lightPos), lightRadius, ambientAlpha, spotAlpha, (SkShadowMaskFilter::ShadowFlags)flags).release());
}
