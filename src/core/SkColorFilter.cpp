
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkColorFilter.h"
#include "SkShader.h"
#include "SkUnPreMultiply.h"

bool SkColorFilter::asColorMode(SkColor* color, SkXfermode::Mode* mode) {
    return false;
}

bool SkColorFilter::asColorMatrix(SkScalar matrix[20]) {
    return false;
}

bool SkColorFilter::asComponentTable(SkBitmap*) {
    return false;
}

void SkColorFilter::filterSpan16(const uint16_t s[], int count, uint16_t d[]) {
    SkASSERT(this->getFlags() & SkColorFilter::kHasFilter16_Flag);
    SkDEBUGFAIL("missing implementation of SkColorFilter::filterSpan16");

    if (d != s) {
        memcpy(d, s, count * sizeof(uint16_t));
    }
}

SkColor SkColorFilter::filterColor(SkColor c) {
    SkPMColor dst, src = SkPreMultiplyColor(c);
    this->filterSpan(&src, 1, &dst);
    return SkUnPreMultiply::PMColorToColor(dst);
}

///////////////////////////////////////////////////////////////////////////////

SkFilterShader::SkFilterShader(SkShader* shader, SkColorFilter* filter) {
    fShader = shader;   shader->ref();
    fFilter = filter;   filter->ref();
}

SkFilterShader::SkFilterShader(SkFlattenableReadBuffer& buffer) :
        INHERITED(buffer) {
    fShader = static_cast<SkShader*>(buffer.readFlattenable());
    fFilter = static_cast<SkColorFilter*>(buffer.readFlattenable());
}

SkFilterShader::~SkFilterShader() {
    fFilter->unref();
    fShader->unref();
}

void SkFilterShader::beginSession() {
    this->INHERITED::beginSession();
    fShader->beginSession();
}

void SkFilterShader::endSession() {
    fShader->endSession();
    this->INHERITED::endSession();
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
    return  this->INHERITED::setContext(device, paint, matrix) &&
            fShader->setContext(device, paint, matrix);
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

