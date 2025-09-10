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

sk_sp<SkShader> SkWorkingColorSpaceShader::Make(sk_sp<SkShader> shader,
                                                sk_sp<SkColorSpace> inputCS,
                                                sk_sp<SkColorSpace> outputCS,
                                                bool workInUnpremul) {
    if (!shader) {
        return nullptr;
    }

    if (!inputCS && !outputCS && !workInUnpremul) {
        // A null input is the final dst CS, and a null output is the input CS, so if both are null
        // then there's no additional conversion for children and no additional conversion
        // applied to the shader's output.
        return shader;
    } else {
        // Otherwise there's some conversion that has to happen on the input side or the output side
        // that makes this not a no-op.
        return sk_sp(new SkWorkingColorSpaceShader(std::move(shader),
                                                   std::move(inputCS),
                                                   std::move(outputCS),
                                                   workInUnpremul));
    }
}

bool SkWorkingColorSpaceShader::appendStages(const SkStageRec& rec,
                                             const SkShaders::MatrixRec& mRec) const {
    sk_sp<SkColorSpace> dstCS = sk_ref_sp(rec.fDstCS);
    if (!dstCS) {
        dstCS = SkColorSpace::MakeSRGB();
    }

    // TODO(b/431253455): Should get the dstAT from `rec`
    const SkAlphaType dstAT = kPremul_SkAlphaType;
    auto [inputCS, outputCS, workingAT]  = this->workingSpace(dstCS, dstAT);

    SkColorInfo dst    = {rec.fDstColorType, dstAT,     dstCS},
                input  = {rec.fDstColorType, workingAT, inputCS},
                output = {rec.fDstColorType, workingAT, outputCS};

    const auto* dstToInput  = rec.fAlloc->make<SkColorSpaceXformSteps>(dst, input);
    const auto* outputToDst = rec.fAlloc->make<SkColorSpaceXformSteps>(output, dst);
    // NOTE: There is no inputToOutput steps to apply because it is assumed that the child shader
    // is responsible for such conversion (or input == output and it's a no-op).

    // Alpha-only image shaders reference the paint color, which is already in the destination
    // color space. We need to transform it to the working space for consistency.
    SkColor4f paintColorInWorkingSpace = rec.fPaintColor.makeOpaque();
    dstToInput->apply(paintColorInWorkingSpace.vec());

    // TODO(b/431253455): The working rec should have its alpha type set to `workingAT`
    SkStageRec workingRec = {rec.fPipeline,
                             rec.fAlloc,
                             rec.fDstColorType,
                             fInputSpace.get(),
                             paintColorInWorkingSpace,
                             rec.fSurfaceProps,
                             rec.fDstBounds};

    if (!as_SB(fShader)->appendStages(workingRec, mRec)) {
        return false;
    }

    outputToDst->apply(rec.fPipeline);
    return true;
}

void SkWorkingColorSpaceShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fShader.get());
    buffer.writeBool(fWorkInUnpremul);

    buffer.writeBool(SkToBool(fInputSpace));
    if (fInputSpace) {
        buffer.writeDataAsByteArray(fInputSpace->serialize().get());
    }

    buffer.writeBool(SkToBool(fOutputSpace));
    if (fOutputSpace) {
        buffer.writeDataAsByteArray(fOutputSpace->serialize().get());
    }
}

sk_sp<SkFlattenable> SkWorkingColorSpaceShader::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkShader> shader(buffer.readShader());

    // If true, will not work in unpremul and assume inputSpace will be non-null and the outputSpace
    // will be null.
    const bool legacyWorkingCS = buffer.isVersionLT(SkPicturePriv::kWorkingColorSpaceOutput);

    bool workInUnpremul = !legacyWorkingCS && buffer.readBool();

    // The input/output spaces are allowed to be null, but if we think we have a non-null CS, then
    // it better be deserializable.
    sk_sp<SkColorSpace> inputSpace;
    if (legacyWorkingCS || buffer.readBool()) {
        auto data = buffer.readByteArrayAsData();
        if (!buffer.validate(data != nullptr)) {
            return nullptr;
        }
        inputSpace = SkColorSpace::Deserialize(data->data(), data->size());
        if (!buffer.validate(inputSpace != nullptr)) {
            return nullptr;
        }
    }

    sk_sp<SkColorSpace> outputSpace;
    if (!legacyWorkingCS && buffer.readBool()) {
        auto data = buffer.readByteArrayAsData();
        if (!buffer.validate(data != nullptr)) {
            return nullptr;
        }
        outputSpace = SkColorSpace::Deserialize(data->data(), data->size());
        if (!buffer.validate(outputSpace != nullptr)) {
            return nullptr;
        }
    }

    return Make(std::move(shader), std::move(inputSpace), std::move(outputSpace), workInUnpremul);
}

void SkRegisterWorkingColorSpaceShaderFlattenable() {
    SK_REGISTER_FLATTENABLE(SkWorkingColorSpaceShader);
}
