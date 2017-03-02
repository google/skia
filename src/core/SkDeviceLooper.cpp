/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDeviceLooper.h"

SkDeviceLooper::SkDeviceLooper(const SkPixmap& base, const SkRasterClip& rc, const SkIRect& bounds,
                               bool aa)
    : fBaseDst(base)
    , fBaseRC(rc)
    , fDelta(aa ? kAA_Delta : kBW_Delta)
{
    // sentinels that next() has not yet been called, and so our mapper functions
    // should not be called either.
    fCurrDst = nullptr;
    fCurrRC = nullptr;

    if (!rc.isEmpty()) {
        // clip must be contained by the bitmap
        SkASSERT(SkIRect::MakeWH(base.width(), base.height()).contains(rc.getBounds()));
    }

    if (rc.isEmpty() || !fClippedBounds.intersect(bounds, rc.getBounds())) {
        fState = kDone_State;
    } else if (this->fitsInDelta(fClippedBounds)) {
        fState = kSimple_State;
    } else {
        // back up by 1 DX, so that next() will put us in a correct starting
        // position.
        fCurrOffset.set(fClippedBounds.left() - fDelta,
                        fClippedBounds.top());
        fState = kComplex_State;
    }
}

SkDeviceLooper::~SkDeviceLooper() {}

void SkDeviceLooper::mapRect(SkRect* dst, const SkRect& src) const {
    SkASSERT(kDone_State != fState);
    SkASSERT(fCurrDst);
    SkASSERT(fCurrRC);

    *dst = src;
    dst->offset(SkIntToScalar(-fCurrOffset.fX),
                SkIntToScalar(-fCurrOffset.fY));
}

void SkDeviceLooper::mapMatrix(SkMatrix* dst, const SkMatrix& src) const {
    SkASSERT(kDone_State != fState);
    SkASSERT(fCurrDst);
    SkASSERT(fCurrRC);

    *dst = src;
    dst->postTranslate(SkIntToScalar(-fCurrOffset.fX), SkIntToScalar(-fCurrOffset.fY));
}

bool SkDeviceLooper::computeCurrBitmapAndClip() {
    SkASSERT(kComplex_State == fState);

    SkIRect r = SkIRect::MakeXYWH(fCurrOffset.x(), fCurrOffset.y(),
                                  fDelta, fDelta);
    if (!fBaseDst.extractSubset(&fSubsetDst, r)) {
        fSubsetRC.setEmpty();
    } else {
        fBaseRC.translate(-r.left(), -r.top(), &fSubsetRC);
        (void)fSubsetRC.op(SkIRect::MakeWH(fDelta, fDelta), SkRegion::kIntersect_Op);
    }

    fCurrDst = &fSubsetDst;
    fCurrRC = &fSubsetRC;
    return !fCurrRC->isEmpty();
}

static bool next_tile(const SkIRect& boundary, int delta, SkIPoint* offset) {
    // can we move to the right?
    if (offset->x() + delta < boundary.right()) {
        offset->fX += delta;
        return true;
    }

    // reset to the left, but move down a row
    offset->fX = boundary.left();
    if (offset->y() + delta < boundary.bottom()) {
        offset->fY += delta;
        return true;
    }

    // offset is now outside of boundary, so we're done
    return false;
}

bool SkDeviceLooper::next() {
    switch (fState) {
        case kDone_State:
            // in theory, we should not get called here, since we must have
            // previously returned false, but we check anyway.
            break;

        case kSimple_State:
            // first time for simple
            if (nullptr == fCurrDst) {
                fCurrDst = &fBaseDst;
                fCurrRC = &fBaseRC;
                fCurrOffset.set(0, 0);
                return true;
            }
            // 2nd time for simple, we are done
            break;

        case kComplex_State:
            // need to propogate fCurrOffset through clippedbounds
            // left to right, until we wrap around and move down

            while (next_tile(fClippedBounds, fDelta, &fCurrOffset)) {
                if (this->computeCurrBitmapAndClip()) {
                    return true;
                }
            }
            break;
    }
    fState = kDone_State;
    return false;
}
