/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#ifndef SkGrTexturePixelRef_DEFINED
#define SkGrTexturePixelRef_DEFINED

#include "SkBitmap.h"
#include "SkPixelRef.h"
#include "GrGpu.h"

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

