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

SkRasterClip::SkRasterClip(const SkRasterClip& src) {
    AUTO_RASTERCLIP_VALIDATE(src);
    
    fIsBW = src.fIsBW;
    if (fIsBW) {
        fBW = src.fBW;
    } else {
        fAA = src.fAA;
    }
}

SkRasterClip::SkRasterClip(const SkIRect& bounds) : fBW(bounds) {
    fIsBW = true;
}

SkRasterClip::~SkRasterClip() {
    AUTO_RASTERCLIP_VALIDATE(*this);
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
    AUTO_RASTERCLIP_VALIDATE(*this);

    fIsBW = true;
    fBW.setEmpty();
    fAA.setEmpty();
    return false;
}

bool SkRasterClip::setRect(const SkIRect& rect) {
    AUTO_RASTERCLIP_VALIDATE(*this);
    
    fIsBW = true;
    fAA.setEmpty();
    return fBW.setRect(rect);
}

bool SkRasterClip::setPath(const SkPath& path, const SkRegion& clip, bool doAA) {
    AUTO_RASTERCLIP_VALIDATE(*this);

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
    AUTO_RASTERCLIP_VALIDATE(*this);
    
    return fIsBW ? fBW.op(rect, op) : fAA.op(rect, op);
}

bool SkRasterClip::op(const SkRegion& rgn, SkRegion::Op op) {
    AUTO_RASTERCLIP_VALIDATE(*this);
    
    if (fIsBW) {
        return fBW.op(rgn, op);
    } else {
        SkAAClip tmp;
        tmp.setRegion(rgn);
        return fAA.op(tmp, op);
    }
}

bool SkRasterClip::op(const SkRasterClip& clip, SkRegion::Op op) {
    AUTO_RASTERCLIP_VALIDATE(*this);
    clip.validate();

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

// return true if x is nearly integral (within 1/16) since that is the highest
// precision our aa code can have.
static bool is_integral(SkScalar x) {
    int ix = SkScalarRoundToInt(x);
    SkScalar sx = SkIntToScalar(ix);
    return SkScalarAbs(sx - x) < (SK_Scalar1 / 16);
}

bool SkRasterClip::op(const SkRect& r, SkRegion::Op op, bool doAA) {
    AUTO_RASTERCLIP_VALIDATE(*this);
    
    if (doAA) {
        // check that the rect really needs aa
        if (is_integral(r.fLeft) && is_integral(r.fTop) &&
            is_integral(r.fRight) && is_integral(r.fBottom)) {
            doAA = false;
        }
    }

    if (fIsBW && !doAA) {
        SkIRect ir;
        r.round(&ir);
        return fBW.op(ir, op);
    } else {
        if (fIsBW) {
            this->convertToAA();
        }
        return fAA.op(r, op, doAA);
    }
}

void SkRasterClip::translate(int dx, int dy, SkRasterClip* dst) const {
    if (NULL == dst) {
        return;
    }

    AUTO_RASTERCLIP_VALIDATE(*this);
    
    if (this->isEmpty()) {
        dst->setEmpty();
        return;
    }
    if (0 == (dx | dy)) {
        *dst = *this;
        return;
    }

    dst->fIsBW = fIsBW;
    if (fIsBW) {
        fBW.translate(dx, dy, &dst->fBW);
        dst->fAA.setEmpty();
    } else {
        fAA.translate(dx, dy, &dst->fAA);
        dst->fBW.setEmpty();
    }
}

bool SkRasterClip::quickContains(const SkIRect& ir) const {
    return fIsBW ? fBW.quickContains(ir) : fAA.quickContains(ir);
}

///////////////////////////////////////////////////////////////////////////////

const SkRegion& SkRasterClip::forceGetBW() {
    AUTO_RASTERCLIP_VALIDATE(*this);
    
    if (!fIsBW) {
        fBW.setRect(fAA.getBounds());
    }
    return fBW;
}

void SkRasterClip::convertToAA() {
    AUTO_RASTERCLIP_VALIDATE(*this);
    
    SkASSERT(fIsBW);
    fAA.setRegion(fBW);
    fIsBW = false;
}

#ifdef SK_DEBUG
void SkRasterClip::validate() const {
    // can't ever assert that fBW is empty, since we may have called forceGetBW
    if (fIsBW) {
        SkASSERT(fAA.isEmpty());
    }

    fBW.validate();
    fAA.validate();
}
#endif

///////////////////////////////////////////////////////////////////////////////

SkAAClipBlitterWrapper::SkAAClipBlitterWrapper() {
    SkDEBUGCODE(fClipRgn = NULL;)
    SkDEBUGCODE(fBlitter = NULL;)
}

SkAAClipBlitterWrapper::SkAAClipBlitterWrapper(const SkRasterClip& clip,
                                               SkBlitter* blitter) {
    this->init(clip, blitter);
}

SkAAClipBlitterWrapper::SkAAClipBlitterWrapper(const SkAAClip* aaclip,
                                               SkBlitter* blitter) {
    SkASSERT(blitter);
    SkASSERT(aaclip);
    fBWRgn.setRect(aaclip->getBounds());
    fAABlitter.init(blitter, aaclip);
    // now our return values
    fClipRgn = &fBWRgn;
    fBlitter = &fAABlitter;
}

void SkAAClipBlitterWrapper::init(const SkRasterClip& clip, SkBlitter* blitter) {
    SkASSERT(blitter);
    if (clip.isBW()) {
        fClipRgn = &clip.bwRgn();
        fBlitter = blitter;
    } else {
        const SkAAClip& aaclip = clip.aaRgn();
        fBWRgn.setRect(aaclip.getBounds());
        fAABlitter.init(blitter, &aaclip);
        // now our return values
        fClipRgn = &fBWRgn;
        fBlitter = &fAABlitter;
    }
}

