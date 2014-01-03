/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGrPixelRef_DEFINED
#define SkGrPixelRef_DEFINED

#include "SkBitmap.h"
#include "SkPixelRef.h"
#include "GrTexture.h"
#include "GrRenderTarget.h"


/**
 *  Common baseclass that implements onLockPixels() by calling onReadPixels().
 *  Since it has a copy, it always returns false for onLockPixelsAreWritable().
 */
class SK_API SkROLockPixelsPixelRef : public SkPixelRef {
public:
    SkROLockPixelsPixelRef(const SkImageInfo&);
    virtual ~SkROLockPixelsPixelRef();

protected:
    // override from SkPixelRef
    virtual void* onLockPixels(SkColorTable** ptr);
    virtual void onUnlockPixels();
    virtual bool onLockPixelsAreWritable() const;   // return false;

private:
    SkBitmap    fBitmap;
    typedef SkPixelRef INHERITED;
};

/**
 *  PixelRef that wraps a GrSurface
 */
class SK_API SkGrPixelRef : public SkROLockPixelsPixelRef {
public:
    /**
     * Constructs a pixel ref around a GrSurface. If the caller has locked the GrSurface in the
     * cache and would like the pixel ref to unlock it in its destructor then transferCacheLock
     * should be set to true.
     */
    SkGrPixelRef(const SkImageInfo&, GrSurface*, bool transferCacheLock = false);
    virtual ~SkGrPixelRef();

    // override from SkPixelRef
    virtual GrTexture* getTexture() SK_OVERRIDE;

    SK_DECLARE_UNFLATTENABLE_OBJECT()

protected:
    // overrides from SkPixelRef
    virtual bool onReadPixels(SkBitmap* dst, const SkIRect* subset) SK_OVERRIDE;
    virtual SkPixelRef* deepCopy(SkBitmap::Config dstConfig, const SkIRect* subset) SK_OVERRIDE;

private:
    GrSurface*  fSurface;
    bool        fUnlock;   // if true the pixel ref owns a texture cache lock on fSurface

    typedef SkROLockPixelsPixelRef INHERITED;
};

#endif
