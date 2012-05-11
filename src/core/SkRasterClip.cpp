/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRasterClip.h"


SkRasterClip::SkRasterClip() {
    fIsBW = true;
    fIsEmpty = true;
    fIsRect = false;
    SkDEBUGCODE(this->validate();)
}

SkRasterClip::SkRasterClip(const SkRasterClip& src) {
    AUTO_RASTERCLIP_VALIDATE(src);
    
    fIsBW = src.fIsBW;
    if (fIsBW) {
        fBW = src.fBW;
    } else {
        fAA = src.fAA;
    }

    fIsEmpty = src.isEmpty();
    fIsRect = src.isRect();
    SkDEBUGCODE(this->validate();)
}

SkRasterClip::SkRasterClip(const SkIRect& bounds) : fBW(bounds) {
    fIsBW = true;
    fIsEmpty = this->computeIsEmpty();  // bounds might be empty, so compute
    fIsRect = !fIsEmpty;
    SkDEBUGCODE(this->validate();)
}

SkRasterClip::~SkRasterClip() {
    SkDEBUGCODE(this->validate();)
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
    fIsEmpty = true;
    fIsRect = false;
    return false;
}

bool SkRasterClip::setRect(const SkIRect& rect) {
    AUTO_RASTERCLIP_VALIDATE(*this);
    
    fIsBW = true;
    fAA.setEmpty();
    fIsRect = fBW.setRect(rect);
    fIsEmpty = !fIsRect;
    return fIsRect;
}

bool SkRasterClip::setPath(const SkPath& path, const SkRegion& clip, bool doAA) {
    AUTO_RASTERCLIP_VALIDATE(*this);

    if (this->isBW() && !doAA) {
        (void)fBW.setPath(path, clip);
    } else {
        // TODO: since we are going to over-write fAA completely (aren't we?)
        // we should just clear our BW data (if any) and set fIsAA=true
        if (this->isBW()) {
            this->convertToAA();
        }
        (void)fAA.setPath(path, &clip, doAA);
    }
    return this->updateCacheAndReturnNonEmpty();
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
    
    fIsBW ? fBW.op(rect, op) : fAA.op(rect, op);
    return this->updateCacheAndReturnNonEmpty();
}

bool SkRasterClip::op(const SkRegion& rgn, SkRegion::Op op) {
    AUTO_RASTERCLIP_VALIDATE(*this);
    
    if (fIsBW) {
        (void)fBW.op(rgn, op);
    } else {
        SkAAClip tmp;
        tmp.setRegion(rgn);
        (void)fAA.op(tmp, op);
    }
    return this->updateCacheAndReturnNonEmpty();
}

bool SkRasterClip::op(const SkRasterClip& clip, SkRegion::Op op) {
    AUTO_RASTERCLIP_VALIDATE(*this);
    clip.validate();

    if (this->isBW() && clip.isBW()) {
        (void)fBW.op(clip.fBW, op);
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
        (void)fAA.op(*other, op);
    }
    return this->updateCacheAndReturnNonEmpty();
}

/**
 *  Our antialiasing currently has a granularity of 1/4 of a pixel along each
 *  axis. Thus we can treat an axis coordinate as an integer if it differs
 *  from its nearest int by < half of that value (1.8 in this case).
 */
static bool nearly_integral(SkScalar x) {
    static const SkScalar domain = SK_Scalar1 / 4;
    static const SkScalar halfDomain = domain / 2;

    x += halfDomain;
    return x - SkScalarFloorToScalar(x) < domain;
}

bool SkRasterClip::op(const SkRect& r, SkRegion::Op op, bool doAA) {
    AUTO_RASTERCLIP_VALIDATE(*this);
    
    if (fIsBW && doAA) {
        // check that the rect really needs aa, or is it close enought to
        // integer boundaries that we can just treat it as a BW rect?
        if (nearly_integral(r.fLeft) && nearly_integral(r.fTop) &&
            nearly_integral(r.fRight) && nearly_integral(r.fBottom)) {
            doAA = false;
        }
    }

    if (fIsBW && !doAA) {
        SkIRect ir;
        r.round(&ir);
        (void)fBW.op(ir, op);
    } else {
        if (fIsBW) {
            this->convertToAA();
        }
        (void)fAA.op(r, op, doAA);
    }
    return this->updateCacheAndReturnNonEmpty();
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
    dst->updateCacheAndReturnNonEmpty();
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
    (void)this->updateCacheAndReturnNonEmpty();
}

#ifdef SK_DEBUG
void SkRasterClip::validate() const {
    // can't ever assert that fBW is empty, since we may have called forceGetBW
    if (fIsBW) {
        SkASSERT(fAA.isEmpty());
    }

    fBW.validate();
    fAA.validate();

    SkASSERT(this->computeIsEmpty() == fIsEmpty);
    SkASSERT(this->computeIsRect() == fIsRect);
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

