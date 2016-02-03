/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRect_DEFINED
#define GrRect_DEFINED

#include "SkTypes.h"
#include "SkRect.h"

struct GrIRect16 {
    int16_t fLeft, fTop, fRight, fBottom;

    static GrIRect16 SK_WARN_UNUSED_RESULT MakeEmpty() {
        GrIRect16 r;
        r.setEmpty();
        return r;
    }

    static GrIRect16 SK_WARN_UNUSED_RESULT MakeWH(int16_t w, int16_t h) {
        GrIRect16 r;
        r.set(0, 0, w, h);
        return r;
    }

    static GrIRect16 SK_WARN_UNUSED_RESULT MakeXYWH(int16_t x, int16_t y, int16_t w, int16_t h) {
        GrIRect16 r;
        r.set(x, y, x + w, y + h);
        return r;
    }

    int width() const { return fRight - fLeft; }
    int height() const { return fBottom - fTop; }
    int area() const { return this->width() * this->height(); }
    bool isEmpty() const { return fLeft >= fRight || fTop >= fBottom; }

    void setEmpty() { memset(this, 0, sizeof(*this)); }

    void set(int16_t left, int16_t top, int16_t right, int16_t bottom) {
        fLeft = left;
        fTop = top;
        fRight = right;
        fBottom = bottom;
    }

    void set(const SkIRect& r) {
        fLeft   = SkToS16(r.fLeft);
        fTop    = SkToS16(r.fTop);
        fRight  = SkToS16(r.fRight);
        fBottom = SkToS16(r.fBottom);
    }
};

#endif
