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
    // sentinels that next() has not yet been called, and so our mapper functions
    // should not be called either.
    fCurrBitmap = NULL;
    fCurrRC = NULL;

    SkIRect bitmapBounds = SkIRect::MakeWH(base.width(), base.height());
    if (!fClippedBounds.intersect(bounds, bitmapBounds)) {
        fState = kDone_State;
    } else if (this->fitsInDelta(bounds)) {
        fState = kSimple_State;
    } else {
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
    SkASSERT(fCurrBitmap);
    SkASSERT(fCurrRC);

    *dst = src;
    dst->offset(SkIntToScalar(-fCurrOffset.fX),
                SkIntToScalar(-fCurrOffset.fY));
}

void SkDeviceLooper::mapMatrix(SkMatrix* dst, const SkMatrix& src) const {
    SkASSERT(kDone_State != fState);
    SkASSERT(fCurrBitmap);
    SkASSERT(fCurrRC);

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
    
    fCurrBitmap = &fSubsetBitmap;
    fCurrRC = &fSubsetRC;
    return true;
}

bool SkDeviceLooper::next() {
    switch (fState) {
        case kDone_State:
            // in theory, we should not get called here, since we must have
            // previously returned false, but we check anyway.
            break;
    
        case kSimple_State:
            // first time for simple
            if (NULL == fCurrBitmap) {
                fCurrBitmap = &fBaseBitmap;
                fCurrRC = &fBaseRC;
                fCurrOffset.set(0, 0);
                return true;
            }
            // 2nd time for simple, we are done
            break;

        case kComplex_State:
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
            break;
    }
    
    fState = kDone_State;
    return false;
}
