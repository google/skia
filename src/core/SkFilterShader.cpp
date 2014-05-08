/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFilterShader.h"

#include "SkColorFilter.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkShader.h"
#include "SkString.h"

SkFilterShader::SkFilterShader(SkShader* shader, SkColorFilter* filter) {
    fShader = shader;
    shader->ref();

    fFilter = filter;
    filter->ref();
}

SkFilterShader::SkFilterShader(SkReadBuffer& buffer)
    : INHERITED(buffer) {
    fShader = buffer.readShader();
    fFilter = buffer.readColorFilter();
}

SkFilterShader::~SkFilterShader() {
    fFilter->unref();
    fShader->unref();
}

void SkFilterShader::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeFlattenable(fShader);
    buffer.writeFlattenable(fFilter);
}

uint32_t SkFilterShader::FilterShaderContext::getFlags() const {
    const SkFilterShader& filterShader = static_cast<const SkFilterShader&>(fShader);

    uint32_t shaderF = fShaderContext->getFlags();
    uint32_t filterF = filterShader.fFilter->getFlags();

    // if the filter doesn't support 16bit, clear the matching bit in the shader
    if (!(filterF & SkColorFilter::kHasFilter16_Flag)) {
        shaderF &= ~SkShader::kHasSpan16_Flag;
    }
    // if the filter might change alpha, clear the opaque flag in the shader
    if (!(filterF & SkColorFilter::kAlphaUnchanged_Flag)) {
        shaderF &= ~(SkShader::kOpaqueAlpha_Flag | SkShader::kHasSpan16_Flag);
    }
    return shaderF;
}

SkShader::Context* SkFilterShader::onCreateContext(const ContextRec& rec, void* storage) const {
    char* shaderContextStorage = (char*)storage + sizeof(FilterShaderContext);
    SkShader::Context* shaderContext = fShader->createContext(rec, shaderContextStorage);
    if (NULL == shaderContext) {
        return NULL;
    }
    return SkNEW_PLACEMENT_ARGS(storage, FilterShaderContext, (*this, shaderContext, rec));
}

size_t SkFilterShader::contextSize() const {
    return sizeof(FilterShaderContext) + fShader->contextSize();
}

SkFilterShader::FilterShaderContext::FilterShaderContext(const SkFilterShader& filterShader,
                                                         SkShader::Context* shaderContext,
                                                         const ContextRec& rec)
    : INHERITED(filterShader, rec)
    , fShaderContext(shaderContext) {}

SkFilterShader::FilterShaderContext::~FilterShaderContext() {
    fShaderContext->~Context();
}

void SkFilterShader::FilterShaderContext::shadeSpan(int x, int y, SkPMColor result[], int count) {
    const SkFilterShader& filterShader = static_cast<const SkFilterShader&>(fShader);

    fShaderContext->shadeSpan(x, y, result, count);
    filterShader.fFilter->filterSpan(result, count, result);
}

void SkFilterShader::FilterShaderContext::shadeSpan16(int x, int y, uint16_t result[], int count) {
    const SkFilterShader& filterShader = static_cast<const SkFilterShader&>(fShader);

    SkASSERT(fShaderContext->getFlags() & SkShader::kHasSpan16_Flag);
    SkASSERT(filterShader.fFilter->getFlags() & SkColorFilter::kHasFilter16_Flag);

    fShaderContext->shadeSpan16(x, y, result, count);
    filterShader.fFilter->filterSpan16(result, count, result);
}

#ifndef SK_IGNORE_TO_STRING
void SkFilterShader::toString(SkString* str) const {
    str->append("SkFilterShader: (");

    str->append("Shader: ");
    fShader->toString(str);
    str->append(" Filter: ");
    // TODO: add "fFilter->toString(str);" once SkColorFilter::toString is added

    this->INHERITED::toString(str);

    str->append(")");
}
#endif
