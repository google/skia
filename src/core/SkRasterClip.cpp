/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRasterClip.h"


SkRasterClip::SkRasterClip() {
    fIsBW = true;
}

bool SkRasterClip::isEmpty() const {
    return fIsBW ? fBW.isEmpty() : fAA.isEmpty();
}

bool SkRasterClip::isRect() const {
    return fIsBW ? fBW.isRect() : false;
}

bool SkRasterClip::isComplex() const {
    return fIsBW ? fBW.isComplex() : !fAA.isEmpty();
}

const SkIRect& SkRasterClip::getBounds() const {
    return fIsBW ? fBW.getBounds() : fAA.getBounds();
}

bool SkRasterClip::setEmpty() {
    fIsBW = true;
    fBW.setEmpty();
    fAA.setEmpty();
    return false;
}

bool SkRasterClip::setIRect(const SkIRect& rect) {
    fIsBW = true;
    fAA.setEmpty();
    return fBW.setRect(rect);
}

bool SkRasterClip::setPath(const SkPath& path, const SkRegion& clip, bool doAA) {
    if (this->isBW() && !doAA) {
        return fBW.setPath(path, clip);
    } else {
        if (this->isBW()) {
            this->convertToAA();
        }
        return fAA.setPath(path, &clip, doAA);
    }
}

bool SkRasterClip::setPath(const SkPath& path, const SkIRect& clip, bool doAA) {
    SkRegion tmp;
    tmp.setRect(clip);
    return this->setPath(path, tmp, doAA);
}

bool SkRasterClip::setPath(const SkPath& path, const SkRasterClip& clip,
                           bool doAA) {
    if (clip.isBW()) {
        return this->setPath(path, clip.bwRgn(), doAA);
    } else {
        SkRegion tmp;
        tmp.setRect(clip.getBounds());
        if (!this->setPath(path, clip, doAA)) {
            return false;
        }
        return this->op(clip, SkRegion::kIntersect_Op);
    }
}

bool SkRasterClip::op(const SkIRect& rect, SkRegion::Op op) {
    return fIsBW ? fBW.op(rect, op) : fAA.op(rect, op);
}

bool SkRasterClip::op(const SkRasterClip& clip, SkRegion::Op op) {
    if (this->isBW() && clip.isBW()) {
        return fBW.op(clip.fBW, op);
    } else {
        SkAAClip tmp;
        const SkAAClip* other;

        if (this->isBW()) {
            this->convertToAA();
        }
        if (clip.isBW()) {
            tmp.setRegion(clip.bwRgn());
            other = &tmp;
        } else {
            other = &clip.aaRgn();
        }
        return fAA.op(*other, op);
    }
}

void SkRasterClip::convertToAA() {
    SkASSERT(fIsBW);
    fAA.setRegion(fBW);
    fIsBW = false;
}

