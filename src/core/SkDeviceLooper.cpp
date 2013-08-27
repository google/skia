/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDeviceLooper.h"

SkDeviceLooper::SkDeviceLooper(const SkBitmap& base,
                               const SkRasterClip& rc,
                               const SkIRect& bounds, bool aa)
: fBaseBitmap(base)
, fBaseRC(rc)
, fDelta(aa ? kAA_Delta : kBW_Delta)
{
    SkIRect bitmapBounds = SkIRect::MakeWH(base.width(), base.height());
    if (!fClippedBounds.intersect(bounds, bitmapBounds)) {
        fState = kDone_State;
    } else if (this->fitsInDelta(bounds)) {
        fCurrBitmap = &fBaseBitmap;
        fCurrRC = &fBaseRC;
        fState = kSimple_State;
    } else {
        fCurrBitmap = &fSubsetBitmap;
        fCurrRC = &fSubsetRC;
        // back up by 1 DX, so that next() will put us in a correct starting
        // position.
        fCurrOffset.set(fClippedBounds.left() - fDelta,
                        fClippedBounds.top());
        fState = kComplex_State;
    }
}

SkDeviceLooper::~SkDeviceLooper() {
}

void SkDeviceLooper::mapRect(SkRect* dst, const SkRect& src) const {
    SkASSERT(kDone_State != fState);
    *dst = src;
    dst->offset(SkIntToScalar(-fCurrOffset.fX),
                SkIntToScalar(-fCurrOffset.fY));
}

void SkDeviceLooper::mapMatrix(SkMatrix* dst, const SkMatrix& src) const {
    SkASSERT(kDone_State != fState);
    *dst = src;
    dst->postTranslate(SkIntToScalar(-fCurrOffset.fX),
                       SkIntToScalar(-fCurrOffset.fY));
}

bool SkDeviceLooper::computeCurrBitmapAndClip() {
    SkASSERT(kComplex_State == fState);
    
    SkIRect r = SkIRect::MakeXYWH(fCurrOffset.x(), fCurrOffset.y(),
                                  fDelta, fDelta);
    if (!fBaseBitmap.extractSubset(&fSubsetBitmap, r)) {
        fState = kDone_State;
        return false;
    }
    fSubsetBitmap.lockPixels();
    
    fBaseRC.translate(-r.left(), -r.top(), &fSubsetRC);
    (void)fSubsetRC.op(SkIRect::MakeWH(fDelta, fDelta), SkRegion::kIntersect_Op);
    return true;
}

bool SkDeviceLooper::next() {
    if (kDone_State == fState) {
        return false;
    }
    
    if (kSimple_State == fState) {
        fCurrBitmap = &fBaseBitmap;
        fCurrRC = &fBaseRC;
        fCurrOffset.set(0, 0);
        fState = kDone_State;
        return true;
    }
    
    SkASSERT(kComplex_State == fState);
    
    // need to propogate fCurrOffset through clippedbounds
    // left to right, until we wrap around and move down
    
    if (fCurrOffset.x() + fDelta < fClippedBounds.right()) {
        fCurrOffset.fX += fDelta;
        return this->computeCurrBitmapAndClip();
    }
    fCurrOffset.fX = fClippedBounds.left();
    if (fCurrOffset.y() + fDelta < fClippedBounds.bottom()) {
        fCurrOffset.fY += fDelta;
        return this->computeCurrBitmapAndClip();
    }
    fState = kDone_State;
    return false;
}
