/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "modules/skunicode/src/SkBidiFactory_icu_subset.h"

#include <unicode/umachine.h>

const char* SkBidiSubsetFactory::errorName(UErrorCode status) const {
    return u_errorName_skia(status);
}

SkBidiFactory::BidiCloseCallback SkBidiSubsetFactory::bidi_close_callback() const {
    return ubidi_close_skia;
}

UBiDiDirection SkBidiSubsetFactory::bidi_getDirection(const UBiDi* bidi) const {
    return ubidi_getDirection_skia(bidi);
}

SkBidiIterator::Position SkBidiSubsetFactory::bidi_getLength(const UBiDi* bidi) const {
    return ubidi_getLength_skia(bidi);
}

SkBidiIterator::Level SkBidiSubsetFactory::bidi_getLevelAt(const UBiDi* bidi, int pos) const {
    return ubidi_getLevelAt_skia(bidi, pos);
}

UBiDi* SkBidiSubsetFactory::bidi_openSized(int32_t maxLength,
                                           int32_t maxRunCount,
                                           UErrorCode* pErrorCode) const {
    return ubidi_openSized_skia(maxLength, maxRunCount, pErrorCode);
}

void SkBidiSubsetFactory::bidi_setPara(UBiDi* bidi,
                                       const UChar* text,
                                       int32_t length,
                                       UBiDiLevel paraLevel,
                                       UBiDiLevel* embeddingLevels,
                                       UErrorCode* status) const {
    return ubidi_setPara_skia(bidi, text, length, paraLevel, embeddingLevels, status);
}

void SkBidiSubsetFactory::bidi_reorderVisual(const SkUnicode::BidiLevel runLevels[],
                                             int levelsCount,
                                             int32_t logicalFromVisual[]) const {
    ubidi_reorderVisual_skia(runLevels, levelsCount, logicalFromVisual);
}
