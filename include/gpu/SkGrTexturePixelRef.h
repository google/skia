
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
    SkROLockPixelsPixelRef();
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
    SkGrPixelRef(GrSurface* surface);
    virtual ~SkGrPixelRef();

    // override from SkPixelRef
    virtual SkGpuTexture* getTexture() SK_OVERRIDE;

    SK_DECLARE_UNFLATTENABLE_OBJECT()

protected:
    // overrides from SkPixelRef
    virtual bool onReadPixels(SkBitmap* dst, const SkIRect* subset) SK_OVERRIDE;
    virtual SkPixelRef* deepCopy(SkBitmap::Config dstConfig) SK_OVERRIDE;

private:
    GrSurface*  fSurface;
    typedef SkROLockPixelsPixelRef INHERITED;
};

#endif

