/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/private/SkArenaAlloc.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkColorFilterShader.h"

#if SK_SUPPORT_GPU
#include "src/gpu/GrFragmentProcessor.h"
#endif

SkColorFilterShader::SkColorFilterShader(sk_sp<SkShader> shader, sk_sp<SkColorFilter> filter)
    : fShader(std::move(shader))
    , fFilter(std::move(filter))
{
    SkASSERT(fShader);
    SkASSERT(fFilter);
}

sk_sp<SkFlattenable> SkColorFilterShader::CreateProc(SkReadBuffer& buffer) {
    auto shader = buffer.readShader();
    auto filter = buffer.readColorFilter();
    if (!shader || !filter) {
        return nullptr;
    }
    return sk_make_sp<SkColorFilterShader>(shader, filter);
}

void SkColorFilterShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fShader.get());
    buffer.writeFlattenable(fFilter.get());
}

bool SkColorFilterShader::onAppendStages(const SkStageRec& rec) const {
    if (!as_SB(fShader)->appendStages(rec)) {
        return false;
    }
    fFilter->appendStages(rec, fShader->isOpaque());
    return true;
}

#if SK_SUPPORT_GPU
/////////////////////////////////////////////////////////////////////

#include "include/gpu/GrContext.h"

std::unique_ptr<GrFragmentProcessor> SkColorFilterShader::asFragmentProcessor(
        const GrFPArgs& args) const {
    auto fp1 = as_SB(fShader)->asFragmentProcessor(args);
    if (!fp1) {
        return nullptr;
    }

    auto fp2 = fFilter->asFragmentProcessor(args.fContext, *args.fDstColorSpaceInfo);
    if (!fp2) {
        return fp1;
    }

    std::unique_ptr<GrFragmentProcessor> fpSeries[] = { std::move(fp1), std::move(fp2) };
    return GrFragmentProcessor::RunInSeries(fpSeries, 2);
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkShader> SkShader::makeWithColorFilter(sk_sp<SkColorFilter> filter) const {
    SkShader* base = const_cast<SkShader*>(this);
    if (!filter) {
        return sk_ref_sp(base);
    }
    return sk_make_sp<SkColorFilterShader>(sk_ref_sp(base), filter);
}
