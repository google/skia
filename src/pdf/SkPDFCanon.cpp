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

SkPDFShader* SkPDFCanon::findShader(const SkPDFShader::State& state) const {
    assert_mutex_held(this, gSkPDFCanonShaderMutex);
    for (int i = 0; i < fShaderRecords.count(); ++i) {
        if (fShaderRecords[i]->equals(state)) {
            return fShaderRecords[i];
        }
    }
    return NULL;
}

void SkPDFCanon::addShader(SkPDFShader* shader) {
    assert_mutex_held(this, gSkPDFCanonShaderMutex);
    SkASSERT(shader);
    fShaderRecords.push(shader);
}

void SkPDFCanon::removeShader(SkPDFShader* pdfShader) {
    assert_mutex_held(this, gSkPDFCanonShaderMutex);
    for (int i = 0; i < fShaderRecords.count(); ++i) {
        if (fShaderRecords[i] == pdfShader) {
            fShaderRecords.removeShuffle(i);
            return;
        }
    }
    SkDEBUGFAIL("pdfShader not found");
}

////////////////////////////////////////////////////////////////////////////////

SkPDFGraphicState* SkPDFCanon::findGraphicState(const SkPaint& paint) const {
    assert_mutex_held(this, gSkPDFCanonPaintMutex);
    for (int i = 0; i < fGraphicStateRecords.count(); ++i) {
        if (fGraphicStateRecords[i]->equals(paint)) {
            return fGraphicStateRecords[i];
        }
    }
    return NULL;
}

void SkPDFCanon::addGraphicState(SkPDFGraphicState* state) {
    assert_mutex_held(this, gSkPDFCanonPaintMutex);
    SkASSERT(state);
    fGraphicStateRecords.push(state);
}

void SkPDFCanon::removeGraphicState(SkPDFGraphicState* pdfGraphicState) {
    assert_mutex_held(this, gSkPDFCanonPaintMutex);
    for (int i = 0; i < fGraphicStateRecords.count(); ++i) {
        if (fGraphicStateRecords[i] == pdfGraphicState) {
            fGraphicStateRecords.removeShuffle(i);
            return;
        }
    }
    SkDEBUGFAIL("pdfGraphicState not found");
}
