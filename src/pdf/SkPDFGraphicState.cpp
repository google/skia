/*
 * Copyright (C) 2011 Google Inc.
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
        insert("Type", new SkPDFName("ExtGState"))->unref();

        SkRefPtr<SkPDFScalar> alpha =
            new SkPDFScalar(fPaint.getAlpha() * SkScalarInvert(0xFF));
        alpha->unref();  // SkRefPtr and new both took a reference.
        insert("CA", alpha.get());
        insert("ca", alpha.get());

        SK_COMPILE_ASSERT(SkPaint::kButt_Cap == 0, paint_cap_mismatch);
        SK_COMPILE_ASSERT(SkPaint::kRound_Cap == 1, paint_cap_mismatch);
        SK_COMPILE_ASSERT(SkPaint::kSquare_Cap == 2, paint_cap_mismatch);
        SK_COMPILE_ASSERT(SkPaint::kCapCount == 3, paint_cap_mismatch);
        SkASSERT(fPaint.getStrokeCap() >= 0 && fPaint.getStrokeCap() <= 2);
        insert("LC", new SkPDFInt(fPaint.getStrokeCap()))->unref();

        SK_COMPILE_ASSERT(SkPaint::kMiter_Join == 0, paint_join_mismatch);
        SK_COMPILE_ASSERT(SkPaint::kRound_Join == 1, paint_join_mismatch);
        SK_COMPILE_ASSERT(SkPaint::kBevel_Join == 2, paint_join_mismatch);
        SK_COMPILE_ASSERT(SkPaint::kJoinCount == 3, paint_join_mismatch);
        SkASSERT(fPaint.getStrokeJoin() >= 0 && fPaint.getStrokeJoin() <= 2);
        insert("LJ", new SkPDFInt(fPaint.getStrokeJoin()))->unref();

        insert("LW", new SkPDFScalar(fPaint.getStrokeWidth()))->unref();
        insert("ML", new SkPDFScalar(fPaint.getStrokeMiter()))->unref();
        insert("SA", new SkPDFBool(true))->unref();  // Auto stroke adjustment.
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
    return SkColorGetA(a->getColor()) == SkColorGetA(b->getColor()) &&
           a->getStrokeCap() == b->getStrokeCap() &&
           a->getStrokeJoin() == b->getStrokeJoin() &&
           a->getStrokeWidth() == b->getStrokeWidth() &&
           a->getStrokeMiter() == b->getStrokeMiter();
}
