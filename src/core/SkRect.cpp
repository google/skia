
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRect.h"

void SkIRect::join(int32_t left, int32_t top, int32_t right, int32_t bottom) {
    // do nothing if the params are empty
    if (left >= right || top >= bottom) {
        return;
    }

    // if we are empty, just assign
    if (fLeft >= fRight || fTop >= fBottom) {
        this->set(left, top, right, bottom);
    } else {
        if (left < fLeft) fLeft = left;
        if (top < fTop) fTop = top;
        if (right > fRight) fRight = right;
        if (bottom > fBottom) fBottom = bottom;
    }
}

void SkIRect::sort() {
    if (fLeft > fRight) {
        SkTSwap<int32_t>(fLeft, fRight);
    }
    if (fTop > fBottom) {
        SkTSwap<int32_t>(fTop, fBottom);
    }
}

/////////////////////////////////////////////////////////////////////////////

void SkRect::toQuad(SkPoint quad[4]) const {
    SkASSERT(quad);

    quad[0].set(fLeft, fTop);
    quad[1].set(fRight, fTop);
    quad[2].set(fRight, fBottom);
    quad[3].set(fLeft, fBottom);
}

bool SkRect::setBoundsCheck(const SkPoint pts[], int count) {
    SkASSERT((pts && count > 0) || count == 0);

    bool isFinite = true;

    if (count <= 0) {
        sk_bzero(this, sizeof(SkRect));
    } else {
        SkScalar    l, t, r, b;

        l = r = pts[0].fX;
        t = b = pts[0].fY;

        // If all of the points are finite, accum should stay 0. If we encounter
        // a NaN or infinity, then accum should become NaN.
        float accum = 0;
        accum *= l; accum *= t;

        for (int i = 1; i < count; i++) {
            SkScalar x = pts[i].fX;
            SkScalar y = pts[i].fY;

            accum *= x; accum *= y;

            // we use if instead of if/else, so we can generate min/max
            // float instructions (at least on SSE)
            if (x < l) l = x;
            if (x > r) r = x;

            if (y < t) t = y;
            if (y > b) b = y;
        }

        SkASSERT(!accum || !SkScalarIsFinite(accum));
        if (accum) {
            l = t = r = b = 0;
            isFinite = false;
        }
        this->set(l, t, r, b);
    }

    return isFinite;
}

#define CHECK_INTERSECT(al, at, ar, ab, bl, bt, br, bb) \
    SkScalar L = SkMaxScalar(al, bl);                   \
    SkScalar R = SkMinScalar(ar, br);                   \
    SkScalar T = SkMaxScalar(at, bt);                   \
    SkScalar B = SkMinScalar(ab, bb);                   \
    do { if (L >= R || T >= B) return false; } while (0)

bool SkRect::intersect(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom) {
    CHECK_INTERSECT(left, top, right, bottom, fLeft, fTop, fRight, fBottom);
    this->setLTRB(L, T, R, B);
    return true;
}

bool SkRect::intersect(const SkRect& r) {
    return this->intersect(r.fLeft, r.fTop, r.fRight, r.fBottom);
}

bool SkRect::intersect(const SkRect& a, const SkRect& b) {
    CHECK_INTERSECT(a.fLeft, a.fTop, a.fRight, a.fBottom, b.fLeft, b.fTop, b.fRight, b.fBottom);
    this->setLTRB(L, T, R, B);
    return true;
}

void SkRect::join(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom) {
    // do nothing if the params are empty
    if (left >= right || top >= bottom) {
        return;
    }

    // if we are empty, just assign
    if (fLeft >= fRight || fTop >= fBottom) {
        this->set(left, top, right, bottom);
    } else {
        fLeft   = SkMinScalar(fLeft, left);
        fTop    = SkMinScalar(fTop, top);
        fRight  = SkMaxScalar(fRight, right);
        fBottom = SkMaxScalar(fBottom, bottom);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkString.h"
#include "SkStringUtils.h"

static const char* set_scalar(SkString* storage, SkScalar value, SkScalarAsStringType asType) {
    storage->reset();
    SkAppendScalar(storage, value, asType);
    return storage->c_str();
}

void SkRect::dump(bool asHex) const {
    SkScalarAsStringType asType = asHex ? kHex_SkScalarAsStringType : kDec_SkScalarAsStringType;

    SkString line;
    if (asHex) {
        SkString tmp;
        line.printf( "SkRect::MakeLTRB(%s, /* %f */\n", set_scalar(&tmp, fLeft, asType), fLeft);
        line.appendf("                 %s, /* %f */\n", set_scalar(&tmp, fTop, asType), fTop);
        line.appendf("                 %s, /* %f */\n", set_scalar(&tmp, fRight, asType), fRight);
        line.appendf("                 %s  /* %f */);", set_scalar(&tmp, fBottom, asType), fBottom);
    } else {
        SkString strL, strT, strR, strB;
        SkAppendScalarDec(&strL, fLeft);
        SkAppendScalarDec(&strT, fTop);
        SkAppendScalarDec(&strR, fRight);
        SkAppendScalarDec(&strB, fBottom);
        line.printf("SkRect::MakeLTRB(%s, %s, %s, %s);",
                    strL.c_str(), strT.c_str(), strR.c_str(), strB.c_str());
    }
    SkDebugf("%s\n", line.c_str());
}
