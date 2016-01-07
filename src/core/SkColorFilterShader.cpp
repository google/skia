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

SkColorFilterShader::SkColorFilterShader(SkShader* shader, SkColorFilter* filter)
    : fShader(SkRef(shader))
    , fFilter(SkRef(filter))
{
    SkASSERT(shader);
    SkASSERT(filter);
}

SkFlattenable* SkColorFilterShader::CreateProc(SkReadBuffer& buffer) {
    SkAutoTUnref<SkShader> shader(buffer.readShader());
    SkAutoTUnref<SkColorFilter> filter(buffer.readColorFilter());
    if (!shader.get() || !filter.get()) {
        return nullptr;
    }
    return new SkColorFilterShader(shader, filter);
}

void SkColorFilterShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fShader);
    buffer.writeFlattenable(fFilter);
}

uint32_t SkColorFilterShader::FilterShaderContext::getFlags() const {
    const SkColorFilterShader& filterShader = static_cast<const SkColorFilterShader&>(fShader);

    uint32_t shaderF = fShaderContext->getFlags();
    uint32_t filterF = filterShader.fFilter->getFlags();

    // if the filter might change alpha, clear the opaque flag in the shader
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

size_t SkColorFilterShader::contextSize() const {
    return sizeof(FilterShaderContext) + fShader->contextSize();
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

#if SK_SUPPORT_GPU
/////////////////////////////////////////////////////////////////////

const GrFragmentProcessor* SkColorFilterShader::asFragmentProcessor(
                                                               GrContext* context,
                                                               const SkMatrix& viewM,
                                                               const SkMatrix* localMatrix,
                                                               SkFilterQuality fq) const {

    SkAutoTUnref<const GrFragmentProcessor> fp1(fShader->asFragmentProcessor(context, viewM,
                                                                             localMatrix, fq));
    if (!fp1.get()) {
        return nullptr;
    }

    SkAutoTUnref<const GrFragmentProcessor> fp2(fFilter->asFragmentProcessor(context));
    if (!fp2.get()) {
        return fp1.release();
    }

    const GrFragmentProcessor* fpSeries[] = { fp1.get(), fp2.get() };

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

SkShader* SkShader::newWithColorFilter(SkColorFilter* filter) const {
    SkShader* base = const_cast<SkShader*>(this);
    if (!filter) {
        return SkRef(base);
    }
    return new SkColorFilterShader(base, filter);
}
