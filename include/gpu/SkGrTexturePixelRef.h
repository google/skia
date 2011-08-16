
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
class SkROLockPixelsPixelRef : public SkPixelRef {
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
 *  PixelRef that wraps a GrTexture
 */
class SkGrTexturePixelRef : public SkROLockPixelsPixelRef {
public:
            SkGrTexturePixelRef(GrTexture*);
    virtual ~SkGrTexturePixelRef();

    // override from SkPixelRef
    virtual SkGpuTexture* getTexture();

protected:
    // override from SkPixelRef
    virtual bool onReadPixels(SkBitmap* dst, const SkIRect* subset);

private:
    GrTexture*  fTexture;
    typedef SkROLockPixelsPixelRef INHERITED;
};

/**
 *  PixelRef that wraps a GrRenderTarget
 */
class SkGrRenderTargetPixelRef : public SkROLockPixelsPixelRef {
public:
            SkGrRenderTargetPixelRef(GrRenderTarget* rt);
    virtual ~SkGrRenderTargetPixelRef();

    // override from SkPixelRef
    virtual SkGpuTexture* getTexture();

protected:
    // override from SkPixelRef
    virtual bool onReadPixels(SkBitmap* dst, const SkIRect* subset);

private:
    GrRenderTarget*  fRenderTarget;
    typedef SkROLockPixelsPixelRef INHERITED;
};

#endif

