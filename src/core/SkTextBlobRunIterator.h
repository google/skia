/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkTextBlobRunIterator_DEFINED
#define SkTextBlobRunIterator_DEFINED

#include "SkTextBlob.h"

/**
 *  Iterate through all of the text runs of the text blob.  For example:
 *    for (SkTextBlobRunIterator it(blob); !it.done(); it.next()) {
 *         .....
 *    }
 */
class SkTextBlobRunIterator {
public:
    SkTextBlobRunIterator(const SkTextBlob* blob);

    bool done() const;
    void next();

    uint32_t glyphCount() const;
    const uint16_t* glyphs() const;
    const SkScalar* pos() const;
    const SkPoint& offset() const;
    void applyFontToPaint(SkPaint*) const;
    SkTextBlob::GlyphPositioning positioning() const;
    bool isLCD() const;

private:
    const SkTextBlob::RunRecord* fCurrentRun;
    int fRemainingRuns;

    SkDEBUGCODE(uint8_t* fStorageTop;)
};

#endif  // SkTextBlobRunIterator_DEFINED
