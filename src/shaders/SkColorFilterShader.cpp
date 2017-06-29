/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkColorFilterShader.h"
#include "SkColorSpaceXformer.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkShader.h"
#include "SkString.h"

#if SK_SUPPORT_GPU
#include "GrFragmentProcessor.h"
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

bool SkColorFilterShader::onAppendStages(SkRasterPipeline* pipeline, SkColorSpace* dstCS,
                                         SkArenaAlloc* alloc, const SkMatrix& ctm,
                                         const SkPaint& paint, const SkMatrix* localM) const {
    if (!as_SB(fShader)->appendStages(pipeline, dstCS, alloc, ctm, paint, localM)) {
        return false;
    }
    fFilter->appendStages(pipeline, dstCS, alloc, fShader->isOpaque());
    return true;
}

sk_sp<SkShader> SkColorFilterShader::onMakeColorSpace(SkColorSpaceXformer* xformer) const {
    return xformer->apply(fShader.get())->makeWithColorFilter(xformer->apply(fFilter.get()));
}

#if SK_SUPPORT_GPU
/////////////////////////////////////////////////////////////////////

sk_sp<GrFragmentProcessor> SkColorFilterShader::asFragmentProcessor(const AsFPArgs& args) const {

    sk_sp<GrFragmentProcessor> fp1(as_SB(fShader)->asFragmentProcessor(args));
    if (!fp1) {
        return nullptr;
    }

    sk_sp<GrFragmentProcessor> fp2(fFilter->asFragmentProcessor(args.fContext,
                                                                args.fDstColorSpace));
    if (!fp2) {
        return fp1;
    }

    sk_sp<GrFragmentProcessor> fpSeries[] = { std::move(fp1), std::move(fp2) };
    return GrFragmentProcessor::RunInSeries(fpSeries, 2);
}
#endif

#ifndef SK_IGNORE_TO_STRING
void SkColorFilterShader::toString(SkString* str) const {
    str->append("SkColorFilterShader: (");

    str->append("Shader: ");
    as_SB(fShader)->toString(str);
    str->append(" Filter: ");
    // TODO: add "fFilter->toString(str);" once SkColorFilter::toString is added

    this->INHERITED::toString(str);

    str->append(")");
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
