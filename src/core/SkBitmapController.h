/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapController_DEFINED
#define SkBitmapController_DEFINED

#include "SkBitmap.h"
#include "SkFilterQuality.h"
#include "SkMatrix.h"
#include "SkPixmap.h"

class SkBitmapProvider;
class SkMipMap;

/**
 *  Handles request to scale, filter, and lock a bitmap to be rasterized.
 */
class SkBitmapController {
public:
    class State : SkNoncopyable {
    public:
        State(const SkBitmapProvider&, const SkMatrix& inv, SkFilterQuality);

        const SkPixmap& pixmap() const { return fPixmap; }
        const SkMatrix& invMatrix() const { return fInvMatrix; }
        SkFilterQuality quality() const { return fQuality; }

    private:
        SkPixmap        fPixmap;
        SkMatrix        fInvMatrix;
        SkFilterQuality fQuality;

        SkBitmap                fResultBitmap;
        sk_sp<const SkMipMap>   fCurrMip;

        bool processHighRequest(const SkBitmapProvider&);
        bool processMediumRequest(const SkBitmapProvider&);

        typedef SkNoncopyable INHERITED;
    };

    static State* RequestBitmap(const SkBitmapProvider&, const SkMatrix& inverse, SkFilterQuality,
                                void* storage, size_t storageSize);

    static State* RequestBitmap(const SkBitmapProvider&bp, const SkMatrix& inverse,
                                SkFilterQuality quality) {
        return RequestBitmap(bp, inverse, quality, nullptr, 0);
    }

private:
    SkBitmapController() = delete;

    typedef SkNoncopyable INHERITED;
};

#endif
