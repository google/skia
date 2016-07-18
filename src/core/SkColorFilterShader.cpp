/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilterShader.h"
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

uint32_t SkColorFilterShader::FilterShaderContext::getFlags() const {
    const SkColorFilterShader& filterShader = static_cast<const SkColorFilterShader&>(fShader);

    uint32_t shaderF = fShaderContext->getFlags();
    uint32_t filterF = filterShader.fFilter->getFlags();

    // If the filter does not support a given feature, but sure to clear the corresponding flag
    // in the shader flags.
    //
    if (!(filterF & SkColorFilter::kAlphaUnchanged_Flag)) {
        shaderF &= ~SkShader::kOpaqueAlpha_Flag;
    }
    return shaderF;
}

SkShader::Context* SkColorFilterShader::onCreateContext(const ContextRec& rec,
                                                        void* storage) const {
    char* shaderContextStorage = (char*)storage + sizeof(FilterShaderContext);
    SkShader::Context* shaderContext = fShader->createContext(rec, shaderContextStorage);
    if (nullptr == shaderContext) {
        return nullptr;
    }
    return new (storage) FilterShaderContext(*this, shaderContext, rec);
}

size_t SkColorFilterShader::onContextSize(const ContextRec& rec) const {
    return sizeof(FilterShaderContext) + fShader->contextSize(rec);
}

SkColorFilterShader::FilterShaderContext::FilterShaderContext(
                                                         const SkColorFilterShader& filterShader,
                                                         SkShader::Context* shaderContext,
                                                         const ContextRec& rec)
    : INHERITED(filterShader, rec)
    , fShaderContext(shaderContext)
{}

SkColorFilterShader::FilterShaderContext::~FilterShaderContext() {
    fShaderContext->~Context();
}

void SkColorFilterShader::FilterShaderContext::shadeSpan(int x, int y, SkPMColor result[],
                                                         int count) {
    const SkColorFilterShader& filterShader = static_cast<const SkColorFilterShader&>(fShader);

    fShaderContext->shadeSpan(x, y, result, count);
    filterShader.fFilter->filterSpan(result, count, result);
}

void SkColorFilterShader::FilterShaderContext::shadeSpan4f(int x, int y, SkPM4f result[],
                                                          int count) {
    const SkColorFilterShader& filterShader = static_cast<const SkColorFilterShader&>(fShader);

    fShaderContext->shadeSpan4f(x, y, result, count);
    filterShader.fFilter->filterSpan4f(result, count, result);
}

#if SK_SUPPORT_GPU
/////////////////////////////////////////////////////////////////////

sk_sp<GrFragmentProcessor> SkColorFilterShader::asFragmentProcessor(
                                                     GrContext* context,
                                                     const SkMatrix& viewM,
                                                     const SkMatrix* localMatrix,
                                                     SkFilterQuality fq,
                                                     SkSourceGammaTreatment gammaTreatment) const {

    sk_sp<GrFragmentProcessor> fp1(fShader->asFragmentProcessor(context, viewM, localMatrix, fq,
                                                                gammaTreatment));
    if (!fp1) {
        return nullptr;
    }

    sk_sp<GrFragmentProcessor> fp2(fFilter->asFragmentProcessor(context));
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
    fShader->toString(str);
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
