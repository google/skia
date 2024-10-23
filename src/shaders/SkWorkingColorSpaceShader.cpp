/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/shaders/SkWorkingColorSpaceShader.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkShaderBase.h"

#include <utility>

bool SkWorkingColorSpaceShader::appendStages(const SkStageRec& rec,
                                             const SkShaders::MatrixRec& mRec) const {
    sk_sp<SkColorSpace> dstCS = sk_ref_sp(rec.fDstCS);
    if (!dstCS) {
        dstCS = SkColorSpace::MakeSRGB();
    }

    SkColorInfo dst     = {rec.fDstColorType, kPremul_SkAlphaType, dstCS},
                working = {rec.fDstColorType, kPremul_SkAlphaType, fWorkingSpace};

    const auto* dstToWorking = rec.fAlloc->make<SkColorSpaceXformSteps>(dst, working);
    const auto* workingToDst = rec.fAlloc->make<SkColorSpaceXformSteps>(working, dst);

    // Alpha-only image shaders reference the paint color, which is already in the destination
    // color space. We need to transform it to the working space for consistency.
    SkColor4f paintColorInWorkingSpace = rec.fPaintColor.makeOpaque();
    dstToWorking->apply(paintColorInWorkingSpace.vec());

    SkStageRec workingRec = {rec.fPipeline,
                             rec.fAlloc,
                             rec.fDstColorType,
                             fWorkingSpace.get(),
                             paintColorInWorkingSpace,
                             rec.fSurfaceProps};

    if (!as_SB(fShader)->appendStages(workingRec, mRec)) {
        return false;
    }

    workingToDst->apply(rec.fPipeline);
    return true;
}

void SkWorkingColorSpaceShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fShader.get());
    buffer.writeDataAsByteArray(fWorkingSpace->serialize().get());
}

sk_sp<SkFlattenable> SkWorkingColorSpaceShader::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkShader> shader(buffer.readShader());
    auto data = buffer.readByteArrayAsData();
    if (!buffer.validate(data != nullptr)) {
        return nullptr;
    }
    sk_sp<SkColorSpace> workingSpace = SkColorSpace::Deserialize(data->data(), data->size());
    if (!buffer.validate(workingSpace != nullptr)) {
        return nullptr;
    }
    return Make(std::move(shader), std::move(workingSpace));
}

void SkRegisterWorkingColorSpaceShaderFlattenable() {
    SK_REGISTER_FLATTENABLE(SkWorkingColorSpaceShader);
}
