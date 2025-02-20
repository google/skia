/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_PrecompileColorFiltersPriv_DEFINED
#define skgpu_graphite_precompile_PrecompileColorFiltersPriv_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"

namespace skgpu::graphite {

class PrecompileColorFilter;

namespace PrecompileColorFiltersPriv {
    // These three factories match those in src/core/SkColorFilterPriv.h
    sk_sp<PrecompileColorFilter> Gaussian();

    sk_sp<PrecompileColorFilter> ColorSpaceXform(SkSpan<const sk_sp<SkColorSpace>> src,
                                                 SkSpan<const sk_sp<SkColorSpace>> dst);

    sk_sp<PrecompileColorFilter> WithWorkingFormat(
            SkSpan<const sk_sp<PrecompileColorFilter>> childOptions,
            const skcms_TransferFunction* tf,
            const skcms_Matrix3x3* gamut,
            const SkAlphaType* at);

} // namespace PrecompileColorFiltersPriv

} // namespace skgpu::graphite

#endif // skgpu_graphite_precompile_PrecompileColorFiltersPriv_DEFINED
