
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkTextLayout.h"

SK_DEFINE_INST_COUNT(SkTextStyle)

SkTextStyle::SkTextStyle() {
    fPaint.setAntiAlias(true);
}

SkTextStyle::SkTextStyle(const SkTextStyle& src) : fPaint(src.fPaint) {}

SkTextStyle::SkTextStyle(const SkPaint& paint) : fPaint(paint) {}

SkTextStyle::~SkTextStyle() {}

///////////////////////////////////////////////////////////////////////////////

SkTextLayout::SkTextLayout() {
    fBounds.setEmpty();
    fDefaultStyle = new SkTextStyle;
}

SkTextLayout::~SkTextLayout() {
    fDefaultStyle->unref();
    fLines.deleteAll();
}

void SkTextLayout::setText(const char text[], size_t length) {
    fText.setCount(length);
    memcpy(fText.begin(), text, length);
}

void SkTextLayout::setBounds(const SkRect& bounds) {
    fBounds = bounds;
    // if width changed, inval cache
}

SkTextStyle* SkTextLayout::setDefaultStyle(SkTextStyle* style) {
    SkRefCnt_SafeAssign(fDefaultStyle, style);
    return style;
}

///////////////////////////////////////////////////////////////////////////////

struct SkTextLayout::GlyphRun {
    GlyphRun();
    ~GlyphRun();

    SkPoint*    fLocs;
    uint16_t*   fGlyphIDs;
    int         fCount;
};

SkTextLayout::GlyphRun::GlyphRun() : fLocs(NULL), fGlyphIDs(NULL), fCount(0) {}

SkTextLayout::GlyphRun::~GlyphRun() {
    delete[] fLocs;
    delete[] fGlyphIDs;
}

struct SkTextLayout::Line {
    Line() {}
    ~Line();

    SkScalar                fBaselineY;
    SkTDArray<GlyphRun*>    fRuns;
};

SkTextLayout::Line::~Line() {
    fRuns.deleteAll();
}

void SkTextLayout::draw(SkCanvas* canvas) {
}

