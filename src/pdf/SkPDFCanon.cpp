/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLazyPtr.h"
#include "SkPDFCanon.h"
#include "SkPDFFont.h"
#include "SkPDFGraphicState.h"
#include "SkPDFShader.h"

////////////////////////////////////////////////////////////////////////////////

SK_DECLARE_STATIC_MUTEX(gSkPDFCanonFontMutex);
SK_DECLARE_STATIC_MUTEX(gSkPDFCanonShaderMutex);
SK_DECLARE_STATIC_MUTEX(gSkPDFCanonPaintMutex);

SkBaseMutex& SkPDFCanon::GetFontMutex() { return gSkPDFCanonFontMutex; }
SkBaseMutex& SkPDFCanon::GetShaderMutex() { return gSkPDFCanonShaderMutex; }
SkBaseMutex& SkPDFCanon::GetPaintMutex() { return gSkPDFCanonPaintMutex; }

SkPDFCanon::SkPDFCanon() {}

SkPDFCanon::~SkPDFCanon() {}

SK_DECLARE_STATIC_LAZY_PTR(SkPDFCanon, singleton);

SkPDFCanon& SkPDFCanon::GetCanon() { return *singleton.get(); }

////////////////////////////////////////////////////////////////////////////////

static void assert_mutex_held(const SkPDFCanon* canon, SkBaseMutex& mutex) {
    if (canon == singleton.get()) {
        mutex.assertHeld();
    }
}

template <class T> T* assert_ptr(T* p) { SkASSERT(p); return p; }

// TODO(halcanary):  add this method to SkTDArray.
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
    assert_mutex_held(this, gSkPDFCanonFontMutex);
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
    assert_mutex_held(this, gSkPDFCanonFontMutex);
    SkPDFCanon::FontRec* rec = fFontRecords.push();
    rec->fFont = font;
    rec->fFontID = fontID;
    rec->fGlyphID = fGlyphID;
}

void SkPDFCanon::removeFont(SkPDFFont* pdfFont) {
    assert_mutex_held(this, gSkPDFCanonFontMutex);
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
    assert_mutex_held(this, gSkPDFCanonShaderMutex);
    return find_item(fFunctionShaderRecords, state);
}
void SkPDFCanon::addFunctionShader(SkPDFFunctionShader* pdfShader) {
    assert_mutex_held(this, gSkPDFCanonShaderMutex);
    fFunctionShaderRecords.push(assert_ptr(pdfShader));
}
void SkPDFCanon::removeFunctionShader(SkPDFFunctionShader* pdfShader) {
    assert_mutex_held(this, gSkPDFCanonShaderMutex);
    SkAssertResult(remove_item(&fFunctionShaderRecords, pdfShader));
}

////////////////////////////////////////////////////////////////////////////////

SkPDFAlphaFunctionShader* SkPDFCanon::findAlphaShader(
        const SkPDFShader::State& state) const {
    assert_mutex_held(this, gSkPDFCanonShaderMutex);
    return find_item(fAlphaShaderRecords, state);
}
void SkPDFCanon::addAlphaShader(SkPDFAlphaFunctionShader* pdfShader) {
    assert_mutex_held(this, gSkPDFCanonShaderMutex);
    fAlphaShaderRecords.push(assert_ptr(pdfShader));
}
void SkPDFCanon::removeAlphaShader(SkPDFAlphaFunctionShader* pdfShader) {
    assert_mutex_held(this, gSkPDFCanonShaderMutex);
    SkAssertResult(remove_item(&fAlphaShaderRecords, pdfShader));
}

////////////////////////////////////////////////////////////////////////////////

SkPDFImageShader* SkPDFCanon::findImageShader(
        const SkPDFShader::State& state) const {
    assert_mutex_held(this, gSkPDFCanonShaderMutex);
    return find_item(fImageShaderRecords, state);
}

void SkPDFCanon::addImageShader(SkPDFImageShader* pdfShader) {
    assert_mutex_held(this, gSkPDFCanonShaderMutex);
    fImageShaderRecords.push(assert_ptr(pdfShader));
}

void SkPDFCanon::removeImageShader(SkPDFImageShader* pdfShader) {
    assert_mutex_held(this, gSkPDFCanonShaderMutex);
    SkAssertResult(remove_item(&fImageShaderRecords, pdfShader));
}

////////////////////////////////////////////////////////////////////////////////

SkPDFGraphicState* SkPDFCanon::findGraphicState(const SkPaint& paint) const {
    assert_mutex_held(this, gSkPDFCanonPaintMutex);
    return find_item(fGraphicStateRecords, paint);
}

void SkPDFCanon::addGraphicState(SkPDFGraphicState* state) {
    assert_mutex_held(this, gSkPDFCanonPaintMutex);
    fGraphicStateRecords.push(assert_ptr(state));
}

void SkPDFCanon::removeGraphicState(SkPDFGraphicState* pdfGraphicState) {
    assert_mutex_held(this, gSkPDFCanonPaintMutex);
    SkAssertResult(remove_item(&fGraphicStateRecords, pdfGraphicState));
}
