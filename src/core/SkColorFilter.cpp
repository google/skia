/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkColorFilter.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkTPin.h"
#include "modules/skcms/skcms.h"
#include "src/core/SkColorFilterPriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"

#include <cstddef>

enum class SkBlendMode;
struct SkDeserialProcs;

bool SkColorFilter::asAColorMode(SkColor* color, SkBlendMode* mode) const {
    return as_CFB(this)->onAsAColorMode(color, mode);
}

bool SkColorFilter::asAColorMatrix(float matrix[20]) const {
    return as_CFB(this)->onAsAColorMatrix(matrix);
}

bool SkColorFilter::isAlphaUnchanged() const {
    return as_CFB(this)->onIsAlphaUnchanged();
}

sk_sp<SkColorFilter> SkColorFilter::Deserialize(const void* data, size_t size,
                                                const SkDeserialProcs* procs) {
    return sk_sp<SkColorFilter>(static_cast<SkColorFilter*>(
                                SkFlattenable::Deserialize(
                                kSkColorFilter_Type, data, size, procs).release()));
}

SkColor SkColorFilter::filterColor(SkColor c) const {
    // This is mostly meaningless. We should phase-out this call entirely.
    SkColorSpace* cs = nullptr;
    return this->filterColor4f(SkColor4f::FromColor(c), cs, cs).toSkColor();
}

SkColor4f SkColorFilter::filterColor4f(const SkColor4f& origSrcColor, SkColorSpace* srcCS,
                                       SkColorSpace* dstCS) const {
    SkPMColor4f color = { origSrcColor.fR, origSrcColor.fG, origSrcColor.fB, origSrcColor.fA };
    SkColorSpaceXformSteps(srcCS, kUnpremul_SkAlphaType,
                           dstCS, kPremul_SkAlphaType).apply(color.vec());

    SkPMColor4f filteredColor = as_CFB(this)->onFilterColor4f(color, dstCS);
    // SkColor4f will assert if we allow alpha outside [0,1]. (SkSL color filters might do this).
    filteredColor.fA = SkTPin(filteredColor.fA, 0.0f, 1.0f);
    return filteredColor.unpremul();
}

sk_sp<SkColorFilter> SkColorFilter::makeWithWorkingColorSpace(
        sk_sp<SkColorSpace> workingSpace) const {
    SkColorFilter* base = const_cast<SkColorFilter*>(this);
    if (!workingSpace) {
        return sk_ref_sp(base);
    }

    skcms_TransferFunction tf;
    skcms_Matrix3x3 toXYZ;
    workingSpace->transferFn(&tf);
    workingSpace->toXYZD50(&toXYZ);
    const SkAlphaType* kOriginalAlphaType = nullptr;
    return SkColorFilterPriv::WithWorkingFormat(sk_ref_sp(base), &tf, &toXYZ, kOriginalAlphaType);
}
