/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRect.h"

#include "include/private/SkMalloc.h"

bool SkIRect::intersect(const SkIRect& r) {
    SkIRect tmp = {
        SkMax32(fLeft,   r.fLeft),
        SkMax32(fTop,    r.fTop),
        SkMin32(fRight,  r.fRight),
        SkMin32(fBottom, r.fBottom)
    };
    if (tmp.isEmpty()) {
        return false;
    }
    *this = tmp;
    return true;
}

void SkIRect::join(const SkIRect& r) {
    // do nothing if the params are empty
    if (r.fLeft >= r.fRight || r.fTop >= r.fBottom) {
        return;
    }

    // if we are empty, just assign
    if (fLeft >= fRight || fTop >= fBottom) {
        *this = r;
    } else {
        if (r.fLeft < fLeft)     fLeft = r.fLeft;
        if (r.fTop < fTop)       fTop = r.fTop;
        if (r.fRight > fRight)   fRight = r.fRight;
        if (r.fBottom > fBottom) fBottom = r.fBottom;
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

#include "include/private/SkNx.h"

bool SkRect::setBoundsCheck(const SkPoint pts[], int count) {
    SkASSERT((pts && count > 0) || count == 0);

    if (count <= 0) {
        this->setEmpty();
        return true;
    }

    Sk4s min, max;
    if (count & 1) {
        min = max = Sk4s(pts->fX, pts->fY,
                         pts->fX, pts->fY);
        pts   += 1;
        count -= 1;
    } else {
        min = max = Sk4s::Load(pts);
        pts   += 2;
        count -= 2;
    }

    Sk4s accum = min * 0;
    while (count) {
        Sk4s xy = Sk4s::Load(pts);
        accum = accum * xy;
        min = Sk4s::Min(min, xy);
        max = Sk4s::Max(max, xy);
        pts   += 2;
        count -= 2;
    }

    bool all_finite = (accum * 0 == 0).allTrue();
    if (all_finite) {
        this->setLTRB(SkTMin(min[0], min[2]), SkTMin(min[1], min[3]),
                      SkTMax(max[0], max[2]), SkTMax(max[1], max[3]));
    } else {
        this->setEmpty();
    }
    return all_finite;
}

void SkRect::setBoundsNoCheck(const SkPoint pts[], int count) {
    if (!this->setBoundsCheck(pts, count)) {
        this->setLTRB(SK_ScalarNaN, SK_ScalarNaN, SK_ScalarNaN, SK_ScalarNaN);
    }
}

#define CHECK_INTERSECT(al, at, ar, ab, bl, bt, br, bb) \
    SkScalar L = SkMaxScalar(al, bl);                   \
    SkScalar R = SkMinScalar(ar, br);                   \
    SkScalar T = SkMaxScalar(at, bt);                   \
    SkScalar B = SkMinScalar(ab, bb);                   \
    do { if (!(L < R && T < B)) return false; } while (0)
    // do the !(opposite) check so we return false if either arg is NaN

bool SkRect::intersect(const SkRect& r) {
    CHECK_INTERSECT(r.fLeft, r.fTop, r.fRight, r.fBottom, fLeft, fTop, fRight, fBottom);
    this->setLTRB(L, T, R, B);
    return true;
}

bool SkRect::intersect(const SkRect& a, const SkRect& b) {
    CHECK_INTERSECT(a.fLeft, a.fTop, a.fRight, a.fBottom, b.fLeft, b.fTop, b.fRight, b.fBottom);
    this->setLTRB(L, T, R, B);
    return true;
}

void SkRect::join(const SkRect& r) {
    if (r.isEmpty()) {
        return;
    }

    if (this->isEmpty()) {
        *this = r;
    } else {
        fLeft   = SkMinScalar(fLeft, r.fLeft);
        fTop    = SkMinScalar(fTop, r.fTop);
        fRight  = SkMaxScalar(fRight, r.fRight);
        fBottom = SkMaxScalar(fBottom, r.fBottom);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////

#include "include/core/SkString.h"
#include "src/core/SkStringUtils.h"

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
