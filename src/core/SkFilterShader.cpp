/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFilterShader.h"

#include "SkColorFilter.h"
#include "SkFlattenableBuffers.h"
#include "SkShader.h"
#include "SkString.h"

SkFilterShader::SkFilterShader(SkShader* shader, SkColorFilter* filter) {
    fShader = shader;
    shader->ref();

    fFilter = filter;
    filter->ref();
}

SkFilterShader::SkFilterShader(SkFlattenableReadBuffer& buffer)
    : INHERITED(buffer) {
    fShader = buffer.readShader();
    fFilter = buffer.readColorFilter();
}

SkFilterShader::~SkFilterShader() {
    fFilter->unref();
    fShader->unref();
}

void SkFilterShader::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeFlattenable(fShader);
    buffer.writeFlattenable(fFilter);
}

uint32_t SkFilterShader::getFlags() {
    uint32_t shaderF = fShader->getFlags();
    uint32_t filterF = fFilter->getFlags();

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

bool SkFilterShader::setContext(const SkBitmap& device,
                                const SkPaint& paint,
                                const SkMatrix& matrix) {
    // we need to keep the setContext/endContext calls balanced. If we return
    // false, our endContext() will not be called.

    if (!this->INHERITED::setContext(device, paint, matrix)) {
        return false;
    }
    if (!fShader->setContext(device, paint, matrix)) {
        this->INHERITED::endContext();
        return false;
    }
    return true;
}

void SkFilterShader::endContext() {
    fShader->endContext();
    this->INHERITED::endContext();
}

void SkFilterShader::shadeSpan(int x, int y, SkPMColor result[], int count) {
    fShader->shadeSpan(x, y, result, count);
    fFilter->filterSpan(result, count, result);
}

void SkFilterShader::shadeSpan16(int x, int y, uint16_t result[], int count) {
    SkASSERT(fShader->getFlags() & SkShader::kHasSpan16_Flag);
    SkASSERT(fFilter->getFlags() & SkColorFilter::kHasFilter16_Flag);

    fShader->shadeSpan16(x, y, result, count);
    fFilter->filterSpan16(result, count, result);
}

#ifdef SK_DEVELOPER
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
