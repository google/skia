
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "GrClip.h"

GrClip::GrClip() {
    fConservativeBounds.setEmpty();
    fConservativeBoundsValid = true;
}

GrClip::GrClip(const GrClip& src) {
    *this = src;
}

GrClip::GrClip(const GrIRect& rect) {
    this->setFromIRect(rect);
}

GrClip::GrClip(const GrRect& rect) {
    this->setFromRect(rect);
}

GrClip::GrClip(GrClipIterator* iter, GrScalar tx, GrScalar ty,
               const GrRect* bounds) {
    this->setFromIterator(iter, tx, ty, bounds);
}

GrClip::~GrClip() {}

GrClip& GrClip::operator=(const GrClip& src) {
    fList = src.fList;
    fConservativeBounds = src.fConservativeBounds;
    fConservativeBoundsValid = src.fConservativeBoundsValid;
    return *this;
}

void GrClip::setEmpty() {
    fList.reset();
    fConservativeBounds.setEmpty();
    fConservativeBoundsValid = true;
}

void GrClip::setFromRect(const GrRect& r) {
    fList.reset();
    if (r.isEmpty()) {
        // use a canonical empty rect for == testing.
        setEmpty();
    } else {
        fList.push_back();
        fList.back().fRect = r;
        fList.back().fType = kRect_ClipType;
        fList.back().fOp = kReplace_SetOp;
        fConservativeBounds = r;
        fConservativeBoundsValid = true;
    }
}

void GrClip::setFromIRect(const GrIRect& r) {
    fList.reset();
    if (r.isEmpty()) {
        // use a canonical empty rect for == testing.
        setEmpty();
    } else {
        fList.push_back();
        fList.back().fRect.set(r);
        fList.back().fType = kRect_ClipType;
        fList.back().fOp = kReplace_SetOp;
        fConservativeBounds.set(r);
        fConservativeBoundsValid = true;
    }
}

static void intersectWith(SkRect* dst, const SkRect& src) {
    if (!dst->intersect(src)) {
        dst->setEmpty();
    }
}

void GrClip::setFromIterator(GrClipIterator* iter, GrScalar tx, GrScalar ty,
                             const GrRect* conservativeBounds) {
    fList.reset();

    int rectCount = 0;

    // compute bounds for common case of series of intersecting rects.
    bool isectRectValid = true;

    if (iter) {
        for (iter->rewind(); !iter->isDone(); iter->next()) {
            Element& e = fList.push_back();
            e.fType = iter->getType();
            e.fOp = iter->getOp();
            // iterators should not emit replace
            GrAssert(kReplace_SetOp != e.fOp);
            switch (e.fType) {
                case kRect_ClipType:
                    iter->getRect(&e.fRect);
                    if (tx || ty) {
                        e.fRect.offset(tx, ty);
                    }
                    ++rectCount;
                    if (isectRectValid) {
                        if (kIntersect_SetOp == e.fOp) {
                            GrAssert(fList.count() <= 2);
                            if (fList.count() > 1) {
                                GrAssert(2 == rectCount);
                                rectCount = 1;
                                fList.pop_back();
                                GrAssert(kRect_ClipType == fList.back().fType);
                                intersectWith(&fList.back().fRect, e.fRect);
                            }
                        } else {
                            isectRectValid = false;
                        }
                    }
                    break;
                case kPath_ClipType:
                    e.fPath = *iter->getPath();
                    if (tx || ty) {
                        e.fPath.offset(tx, ty);
                    }
                    e.fPathFill = iter->getPathFill();
                    isectRectValid = false;
                    break;
                default:
                    GrCrash("Unknown clip element type.");
            }
        }
    }
    fConservativeBoundsValid = false;
    if (isectRectValid && rectCount) {
        fConservativeBounds = fList[0].fRect;
        fConservativeBoundsValid = true;
    } else if (NULL != conservativeBounds) {
        fConservativeBounds = *conservativeBounds;
        fConservativeBoundsValid = true;
    }
}
