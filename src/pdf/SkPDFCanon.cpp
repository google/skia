/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPDFBitmap.h"
#include "SkPDFCanon.h"
#include "SkPDFFont.h"
#include "SkPDFGraphicState.h"
#include "SkPDFShader.h"

////////////////////////////////////////////////////////////////////////////////

SkPDFCanon::SkPDFCanon() {}

SkPDFCanon::~SkPDFCanon() {}

////////////////////////////////////////////////////////////////////////////////

template <class T> T* assert_ptr(T* p) { SkASSERT(p); return p; }

template <typename T>
bool remove_item(SkTDArray<T>* array, const T& elem) {
    int i = array->find(elem);
    if (i >= 0) {
        array->removeShuffle(i);
        return true;
    }
    return false;
}

// requires `bool T::equals(const U&) const`
template <typename T, typename U>
T* find_item(const SkTDArray<T*>& ptrArray, const U& object) {
    for (int i = 0; i < ptrArray.count(); ++i) {
        if (ptrArray[i]->equals(object)) {
            return ptrArray[i];
        }
    }
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////

SkPDFFont* SkPDFCanon::findFont(uint32_t fontID,
                                uint16_t glyphID,
                                SkPDFFont** relatedFontPtr) const {
    SkASSERT(relatedFontPtr);

    SkPDFFont* relatedFont = NULL;
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
    *relatedFontPtr = relatedFont;  // May still be NULL.
    return NULL;
}

void SkPDFCanon::addFont(SkPDFFont* font, uint32_t fontID, uint16_t fGlyphID) {
    SkPDFCanon::FontRec* rec = fFontRecords.push();
    rec->fFont = font;
    rec->fFontID = fontID;
    rec->fGlyphID = fGlyphID;
}

void SkPDFCanon::removeFont(SkPDFFont* pdfFont) {
    for (int i = 0; i < fFontRecords.count(); i++) {
        if (fFontRecords[i].fFont == pdfFont) {
            fFontRecords.removeShuffle(i);
            return;
        }
    }
    // Not all SkPDFFonts are added to the Canon.
}

////////////////////////////////////////////////////////////////////////////////

SkPDFFunctionShader* SkPDFCanon::findFunctionShader(
        const SkPDFShader::State& state) const {
    return find_item(fFunctionShaderRecords, state);
}
void SkPDFCanon::addFunctionShader(SkPDFFunctionShader* pdfShader) {
    fFunctionShaderRecords.push(assert_ptr(pdfShader));
}
void SkPDFCanon::removeFunctionShader(SkPDFFunctionShader* pdfShader) {
    SkAssertResult(remove_item(&fFunctionShaderRecords, pdfShader));
}

////////////////////////////////////////////////////////////////////////////////

SkPDFAlphaFunctionShader* SkPDFCanon::findAlphaShader(
        const SkPDFShader::State& state) const {
    return find_item(fAlphaShaderRecords, state);
}
void SkPDFCanon::addAlphaShader(SkPDFAlphaFunctionShader* pdfShader) {
    fAlphaShaderRecords.push(assert_ptr(pdfShader));
}
void SkPDFCanon::removeAlphaShader(SkPDFAlphaFunctionShader* pdfShader) {
    SkAssertResult(remove_item(&fAlphaShaderRecords, pdfShader));
}

////////////////////////////////////////////////////////////////////////////////

SkPDFImageShader* SkPDFCanon::findImageShader(
        const SkPDFShader::State& state) const {
    return find_item(fImageShaderRecords, state);
}

void SkPDFCanon::addImageShader(SkPDFImageShader* pdfShader) {
    fImageShaderRecords.push(assert_ptr(pdfShader));
}

void SkPDFCanon::removeImageShader(SkPDFImageShader* pdfShader) {
    SkAssertResult(remove_item(&fImageShaderRecords, pdfShader));
}

////////////////////////////////////////////////////////////////////////////////

SkPDFGraphicState* SkPDFCanon::findGraphicState(const SkPaint& paint) const {
    return find_item(fGraphicStateRecords, paint);
}

void SkPDFCanon::addGraphicState(SkPDFGraphicState* state) {
    fGraphicStateRecords.push(assert_ptr(state));
}

void SkPDFCanon::removeGraphicState(SkPDFGraphicState* pdfGraphicState) {
    SkAssertResult(remove_item(&fGraphicStateRecords, pdfGraphicState));
}

////////////////////////////////////////////////////////////////////////////////

SkPDFBitmap* SkPDFCanon::findBitmap(const SkBitmap& bm) const {
    return find_item(fBitmapRecords, bm);
}

void SkPDFCanon::addBitmap(SkPDFBitmap* pdfBitmap) {
    fBitmapRecords.push(assert_ptr(pdfBitmap));
}

void SkPDFCanon::removeBitmap(SkPDFBitmap* pdfBitmap) {
    SkAssertResult(remove_item(&fBitmapRecords, pdfBitmap));
}
