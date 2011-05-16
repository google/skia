/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#include "GrClip.h"

GrClip::GrClip()
    : fList(&fListStorage) {
    fConservativeBounds.setEmpty();
    fConservativeBoundsValid = true;
}

GrClip::GrClip(const GrClip& src)
    : fList(&fListStorage) {
    *this = src;
}

GrClip::GrClip(const GrIRect& rect)
    : fList(&fListStorage) {
    this->setFromIRect(rect);
}

GrClip::GrClip(const GrRect& rect)
    : fList(&fListStorage) {
    this->setFromRect(rect);
}

GrClip::GrClip(GrClipIterator* iter, GrScalar tx, GrScalar ty,
               const GrRect* bounds)
    : fList(&fListStorage) {
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
                        if (1 == rectCount || kIntersect_SetOp == e.fOp) {
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
    if (isectRectValid) {
        fConservativeBoundsValid = true;
        if (rectCount > 0) {
            fConservativeBounds = fList[0].fRect;
        } else {
            fConservativeBounds.setEmpty();
        }
    } else if (NULL != conservativeBounds) {
        fConservativeBounds = *conservativeBounds;
        fConservativeBoundsValid = true;
    }
}
