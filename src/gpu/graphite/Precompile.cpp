/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/FactoryFunctions.h"
#include "src/gpu/graphite/PrecompileInternal.h"

namespace skgpu::graphite {

//--------------------------------------------------------------------------------------------------
sk_sp<PrecompileColorFilter> PrecompileColorFilter::makeComposed(
        sk_sp<PrecompileColorFilter> inner) const {
    if (!inner) {
        return sk_ref_sp(this);
    }

    return PrecompileColorFilters::Compose({ sk_ref_sp(this) }, { std::move(inner) });
}

} // namespace skgpu::graphite
