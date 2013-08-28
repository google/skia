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

/**
 *  Helper class to manage "tiling" a large coordinate space into managable
 *  chunks, where managable means areas that are <= some max critical coordinate
 *  size.
 *
 *  The constructor takes an antialiasing bool, which affects what this maximum
 *  allowable size is: If we're drawing BW, then we need coordinates to stay
 *  safely within fixed-point range (we use +- 16K, to give ourselves room to
 *  add/subtract two fixed values and still be in range. If we're drawing AA,
 *  then we reduce that size by the amount that the supersampler scan converter
 *  needs (at the moment, that is 4X, so the "safe" range is +- 4K).
 *
 *  For performance reasons, the class first checks to see if any help is needed
 *  at all, and if not (i.e. the specified bounds and base bitmap area already
 *  in the safe-zone, then the class does nothing (effectively).
 */
class SkDeviceLooper {
public:
    SkDeviceLooper(const SkBitmap& base, const SkRasterClip&,
                   const SkIRect& bounds, bool aa);
    ~SkDeviceLooper();

    const SkBitmap& getBitmap() const {
        SkASSERT(kDone_State != fState);
        SkASSERT(fCurrBitmap);
        return *fCurrBitmap;
    }

    const SkRasterClip& getRC() const {
        SkASSERT(kDone_State != fState);
        SkASSERT(fCurrRC);
        return *fCurrRC;
    }

    void mapRect(SkRect* dst, const SkRect& src) const;
    void mapMatrix(SkMatrix* dst, const SkMatrix& src) const;

    /**
     *  Call next to setup the looper to return a valid coordinate chunk.
     *  Each time this returns true, it is safe to call mapRect() and 
     *  mapMatrix(), to convert from "global" coordinate values to ones that
     *  are local to this chunk.
     *
     *  When next() returns false, the list of chunks is done, and mapRect()
     *  and mapMatrix() should no longer be called.
     */
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
