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
#include "SkPDFShader.h"

////////////////////////////////////////////////////////////////////////////////

void SkPDFCanon::reset() {
    for (int i = 0; i < fFontRecords.count(); ++i) {
        fFontRecords[i].fFont->unref();
    }
    fFontRecords.reset();
    fFunctionShaderRecords.unrefAll();
    fFunctionShaderRecords.reset();
    fAlphaShaderRecords.unrefAll();
    fAlphaShaderRecords.reset();
    fImageShaderRecords.unrefAll();
    fImageShaderRecords.reset();
    fGraphicStateRecords.foreach ([](WrapGS w) { w.fPtr->unref(); });
    fGraphicStateRecords.reset();

    fPDFBitmapMap.foreach([](SkBitmapKey, SkPDFObject** p) { (*p)->unref(); });
    fPDFBitmapMap.reset();
}

////////////////////////////////////////////////////////////////////////////////

template <class T> T* assert_ptr(T* p) { SkASSERT(p); return p; }

// requires `bool T::equals(const U&) const`
template <typename T, typename U>
T* find_item(const SkTDArray<T*>& ptrArray, const U& object) {
    for (int i = 0; i < ptrArray.count(); ++i) {
        if (ptrArray[i]->equals(object)) {
            return ptrArray[i];
        }
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////

SkPDFFont* SkPDFCanon::findFont(uint32_t fontID,
                                uint16_t glyphID,
                                SkPDFFont** relatedFontPtr) const {
    SkASSERT(relatedFontPtr);

    SkPDFFont* relatedFont = nullptr;
    for (int i = 0; i < fFontRecords.count(); ++i) {
        SkPDFFont::Match match = SkPDFFont::IsMatch(
                fFontRecords[i].fFont, fFontRecords[i].fFontID,
                fFontRecords[i].fGlyphID, fontID, glyphID);
        if (SkPDFFont::kExact_Match == match) {
            return fFontRecords[i].fFont;
        } else if (!relatedFont && SkPDFFont::kRelated_Match == match) {
            relatedFont = fFontRecords[i].fFont;
        }
    }
    *relatedFontPtr = relatedFont;  // May still be nullptr.
    return nullptr;
}

void SkPDFCanon::addFont(SkPDFFont* font, uint32_t fontID, uint16_t fGlyphID) {
    SkPDFCanon::FontRec* rec = fFontRecords.push();
    rec->fFont = SkRef(font);
    rec->fFontID = fontID;
    rec->fGlyphID = fGlyphID;
}

////////////////////////////////////////////////////////////////////////////////

SkPDFFunctionShader* SkPDFCanon::findFunctionShader(
        const SkPDFShader::State& state) const {
    return find_item(fFunctionShaderRecords, state);
}
void SkPDFCanon::addFunctionShader(SkPDFFunctionShader* pdfShader) {
    fFunctionShaderRecords.push(SkRef(pdfShader));
}

////////////////////////////////////////////////////////////////////////////////

SkPDFAlphaFunctionShader* SkPDFCanon::findAlphaShader(
        const SkPDFShader::State& state) const {
    return find_item(fAlphaShaderRecords, state);
}
void SkPDFCanon::addAlphaShader(SkPDFAlphaFunctionShader* pdfShader) {
    fAlphaShaderRecords.push(SkRef(pdfShader));
}

////////////////////////////////////////////////////////////////////////////////

SkPDFImageShader* SkPDFCanon::findImageShader(
        const SkPDFShader::State& state) const {
    return find_item(fImageShaderRecords, state);
}

void SkPDFCanon::addImageShader(SkPDFImageShader* pdfShader) {
    fImageShaderRecords.push(SkRef(pdfShader));
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
