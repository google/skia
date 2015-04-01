/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

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
    fBitmapRecords.unrefAll();
    fBitmapRecords.reset();
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
    return ptr ? ptr->fPtr : NULL;
}

void SkPDFCanon::addGraphicState(const SkPDFGraphicState* state) {
    SkASSERT(state);
    WrapGS w(SkRef(state));
    SkASSERT(!fGraphicStateRecords.contains(w));
    fGraphicStateRecords.add(w);
}

////////////////////////////////////////////////////////////////////////////////

SkPDFBitmap* SkPDFCanon::findBitmap(const SkBitmap& bm) const {
    return find_item(fBitmapRecords, bm);
}

void SkPDFCanon::addBitmap(SkPDFBitmap* pdfBitmap) {
    fBitmapRecords.push(SkRef(pdfBitmap));
}
