/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDeviceLooper_DEFINED
#define SkDeviceLooper_DEFINED

#include "SkBitmap.h"
#include "SkMatrix.h"
#include "SkRasterClip.h"

class SkDeviceLooper {
public:
    SkDeviceLooper(const SkBitmap& base, const SkRasterClip&,
                   const SkIRect& bounds, bool aa);
    ~SkDeviceLooper();

    const SkBitmap& getBitmap() const {
        SkASSERT(kDone_State != fState);
        return *fCurrBitmap;
    }

    const SkRasterClip& getRC() const {
        SkASSERT(kDone_State != fState);
        return *fCurrRC;
    }

    void mapRect(SkRect* dst, const SkRect& src) const;
    void mapMatrix(SkMatrix* dst, const SkMatrix& src) const;

    bool next();

private:
    const SkBitmap&     fBaseBitmap;
    const SkRasterClip& fBaseRC;

    enum State {
        kDone_State,    // iteration is complete, getters will assert
        kSimple_State,  // no translate/clip mods needed
        kComplex_State
    };

    // storage for our tiled versions. Perhaps could use SkTLazy
    SkBitmap            fSubsetBitmap;
    SkRasterClip        fSubsetRC;

    const SkBitmap*     fCurrBitmap;
    const SkRasterClip* fCurrRC;
    SkIRect             fClippedBounds;
    SkIPoint            fCurrOffset;
    int                 fDelta;
    State               fState;

    enum Delta {
        kBW_Delta = 1 << 14,        // 16K, gives room to spare for fixedpoint
        kAA_Delta = kBW_Delta >> 2  // supersample 4x
    };

    bool fitsInDelta(const SkIRect& r) const {
        return r.right() < fDelta && r.bottom() < fDelta;
    }
    
    bool computeCurrBitmapAndClip();
};

#endif
