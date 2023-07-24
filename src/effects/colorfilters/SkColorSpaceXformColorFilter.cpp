/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/effects/colorfilters/SkColorSpaceXformColorFilter.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "src/base/SkNoDestructor.h"
#include "src/core/SkColorFilterPriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"

#include <cstdint>
#include <utility>

SkColorSpaceXformColorFilter::SkColorSpaceXformColorFilter(sk_sp<SkColorSpace> src,
                                                           sk_sp<SkColorSpace> dst)
        : fSrc(std::move(src))
        , fDst(std::move(dst))
        , fSteps(  // We handle premul/unpremul separately, so here just always upm->upm.
                  fSrc.get(),
                  kUnpremul_SkAlphaType,
                  fDst.get(),
                  kUnpremul_SkAlphaType) {}

bool SkColorSpaceXformColorFilter::appendStages(const SkStageRec& rec, bool shaderIsOpaque) const {
    if (!shaderIsOpaque) {
        rec.fPipeline->append(SkRasterPipelineOp::unpremul);
    }

    fSteps.apply(rec.fPipeline);

    if (!shaderIsOpaque) {
        rec.fPipeline->append(SkRasterPipelineOp::premul);
    }
    return true;
}

void SkColorSpaceXformColorFilter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeDataAsByteArray(fSrc->serialize().get());
    buffer.writeDataAsByteArray(fDst->serialize().get());
}

sk_sp<SkFlattenable> SkColorSpaceXformColorFilter::LegacyGammaOnlyCreateProc(SkReadBuffer& buffer) {
    uint32_t dir = buffer.read32();
    if (!buffer.validate(dir <= 1)) {
        return nullptr;
    }
    if (dir == 0) {
        return SkColorFilters::LinearToSRGBGamma();
    }
    return SkColorFilters::SRGBToLinearGamma();
}

sk_sp<SkFlattenable> SkColorSpaceXformColorFilter::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkColorSpace> colorSpaces[2];
    for (int i = 0; i < 2; ++i) {
        auto data = buffer.readByteArrayAsData();
        if (!buffer.validate(data != nullptr)) {
            return nullptr;
        }
        colorSpaces[i] = SkColorSpace::Deserialize(data->data(), data->size());
        if (!buffer.validate(colorSpaces[i] != nullptr)) {
            return nullptr;
        }
    }
    return sk_sp<SkFlattenable>(
            new SkColorSpaceXformColorFilter(std::move(colorSpaces[0]), std::move(colorSpaces[1])));
}

sk_sp<SkColorFilter> SkColorFilters::LinearToSRGBGamma() {
    static SkNoDestructor<SkColorSpaceXformColorFilter> gSingleton(SkColorSpace::MakeSRGBLinear(),
                                                                   SkColorSpace::MakeSRGB());
    return sk_ref_sp(gSingleton.get());
}

sk_sp<SkColorFilter> SkColorFilters::SRGBToLinearGamma() {
    static SkNoDestructor<SkColorSpaceXformColorFilter> gSingleton(SkColorSpace::MakeSRGB(),
                                                                   SkColorSpace::MakeSRGBLinear());
    return sk_ref_sp(gSingleton.get());
}

sk_sp<SkColorFilter> SkColorFilterPriv::MakeColorSpaceXform(sk_sp<SkColorSpace> src,
                                                            sk_sp<SkColorSpace> dst) {
    return sk_make_sp<SkColorSpaceXformColorFilter>(std::move(src), std::move(dst));
}

void SkRegisterSkColorSpaceXformColorFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkColorSpaceXformColorFilter);
    // Previous name
    SkFlattenable::Register("ColorSpaceXformColorFilter", SkColorSpaceXformColorFilter::CreateProc);
    // TODO(ccameron): Remove after grace period for SKPs to stop using old serialization.
    SkFlattenable::Register("SkSRGBGammaColorFilter",
                            SkColorSpaceXformColorFilter::LegacyGammaOnlyCreateProc);
}
