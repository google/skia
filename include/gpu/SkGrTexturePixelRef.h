
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef SkGrTexturePixelRef_DEFINED
#define SkGrTexturePixelRef_DEFINED

#include "SkBitmap.h"
#include "SkPixelRef.h"
#include "GrTexture.h"
#include "GrRenderTarget.h"


/**
 *  Common baseclass that implements onLockPixels() by calling onReadPixels().
 *  Since it has a copy, it always returns false for onLockPixelsAreWritable().
 */
class SK_API SkROLockPixelsPixelRef_Deprecated : public SkPixelRef {
public:
    SkROLockPixelsPixelRef_Deprecated();
    virtual ~SkROLockPixelsPixelRef_Deprecated();

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
 *  PixelRef that wraps a GrTexture
 */
class SK_API SkGrTexturePixelRef : public SkROLockPixelsPixelRef_Deprecated {
public:
            SkGrTexturePixelRef(GrTexture*);
    virtual ~SkGrTexturePixelRef();

    // override from SkPixelRef
    virtual SkGpuTexture* getTexture();

    SK_DECLARE_UNFLATTENABLE_OBJECT()

protected:
    // override from SkPixelRef
    virtual bool onReadPixels(SkBitmap* dst, const SkIRect* subset);

    // override from SkPixelRef
    virtual SkPixelRef* deepCopy(SkBitmap::Config dstConfig) SK_OVERRIDE;

private:
    GrTexture*  fTexture;
    typedef SkROLockPixelsPixelRef_Deprecated INHERITED;
};

/**
 *  PixelRef that wraps a GrRenderTarget
 */
class SK_API SkGrRenderTargetPixelRef : public SkROLockPixelsPixelRef_Deprecated {
public:
            SkGrRenderTargetPixelRef(GrRenderTarget* rt);
    virtual ~SkGrRenderTargetPixelRef();

    // override from SkPixelRef
    virtual SkGpuTexture* getTexture();

    SK_DECLARE_UNFLATTENABLE_OBJECT()

protected:
    // override from SkPixelRef
    virtual bool onReadPixels(SkBitmap* dst, const SkIRect* subset);

    // override from SkPixelRef
    virtual SkPixelRef* deepCopy(SkBitmap::Config dstConfig) SK_OVERRIDE;

private:
    GrRenderTarget*  fRenderTarget;
    typedef SkROLockPixelsPixelRef_Deprecated INHERITED;
};

#endif

