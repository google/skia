/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/shaders/SkColorFilterShader.h"

#include "include/core/SkColorFilter.h"
#include "include/core/SkShader.h"
#include "include/private/base/SkAssert.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"

#include <utility>

SkColorFilterShader::SkColorFilterShader(sk_sp<SkShader> shader,
                                         float alpha,
                                         sk_sp<SkColorFilter> filter)
    : fShader(std::move(shader))
    , fFilter(as_CFB_sp(std::move(filter)))
    , fAlpha (alpha)
{
    SkASSERT(fShader);
    SkASSERT(fFilter);
}

sk_sp<SkShader> SkColorFilterShader::Make(sk_sp<SkShader> shader,
                                          float alpha,
                                          sk_sp<SkColorFilter> filter) {
    if (!shader) {
        return nullptr;
    } else if (!filter) {
        return shader;
    } else {
        return sk_sp(new SkColorFilterShader(std::move(shader), alpha, std::move(filter)));
    }
}

sk_sp<SkFlattenable> SkColorFilterShader::CreateProc(SkReadBuffer& buffer) {
    auto shader = buffer.readShader();
    auto filter = buffer.readColorFilter();
    return Make(std::move(shader), 1.0f, std::move(filter));
}

bool SkColorFilterShader::isOpaque() const {
    return fShader->isOpaque() && fAlpha == 1.0f && as_CFB(fFilter)->isAlphaUnchanged();
}

void SkColorFilterShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fShader.get());
    SkASSERT(fAlpha == 1.0f);  // Not exposed in public API SkShader::makeWithColorFilter().
    buffer.writeFlattenable(fFilter.get());
}

bool SkColorFilterShader::appendStages(const SkStageRec& rec,
                                       const SkShaders::MatrixRec& mRec) const {
    if (!as_SB(fShader)->appendStages(rec, mRec)) {
        return false;
    }
    if (fAlpha != 1.0f) {
        rec.fPipeline->append(SkRasterPipelineOp::scale_1_float, rec.fAlloc->make<float>(fAlpha));
    }
    if (!fFilter->appendStages(rec, fAlpha == 1.0f && fShader->isOpaque())) {
        return false;
    }
    return true;
}
