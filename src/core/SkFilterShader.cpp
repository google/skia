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

SkShader::Context* SkFilterShader::createContext(const SkBitmap& device,
                                                 const SkPaint& paint,
                                                 const SkMatrix& matrix,
                                                 void* storage) const {
    if (!this->validContext(device, paint, matrix)) {
        return NULL;
    }

    char* shaderContextStorage = (char*)storage + sizeof(FilterShaderContext);
    SkShader::Context* shaderContext = fShader->createContext(device, paint, matrix,
                                                              shaderContextStorage);
    SkASSERT(shaderContext);

    return SkNEW_PLACEMENT_ARGS(storage, FilterShaderContext,
                                (*this, shaderContext, device, paint, matrix));
}

size_t SkFilterShader::contextSize() const {
    return sizeof(FilterShaderContext) + fShader->contextSize();
}

bool SkFilterShader::validContext(const SkBitmap& device,
                                  const SkPaint& paint,
                                  const SkMatrix& matrix,
                                  SkMatrix* totalInverse) const {
    return this->INHERITED::validContext(device, paint, matrix, totalInverse) &&
           fShader->validContext(device, paint, matrix);
}

SkFilterShader::FilterShaderContext::FilterShaderContext(const SkFilterShader& filterShader,
                                                         SkShader::Context* shaderContext,
                                                         const SkBitmap& device,
                                                         const SkPaint& paint,
                                                         const SkMatrix& matrix)
    : INHERITED(filterShader, device, paint, matrix)
    , fShaderContext(shaderContext) {}

SkFilterShader::FilterShaderContext::~FilterShaderContext() {
    fShaderContext->SkShader::Context::~Context();
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
