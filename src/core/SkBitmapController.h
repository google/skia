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
#include "SkImage.h"
#include "SkMatrix.h"

class SkBitmapProvider {
public:
    SkBitmapProvider(const SkBitmap& bm) : fBitmap(bm) {}
    SkBitmapProvider(const SkImage* img) : fImage(SkRef(img)) {}

    int width() const;
    int height() const;
    uint32_t getID() const;

    bool validForDrawing() const;
    SkImageInfo info() const;

    SkBitmapCacheDesc makeCacheDesc(int w, int h) const;
    SkBitmapCacheDesc makeCacheDesc() const;
    void notifyAddedToCache() const;

    // Only call this if you're sure you need the bits, since it make be expensive
    // ... cause a decode and cache, or gpu-readback
    bool asBitmap(SkBitmap*) const;

private:
    SkBitmap fBitmap;
    SkAutoTUnref<const SkImage> fImage;
};

/**
 *  Handles request to scale, filter, and lock a bitmap to be rasterized.
 */
class SkBitmapController : ::SkNoncopyable {
public:
    class State : ::SkNoncopyable {
    public:
        virtual ~State() {}

        const SkPixmap& pixmap() const { return fPixmap; }
        const SkMatrix& invMatrix() const { return fInvMatrix; }
        SkFilterQuality quality() const { return fQuality; }
    
    protected:
        SkPixmap        fPixmap;
        SkMatrix        fInvMatrix;
        SkFilterQuality fQuality;
    
    private:
        friend class SkBitmapController;
    };

    virtual ~SkBitmapController() {}

    State* requestBitmap(const SkBitmapProvider&, const SkMatrix& inverse, SkFilterQuality,
                         void* storage, size_t storageSize);

    State* requestBitmap(const SkBitmapProvider& bp, const SkMatrix& inv, SkFilterQuality quality) {
        return this->requestBitmap(bp, inv, quality, nullptr, 0);
    }

protected:
    virtual State* onRequestBitmap(const SkBitmapProvider&, const SkMatrix& inv, SkFilterQuality,
                                   void* storage, size_t storageSize) = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkDefaultBitmapController : public SkBitmapController {
public:
    SkDefaultBitmapController() {}
    
protected:
    State* onRequestBitmap(const SkBitmapProvider&, const SkMatrix& inverse, SkFilterQuality,
                           void* storage, size_t storageSize) override;
};

#endif
