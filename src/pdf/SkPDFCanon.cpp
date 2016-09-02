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

namespace {
template <typename K, typename V> struct UnrefValue {
    void operator()(K, V** v) { SkSafeUnref(*v); }
};
}

SkPDFCanon::~SkPDFCanon() {
    // TODO(halcanary): make SkTHashSet work nicely with sk_sp<>,
    // or use std::unordered_set<>
    fGraphicStateRecords.foreach ([](WrapGS w) { w.fPtr->unref(); });
    fPDFBitmapMap.foreach(UnrefValue<SkBitmapKey, SkPDFObject>());
    fTypefaceMetrics.foreach(UnrefValue<uint32_t, SkAdvancedTypefaceMetrics>());
    fFontDescriptors.foreach(UnrefValue<uint32_t, SkPDFDict>());
    fFontMap.foreach(UnrefValue<uint64_t, SkPDFFont>());
}

void SkPDFCanon::reset() {
    this->~SkPDFCanon();
    new (this)SkPDFCanon;
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
    fFunctionShaderRecords.emplace_back(std::move(state), std::move(pdfShader));
}

sk_sp<SkPDFObject> SkPDFCanon::findAlphaShader(
        const SkPDFShader::State& state) const {
    return find_shader(fAlphaShaderRecords, state);
}
void SkPDFCanon::addAlphaShader(sk_sp<SkPDFObject> pdfShader,
                                SkPDFShader::State state) {
    fAlphaShaderRecords.emplace_back(std::move(state), std::move(pdfShader));
}

sk_sp<SkPDFObject> SkPDFCanon::findImageShader(
        const SkPDFShader::State& state) const {
    return find_shader(fImageShaderRecords, state);
}

void SkPDFCanon::addImageShader(sk_sp<SkPDFObject> pdfShader,
                                SkPDFShader::State state) {
    fImageShaderRecords.emplace_back(std::move(state), std::move(pdfShader));
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

////////////////////////////////////////////////////////////////////////////////

sk_sp<SkPDFObject> SkPDFCanon::findPDFBitmap(SkBitmapKey key) const {
    SkPDFObject** ptr = fPDFBitmapMap.find(key);
    return ptr ? sk_ref_sp(*ptr) : sk_sp<SkPDFObject>();
}

void SkPDFCanon::addPDFBitmap(SkBitmapKey key, sk_sp<SkPDFObject> pdfBitmap) {
    fPDFBitmapMap.set(key, pdfBitmap.release());
}

////////////////////////////////////////////////////////////////////////////////

sk_sp<SkPDFStream> SkPDFCanon::makeInvertFunction() {
    if (fInvertFunction) {
        return fInvertFunction;
    }
    fInvertFunction = SkPDFGraphicState::MakeInvertFunction();
    return fInvertFunction;
}
sk_sp<SkPDFDict> SkPDFCanon::makeNoSmaskGraphicState() {
    if (fNoSmaskGraphicState) {
        return fNoSmaskGraphicState;
    }
    fNoSmaskGraphicState = SkPDFGraphicState::MakeNoSmaskGraphicState();
    return fNoSmaskGraphicState;
}
sk_sp<SkPDFArray> SkPDFCanon::makeRangeObject() {
    if (fRangeObject) {
        return fRangeObject;
    }
    fRangeObject = SkPDFShader::MakeRangeObject();
    return fRangeObject;
}
