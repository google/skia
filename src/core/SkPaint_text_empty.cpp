/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPaint.h"

bool SkPaint::TooBigToUseCache(const SkMatrix& ctm, const SkMatrix& textM, SkScalar maxLimit) {
    SkASSERT_RELEASE(false);
    return false;
}
SkScalar SkPaint::MaxCacheSize2(SkScalar maxLimit) {
    SkASSERT_RELEASE(false);
    return 0;
}
int SkPaint::countText(const void* text, size_t byteLength) const {
    SkASSERT_RELEASE(false);
    return 0;
}
int SkPaint::textToGlyphs(const void* textData, size_t byteLength, uint16_t glyphs[]) const {
    SkASSERT_RELEASE(false);
    return 0;
}
bool SkPaint::containsText(const void* textData, size_t byteLength) const {
    SkASSERT_RELEASE(false);
    return false;
}
void SkPaint::glyphsToUnichars(const uint16_t glyphs[], int count, SkUnichar textData[]) const {
    SkASSERT_RELEASE(false);
}
SkPaint::GlyphCacheProc SkPaint::GetGlyphCacheProc(TextEncoding encoding,
                                                  bool needFullMetrics) {
    SkASSERT_RELEASE(false);
    return nullptr;
}
SkScalar SkPaint::setupForAsPaths() {
    SkASSERT_RELEASE(false);
    return 0;
}
SkScalar SkPaint::measure_text(SkGlyphCache* cache,
                               const char* text, size_t byteLength,
                               int* count, SkRect* bounds) const {
    SkASSERT_RELEASE(false);
    return 0;
}
SkScalar SkPaint::measureText(const void* textData, size_t length, SkRect* bounds) const {
    SkASSERT_RELEASE(false);
    return 0;
}
size_t SkPaint::breakText(const void* textD, size_t length, SkScalar maxWidth,
                          SkScalar* measuredWidth) const {
    SkASSERT_RELEASE(false);
    return 0;
}
SkScalar SkPaint::getFontMetrics(FontMetrics* metrics, SkScalar zoom) const {
    SkASSERT_RELEASE(false);
    return 0;
}
int SkPaint::getTextWidths(const void* textData, size_t byteLength,
                           SkScalar widths[], SkRect bounds[]) const {
    SkASSERT_RELEASE(false);
    return 0;
}
void SkPaint::getTextPath(const void* textData, size_t length,
                          SkScalar x, SkScalar y, SkPath* path) const {
    SkASSERT_RELEASE(false);
}
void SkPaint::getPosTextPath(const void* textData, size_t length,
                             const SkPoint pos[], SkPath* path) const {
    SkASSERT_RELEASE(false);
}
int SkPaint::getTextIntercepts(const void* textData, size_t length,
                               SkScalar x, SkScalar y, const SkScalar bounds[2],
                               SkScalar* array) const {
    SkASSERT_RELEASE(false);
    return 0;
}
int SkPaint::getPosTextIntercepts(const void* textData, size_t length, const SkPoint pos[],
                                  const SkScalar bounds[2], SkScalar* array) const {
    SkASSERT_RELEASE(false);
    return 0;
}
int SkPaint::getPosTextHIntercepts(const void* textData, size_t length, const SkScalar xpos[],
                                   SkScalar constY, const SkScalar bounds[2],
                                   SkScalar* array) const {
    SkASSERT_RELEASE(false);
    return 0;
}
int SkPaint::getTextBlobIntercepts(const SkTextBlob* blob, const SkScalar bounds[2],
                                   SkScalar* intervals) const {
    SkASSERT_RELEASE(false);
    return 0;
}
SkRect SkPaint::getFontBounds() const {
    SkASSERT_RELEASE(false);
    return {0,0,0,0};
}
SkColor SkPaint::computeLuminanceColor() const {
    SkASSERT_RELEASE(false);
    return 0;
}
