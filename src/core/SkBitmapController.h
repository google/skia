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
#include "include/core/SkMatrix.h"
#include "src/core/SkBitmapCache.h"
#include "src/core/SkMipMap.h"

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
        sk_sp<const SkMipMap> fCurrMip;

    };

    static State* RequestBitmap(const SkImage_Base*, const SkMatrix& inverse, SkFilterQuality,
                                SkArenaAlloc*);

private:
    SkBitmapController() = delete;
};

#endif
