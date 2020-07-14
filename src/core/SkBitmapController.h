/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapController_DEFINED
#define SkBitmapController_DEFINED

#include "include/core/SkBitmap.h"
#include "include/core/SkFilterQuality.h"
#include "include/core/SkImage.h"
#include "include/core/SkMatrix.h"
#include "src/core/SkBitmapCache.h"
#include "src/core/SkMipmap.h"

class SkImage_Base;

/**
 *  Handles request to scale, filter, and lock a bitmap to be rasterized.
 */
class SkBitmapController : ::SkNoncopyable {
public:
    class State : ::SkNoncopyable {
    public:
        State(const SkImage_Base*, const SkMatrix& inv, SkFilterQuality);

        const SkPixmap& pixmap() const { return fPixmap; }
        const SkMatrix& invMatrix() const { return fInvMatrix; }
        SkFilterQuality quality() const { return fQuality; }

    private:
        bool processHighRequest(const SkImage_Base*);
        bool processMediumRequest(const SkImage_Base*);

        SkPixmap              fPixmap;
        SkMatrix              fInvMatrix;
        SkFilterQuality       fQuality;

        // Pixmap storage.
        SkBitmap              fResultBitmap;
        sk_sp<const SkMipmap> fCurrMip;

    };

    static State* RequestBitmap(const SkImage_Base*, const SkMatrix& inverse, SkFilterQuality,
                                SkArenaAlloc*);

private:
    SkBitmapController() = delete;
};

class SkMipmapAccessor : ::SkNoncopyable {
public:
    SkMipmapAccessor(const SkImage_Base*, const SkMatrix& inv, SkMipmapMode requestedMode);

    const SkPixmap& level() const { return fUpper; }
    // only valid if mode() == kLinear
    const SkPixmap& lowerLevel() const { return fLower; }
    // 0....1. Will be 1.0 if there is no lowerLevel
    float           lowerWeight() const { return fLowerWeight; }
    SkMipmapMode    mode() const { return fResolvedMode; }

private:
    SkPixmap     fUpper,
                 fLower; // only valid for mip_linear
    float        fLowerWeight;   // lower * weight + upper * (1 - weight)
    SkMipmapMode fResolvedMode;

    // these manage lifetime for the buffers
    SkBitmap              fBaseStorage;
    sk_sp<const SkMipmap> fCurrMip;
};

#endif
