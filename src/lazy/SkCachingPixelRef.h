/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCachingPixelRef_DEFINED
#define SkCachingPixelRef_DEFINED

#include "SkBitmapCache.h"
#include "SkImageInfo.h"
#include "SkImageGenerator.h"
#include "SkPixelRef.h"

class SkColorTable;

/**
 *  PixelRef which defers decoding until SkBitmap::lockPixels() is
 *  called.  Caches the decoded images in the global
 *  SkScaledImageCache.  When the pixels are unlocked, this cache may
 *  or be destroyed before the next lock.  If so, onLockPixels will
 *  attempt to re-decode.
 *
 *  Decoding is handled by the SkImageGenerator
 */
class SkCachingPixelRef : public SkPixelRef {
public:
    
    /**
     *  Takes ownership of SkImageGenerator.  If this method fails for
     *  whatever reason, it will return false and immediatetely delete
     *  the generator.  If it succeeds, it will modify destination
     *  bitmap.
     *
     *  If Install fails or when the SkCachingPixelRef that is
     *  installed into destination is destroyed, it will call
     *  SkDELETE() on the generator.  Therefore, generator should be
     *  allocated with SkNEW() or SkNEW_ARGS().
     */
    static bool Install(SkImageGenerator* gen, SkBitmap* dst);

protected:
    virtual ~SkCachingPixelRef();
    bool onNewLockPixels(LockRec*) override;
    void onUnlockPixels() override;
    bool onLockPixelsAreWritable() const override { return false; }

    SkData* onRefEncodedData() override {
        return fImageGenerator->refEncodedData();
    }

private:
    SkImageGenerator* const fImageGenerator;
    bool                    fErrorInDecoding;
    const size_t            fRowBytes;

    SkBitmap                fLockedBitmap;

    SkCachingPixelRef(const SkImageInfo&, SkImageGenerator*, size_t rowBytes);

    typedef SkPixelRef INHERITED;
};

#endif  // SkCachingPixelRef_DEFINED
