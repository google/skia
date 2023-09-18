/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/effects/colorfilters/SkWorkingFormatColorFilter.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAssert.h"
#include "modules/skcms/skcms.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkColorFilterPriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"

#include <utility>

SkWorkingFormatColorFilter::SkWorkingFormatColorFilter(sk_sp<SkColorFilter> child,
                                                       const skcms_TransferFunction* tf,
                                                       const skcms_Matrix3x3* gamut,
                                                       const SkAlphaType* at) {
    fChild = std::move(child);
    if (tf) {
        fTF = *tf;
        fUseDstTF = false;
    }
    if (gamut) {
        fGamut = *gamut;
        fUseDstGamut = false;
    }
    if (at) {
        fAT = *at;
        fUseDstAT = false;
    }
}

sk_sp<SkColorSpace> SkWorkingFormatColorFilter::workingFormat(const sk_sp<SkColorSpace>& dstCS,
                                                              SkAlphaType* at) const {
    skcms_TransferFunction tf = fTF;
    skcms_Matrix3x3 gamut = fGamut;

    if (fUseDstTF) {
        SkAssertResult(dstCS->isNumericalTransferFn(&tf));
    }
    if (fUseDstGamut) {
        SkAssertResult(dstCS->toXYZD50(&gamut));
    }

    *at = fUseDstAT ? kPremul_SkAlphaType : fAT;
    return SkColorSpace::MakeRGB(tf, gamut);
}

bool SkWorkingFormatColorFilter::appendStages(const SkStageRec& rec, bool shaderIsOpaque) const {
    sk_sp<SkColorSpace> dstCS = sk_ref_sp(rec.fDstCS);

    if (!dstCS) {
        dstCS = SkColorSpace::MakeSRGB();
    }

    SkAlphaType workingAT;
    sk_sp<SkColorSpace> workingCS = this->workingFormat(dstCS, &workingAT);

    SkColorInfo dst = {rec.fDstColorType, kPremul_SkAlphaType, dstCS},
                working = {rec.fDstColorType, workingAT, workingCS};

    const auto* dstToWorking = rec.fAlloc->make<SkColorSpaceXformSteps>(dst, working);
    const auto* workingToDst = rec.fAlloc->make<SkColorSpaceXformSteps>(working, dst);

    // The paint color is in the destination color space, so *should* be coverted to working space.
    // That's not necessary, though:
    //   - Tinting alpha-only image shaders is the only effect that uses paint-color
    //   - Alpha-only image shaders can't be reached from color-filters without SkSL
    //   - SkSL disables paint-color tinting of alpha-only image shaders

    SkStageRec workingRec = {rec.fPipeline,
                             rec.fAlloc,
                             rec.fDstColorType,
                             workingCS.get(),
                             rec.fPaintColor,
                             rec.fSurfaceProps};

    dstToWorking->apply(rec.fPipeline);
    if (!as_CFB(fChild)->appendStages(workingRec, shaderIsOpaque)) {
        return false;
    }
    workingToDst->apply(rec.fPipeline);
    return true;
}

SkPMColor4f SkWorkingFormatColorFilter::onFilterColor4f(const SkPMColor4f& origColor,
                                                        SkColorSpace* rawDstCS) const {
    sk_sp<SkColorSpace> dstCS = sk_ref_sp(rawDstCS);
    if (!dstCS) {
        dstCS = SkColorSpace::MakeSRGB();
    }

    SkAlphaType workingAT;
    sk_sp<SkColorSpace> workingCS = this->workingFormat(dstCS, &workingAT);

    SkColorInfo dst = {kUnknown_SkColorType, kPremul_SkAlphaType, dstCS},
                working = {kUnknown_SkColorType, workingAT, workingCS};

    SkPMColor4f color = origColor;
    SkColorSpaceXformSteps{dst, working}.apply(color.vec());
    color = as_CFB(fChild)->onFilterColor4f(color, working.colorSpace());
    SkColorSpaceXformSteps{working, dst}.apply(color.vec());
    return color;
}

bool SkWorkingFormatColorFilter::onIsAlphaUnchanged() const { return fChild->isAlphaUnchanged(); }

void SkWorkingFormatColorFilter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fChild.get());
    buffer.writeBool(fUseDstTF);
    buffer.writeBool(fUseDstGamut);
    buffer.writeBool(fUseDstAT);
    if (!fUseDstTF) {
        buffer.writeScalarArray(&fTF.g, 7);
    }
    if (!fUseDstGamut) {
        buffer.writeScalarArray(&fGamut.vals[0][0], 9);
    }
    if (!fUseDstAT) {
        buffer.writeInt(fAT);
    }
}

sk_sp<SkFlattenable> SkWorkingFormatColorFilter::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkColorFilter> child = buffer.readColorFilter();
    bool useDstTF = buffer.readBool(), useDstGamut = buffer.readBool(),
         useDstAT = buffer.readBool();

    skcms_TransferFunction tf;
    skcms_Matrix3x3 gamut;
    SkAlphaType at;

    if (!useDstTF) {
        buffer.readScalarArray(&tf.g, 7);
    }
    if (!useDstGamut) {
        buffer.readScalarArray(&gamut.vals[0][0], 9);
    }
    if (!useDstAT) {
        at = buffer.read32LE(kLastEnum_SkAlphaType);
    }

    return SkColorFilterPriv::WithWorkingFormat(std::move(child),
                                                useDstTF ? nullptr : &tf,
                                                useDstGamut ? nullptr : &gamut,
                                                useDstAT ? nullptr : &at);
}

sk_sp<SkColorFilter> SkColorFilterPriv::WithWorkingFormat(sk_sp<SkColorFilter> child,
                                                          const skcms_TransferFunction* tf,
                                                          const skcms_Matrix3x3* gamut,
                                                          const SkAlphaType* at) {
    return sk_make_sp<SkWorkingFormatColorFilter>(std::move(child), tf, gamut, at);
}

void SkRegisterWorkingFormatColorFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkWorkingFormatColorFilter);
}
