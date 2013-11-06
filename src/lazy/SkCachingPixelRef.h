/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCachingPixelRef_DEFINED
#define SkCachingPixelRef_DEFINED

#include "SkImage.h"
#include "SkPixelRef.h"

class SkColorTable;

/**
 *  PixelRef which defers decoding until SkBitmap::lockPixels() is
 *  called.  Caches the decoded images in the global
 *  SkScaledImageCache.  When the pixels are unlocked, this cache may
 *  or be destroyed before the next lock.  If so, onLockPixels will
 *  attempt to re-decode.
 *
 *  Decoding is handled by the pure-virtual functions onDecodeInfo()
 *  and onDecodePixels().  Subclasses of this class need only provide
 *  those two functions.
 */
class SkCachingPixelRef : public SkPixelRef {
public:
    SkCachingPixelRef();
    virtual ~SkCachingPixelRef();

protected:
    virtual void* onLockPixels(SkColorTable** colorTable) SK_OVERRIDE;
    virtual void onUnlockPixels() SK_OVERRIDE;
    virtual bool onLockPixelsAreWritable() const SK_OVERRIDE { return false; }
    virtual bool onImplementsDecodeInto() SK_OVERRIDE  { return true; }
    virtual bool onDecodeInto(int pow2, SkBitmap*) SK_OVERRIDE;

    /**
     *  Configure the supplied bitmap for this pixelRef, based on
     *  information provided by onDecodeInfo().  Does not set the
     *  bitmap's pixelRef. */
    bool configure(SkBitmap* bitmap);

    /**
     *  Cache info from onDecodeInfo(). Returns false on failure.
     */
    bool getInfo(SkImageInfo* info);

    /**
     *  Return some information about the pixels, allowing this class
     *  to allocate pixels.  @return false if anything goes wrong.
     */
    virtual bool onDecodeInfo(SkImageInfo* info) = 0;
    /**
     *  Decode into the given pixels, a block of memory of size
     *  (info.fHeight - 1) * rowBytes + (info.fWidth * bytesPerPixel)
     *
     *  @param info Should be identical to the info returned by
     *         onDecodeInfo so that the implementation can confirm
     *         that the caller knows what it is asking for (config,
     *         size).  Thiscontract also allows the caller to specify
     *         different output-configs, which the implementation can
     *         decide to support or not.
     *
     *  @return false if anything goes wrong.
     */
    virtual bool onDecodePixels(const SkImageInfo& info,
                                void* pixels,
                                size_t rowBytes) = 0;

private:
    bool                        fErrorInDecoding;
    void*                       fScaledCacheId;
    SkImageInfo                 fInfo;

    typedef SkPixelRef INHERITED;
};

#endif  // SkCachingPixelRef_DEFINED

