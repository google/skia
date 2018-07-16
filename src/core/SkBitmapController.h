/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapController_DEFINED
#define SkBitmapController_DEFINED

#include "SkBitmap.h"
#include "SkBitmapCache.h"
#include "SkFilterQuality.h"
#include "SkMatrix.h"
#include "SkMipMap.h"

class SkBitmapProvider;

/**
 *  Handles request to scale, filter, and lock a bitmap to be rasterized.
 */
class SkBitmapController : ::SkNoncopyable {
public:
    class State : ::SkNoncopyable {
    public:
        State(const SkBitmapProvider&, const SkMatrix& inv, SkFilterQuality);

        const SkPixmap& pixmap() const { return fPixmap; }
        const SkMatrix& invMatrix() const { return fInvMatrix; }
        SkFilterQuality quality() const { return fQuality; }

    private:
        bool processHighRequest(const SkBitmapProvider&);
        bool processMediumRequest(const SkBitmapProvider&);

        SkPixmap              fPixmap;
        SkMatrix              fInvMatrix;
        SkFilterQuality       fQuality;

        // Pixmap storage.
        SkBitmap              fResultBitmap;
        sk_sp<const SkMipMap> fCurrMip;

    };

    static State* RequestBitmap(const SkBitmapProvider&, const SkMatrix& inverse, SkFilterQuality,
                                SkArenaAlloc*);

private:
    SkBitmapController() = delete;
};

#endif
