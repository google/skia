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

SkFilterShader::~SkFilterShader() {
    fFilter->unref();
    fShader->unref();
}

SkFlattenable* SkFilterShader::CreateProc(SkReadBuffer& buffer) {
    SkAutoTUnref<SkShader> shader(buffer.readShader());
    SkAutoTUnref<SkColorFilter> filter(buffer.readColorFilter());
    if (!shader.get() || !filter.get()) {
        return NULL;
    }
    return SkNEW_ARGS(SkFilterShader, (shader, filter));
}

void SkFilterShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fShader);
    buffer.writeFlattenable(fFilter);
}

uint32_t SkFilterShader::FilterShaderContext::getFlags() const {
    const SkFilterShader& filterShader = static_cast<const SkFilterShader&>(fShader);

    uint32_t shaderF = fShaderContext->getFlags();
    uint32_t filterF = filterShader.fFilter->getFlags();

    // filters don't support 16bit, so clear the matching bit in the shader
    shaderF &= ~SkShader::kHasSpan16_Flag;

    // if the filter might change alpha, clear the opaque flag in the shader
    if (!(filterF & SkColorFilter::kAlphaUnchanged_Flag)) {
        shaderF &= ~SkShader::kOpaqueAlpha_Flag;
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
