/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SkPDFGraphicState.h"
#include "SkStream.h"
#include "SkTypeface.h"

SkPDFGraphicState::~SkPDFGraphicState() {
    SkAutoMutexAcquire lock(canonicalPaintsMutex());
    int index = find(fPaint);
    SkASSERT(index >= 0);
    canonicalPaints().removeShuffle(index);
}

void SkPDFGraphicState::emitObject(SkWStream* stream, SkPDFCatalog* catalog,
                                   bool indirect) {
    populateDict();
    SkPDFDict::emitObject(stream, catalog, indirect);
}

size_t SkPDFGraphicState::getOutputSize(SkPDFCatalog* catalog, bool indirect) {
    populateDict();
    return SkPDFDict::getOutputSize(catalog, indirect);
}

// static
SkTDArray<SkPDFGraphicState::GSCanonicalEntry>&
SkPDFGraphicState::canonicalPaints() {
    // This initialization is only thread safe with gcc.
    static SkTDArray<SkPDFGraphicState::GSCanonicalEntry> gCanonicalPaints;
    return gCanonicalPaints;
}

// static
SkMutex& SkPDFGraphicState::canonicalPaintsMutex() {
    // This initialization is only thread safe with gcc.
    static SkMutex gCanonicalPaintsMutex;
    return gCanonicalPaintsMutex;
}

// static
SkPDFGraphicState* SkPDFGraphicState::getGraphicStateForPaint(
        const SkPaint& paint) {
    SkAutoMutexAcquire lock(canonicalPaintsMutex());
    int index = find(paint);
    if (index >= 0) {
        canonicalPaints()[index].fGraphicState->ref();
        return canonicalPaints()[index].fGraphicState;
    }
    GSCanonicalEntry newEntry(new SkPDFGraphicState(paint));
    canonicalPaints().push(newEntry);
    return newEntry.fGraphicState;
}

// static
int SkPDFGraphicState::find(const SkPaint& paint) {
    GSCanonicalEntry search(&paint);
    return canonicalPaints().find(search);
}

SkPDFGraphicState::SkPDFGraphicState(const SkPaint& paint)
    : fPaint(paint),
      fPopulated(false) {
}

// populateDict and operator== have to stay in sync with each other.
void SkPDFGraphicState::populateDict() {
    if (!fPopulated) {
        fPopulated = true;
        SkRefPtr<SkPDFName> typeName = new SkPDFName("ExtGState");
        typeName->unref();  // SkRefPtr and new both took a reference.
        insert("Type", typeName.get());

        SkScalar maxAlpha = SkIntToScalar(0xFF);
        SkRefPtr<SkPDFScalar> alpha =
            new SkPDFScalar(SkColorGetA(fPaint.getColor())/maxAlpha);
        alpha->unref();  // SkRefPtr and new both took a reference.
        insert("CA", alpha.get());
        insert("ca", alpha.get());

        SkASSERT(SkPaint::kButt_Cap == 0);
        SkASSERT(SkPaint::kRound_Cap == 1);
        SkASSERT(SkPaint::kSquare_Cap == 2);
        SkASSERT(fPaint.getStrokeCap() >= 0 && fPaint.getStrokeCap() <= 2);
        SkRefPtr<SkPDFInt> strokeCap = new SkPDFInt(fPaint.getStrokeCap());
        strokeCap->unref();  // SkRefPtr and new both took a reference.
        insert("LC", strokeCap.get());

        SkASSERT(SkPaint::kMiter_Join == 0);
        SkASSERT(SkPaint::kRound_Join == 1);
        SkASSERT(SkPaint::kBevel_Join == 2);
        SkASSERT(fPaint.getStrokeJoin() >= 0 && fPaint.getStrokeJoin() <= 2);
        SkRefPtr<SkPDFInt> strokeJoin = new SkPDFInt(fPaint.getStrokeJoin());
        strokeJoin->unref();  // SkRefPtr and new both took a reference.
        insert("LJ", strokeJoin.get());

        /* TODO(vandebo) Font.
        if (fPaint.getTypeFace() != NULL) {
            SkRefPtr<SkPDFTypeFace> typeFace =
                SkPDFTypeFace::getFontForTypeFace(fPaint.getTypeFace);
            SkRefPtr<SkPDFObjRef> typeFaceRef = new SkPDFObjRef(typeFace.get());
            fontRef->unref();  // SkRefPtr and new both took a reference.
            SkRefPtr<SkPDFScalar> fontSize =
                new SkPDFScalar(fPaint.getTetSize());
            fontSize->unref();  // SkRefPtr and new both took a reference.
            SkRefPtr<SkPDFArray> font = new SkPDFArray();
            font->unref();  // SkRefPtr and new both took a reference.
            font->reserve(2);
            font->append(typeFaceRef.get());
            font->append(fontSize.get());
            insert("LJ", font.get());
        }
        */

        SkRefPtr<SkPDFScalar> strokeWidth =
            new SkPDFScalar(fPaint.getStrokeWidth());
        strokeWidth->unref();  // SkRefPtr and new both took a reference.
        insert("LW", strokeWidth.get());

        SkRefPtr<SkPDFScalar> strokeMiterLimit = new SkPDFScalar(
            fPaint.getStrokeMiter());
        strokeMiterLimit->unref();  // SkRefPtr and new both took a reference.
        insert("ML", strokeWidth.get());

        // Turn on automatic stroke adjustment.
        SkRefPtr<SkPDFBool> trueVal = new SkPDFBool(true);
        trueVal->unref();  // SkRefPtr and new both took a reference.
        insert("SA", trueVal.get());
    }
}

// We're only interested in some fields of the SkPaint, so we have a custom
// operator== function.
bool SkPDFGraphicState::GSCanonicalEntry::operator==(
        const SkPDFGraphicState::GSCanonicalEntry& gs) const {
    const SkPaint* a = fPaint;
    const SkPaint* b = gs.fPaint;
    SkASSERT(a != NULL);
    SkASSERT(b != NULL);
    SkTypeface* aFace = a->getTypeface();
    SkTypeface* bFace = b->getTypeface();
    return SkColorGetA(a->getColor()) == SkColorGetA(b->getColor()) &&
           a->getStrokeCap() == b->getStrokeCap() &&
           a->getStrokeJoin() == b->getStrokeJoin() &&
           a->getTextSize() == b->getTextSize() &&
           a->getStrokeWidth() == b->getStrokeWidth() &&
           a->getStrokeMiter() == b->getStrokeMiter() &&
           (aFace == NULL) == (bFace == NULL) &&
           (aFace == NULL || aFace->uniqueID() == bFace->uniqueID());
}
