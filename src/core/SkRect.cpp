/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRect.h"

#include "SkMalloc.h"

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

/////////////////////////////////////////////////////////////////////////////

void SkRect::toQuad(SkPoint quad[4]) const {
    SkASSERT(quad);

    quad[0].set(fLeft, fTop);
    quad[1].set(fRight, fTop);
    quad[2].set(fRight, fBottom);
    quad[3].set(fLeft, fBottom);
}

#include "SkNx.h"

static inline bool is_finite(const Sk4s& value) {
    auto finite = value * Sk4s(0) == Sk4s(0);
    return finite.allTrue();
}

bool SkRect::setBoundsCheck(const SkPoint pts[], int count) {
    SkASSERT((pts && count > 0) || count == 0);

    bool isFinite = true;

    if (count <= 0) {
        sk_bzero(this, sizeof(SkRect));
    } else {
        Sk4s min, max, accum;

        if (count & 1) {
            min = Sk4s(pts[0].fX, pts[0].fY, pts[0].fX, pts[0].fY);
            pts += 1;
            count -= 1;
        } else {
            min = Sk4s::Load(pts);
            pts += 2;
            count -= 2;
        }
        accum = max = min;
        accum = accum * Sk4s(0);

        count >>= 1;
        for (int i = 0; i < count; ++i) {
            Sk4s xy = Sk4s::Load(pts);
            accum = accum * xy;
            min = Sk4s::Min(min, xy);
            max = Sk4s::Max(max, xy);
            pts += 2;
        }

        /**
         *  With some trickery, we may be able to use Min/Max to also propogate non-finites,
         *  in which case we could eliminate accum entirely, and just check min and max for
         *  "is_finite".
         */
        if (is_finite(accum)) {
            float minArray[4], maxArray[4];
            min.store(minArray);
            max.store(maxArray);
            this->set(SkTMin(minArray[0], minArray[2]), SkTMin(minArray[1], minArray[3]),
                      SkTMax(maxArray[0], maxArray[2]), SkTMax(maxArray[1], maxArray[3]));
        } else {
            // we hit a non-finite value, so zero everything and return false
            this->setEmpty();
            isFinite = false;
        }
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
