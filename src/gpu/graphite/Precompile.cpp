/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#ifdef SK_ENABLE_PRECOMPILE

#include "src/gpu/graphite/Precompile.h"
#include "src/gpu/graphite/PrecompileBasePriv.h"

namespace skgpu::graphite {

int PaintOptions::numCombinations() const {
    int numShaderCombos = 0;
    if (fShaders.empty()) {
        numShaderCombos = 1; // just going to use the SkPaint's color here
    } else {
        for (auto s : fShaders) {
            numShaderCombos += s->priv().numCombinations();
        }
    }

    int numMaskFilterCombos = 0;
    if (fMaskFilters.empty()) {
        numMaskFilterCombos = 1;
    } else {
        for (auto mf : fMaskFilters) {
            numMaskFilterCombos += mf->priv().numCombinations();
        }
    }

    int numColorFilterCombos = 0;
    if (fColorFilters.empty()) {
        numColorFilterCombos = 1;
    } else {
        for (auto cf : fColorFilters) {
            numColorFilterCombos += cf->priv().numCombinations();
        }
    }

    // TODO: handle ImageFilters separately

    int numBlenderCombos = CountBlenderCombos(fBlenders);

    return numShaderCombos * numMaskFilterCombos * numColorFilterCombos * numBlenderCombos;
}

int CountBlenderCombos(const std::vector<sk_sp<PrecompileBlender>>& blenders) {
    bool bmBased = false;
    int numBlendCombos = 0;
    for (auto b: blenders) {
        if (b->asBlendMode().has_value()) {
            bmBased = true;
        } else {
            numBlendCombos += b->priv().numChildCombinations();
        }
    }

    if (bmBased || !numBlendCombos) {
        // If numBlendCombos is zero we will fallback kSrcOver blending
        ++numBlendCombos;
    }

    return numBlendCombos;
}

} // namespace skgpu::graphite

#endif // SK_ENABLE_PRECOMPILE
