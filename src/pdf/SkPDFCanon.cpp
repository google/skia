/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage.h"
#include "SkPDFBitmap.h"
#include "SkPDFCanon.h"
#include "SkPDFFont.h"

////////////////////////////////////////////////////////////////////////////////

SkPDFCanon::~SkPDFCanon() {
    fGraphicStateRecords.foreach ([](WrapGS w) { w.fPtr->unref(); });
}

////////////////////////////////////////////////////////////////////////////////

template <typename T>
sk_sp<SkPDFObject> find_shader(const SkTArray<T>& records,
                               const SkPDFShader::State& state) {
    for (const T& record : records) {
        if (record.fShaderState == state) {
            return record.fShaderObject;
        }
    }
    return nullptr;
}

sk_sp<SkPDFObject> SkPDFCanon::findFunctionShader(
        const SkPDFShader::State& state) const {
    return find_shader(fFunctionShaderRecords, state);
}
void SkPDFCanon::addFunctionShader(sk_sp<SkPDFObject> pdfShader,
                                   SkPDFShader::State state) {
    fFunctionShaderRecords.emplace_back(ShaderRec{std::move(state), std::move(pdfShader)});
}

sk_sp<SkPDFObject> SkPDFCanon::findAlphaShader(
        const SkPDFShader::State& state) const {
    return find_shader(fAlphaShaderRecords, state);
}
void SkPDFCanon::addAlphaShader(sk_sp<SkPDFObject> pdfShader,
                                SkPDFShader::State state) {
    fAlphaShaderRecords.emplace_back(ShaderRec{std::move(state), std::move(pdfShader)});
}

sk_sp<SkPDFObject> SkPDFCanon::findImageShader(
        const SkPDFShader::State& state) const {
    return find_shader(fImageShaderRecords, state);
}

void SkPDFCanon::addImageShader(sk_sp<SkPDFObject> pdfShader,
                                SkPDFShader::State state) {
    fImageShaderRecords.emplace_back(ShaderRec{std::move(state), std::move(pdfShader)});
}

////////////////////////////////////////////////////////////////////////////////

const SkPDFGraphicState* SkPDFCanon::findGraphicState(
        const SkPDFGraphicState& key) const {
    const WrapGS* ptr = fGraphicStateRecords.find(WrapGS(&key));
    return ptr ? ptr->fPtr : nullptr;
}

void SkPDFCanon::addGraphicState(const SkPDFGraphicState* state) {
    SkASSERT(state);
    WrapGS w(SkRef(state));
    SkASSERT(!fGraphicStateRecords.contains(w));
    fGraphicStateRecords.add(w);
}

