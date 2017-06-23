/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_Gpu_DEFINED
#define SkImage_Gpu_DEFINED

#include "GrClip.h"
#include "GrContext.h"
#include "GrGpuResourcePriv.h"
#include "GrSurfaceProxyPriv.h"
#include "SkAtomics.h"
#include "SkBitmap.h"
#include "SkGr.h"
#include "SkImagePriv.h"
#include "SkImage_Base.h"
#include "SkSurface.h"

class GrTexture;

class SkImage_Gpu : public SkImage_Base {
public:
    SkImage_Gpu(GrContext*, uint32_t uniqueID, SkAlphaType, sk_sp<GrTextureProxy>,
                sk_sp<SkColorSpace>, SkBudgeted);
    ~SkImage_Gpu() override;

    SkImageInfo onImageInfo() const override;
    SkAlphaType onAlphaType() const override { return fAlphaType; }

    bool getROPixels(SkBitmap*, SkColorSpace* dstColorSpace, CachingHint) const override;
    sk_sp<SkImage> onMakeSubset(const SkIRect&) const override;

    GrContext* context() const override { return fContext; }
    GrTextureProxy* peekProxy() const override {
        return fProxy.get();
    }
    sk_sp<GrTextureProxy> asTextureProxyRef() const override {
        return fProxy;
    }
    sk_sp<GrTextureProxy> asTextureProxyRef(GrContext*, const GrSamplerParams&, SkColorSpace*,
                                            sk_sp<SkColorSpace>*,
                                            SkScalar scaleAdjust[2]) const override;

    sk_sp<GrTextureProxy> refPinnedTextureProxy(uint32_t* uniqueID) const override {
        *uniqueID = this->uniqueID();
        return fProxy;
    }
    GrBackendObject onGetTextureHandle(bool flushPendingGrContextIO,
                                       GrSurfaceOrigin* origin) const override;
    GrTexture* onGetTexture() const override;

    bool onReadPixels(const SkImageInfo&, void* dstPixels, size_t dstRowBytes,
                      int srcX, int srcY, CachingHint) const override;

    sk_sp<SkColorSpace> refColorSpace() { return fColorSpace; }

    sk_sp<SkImage> onMakeColorSpace(sk_sp<SkColorSpace>, SkColorType,
                                    SkTransferFunctionBehavior) const override;

    bool onIsValid(GrContext*) const override;

private:
    GrContext*             fContext;
    sk_sp<GrTextureProxy>  fProxy;
    const SkAlphaType      fAlphaType;
    const SkBudgeted       fBudgeted;
    sk_sp<SkColorSpace>    fColorSpace;
    mutable SkAtomic<bool> fAddedRasterVersionToCache;


    typedef SkImage_Base INHERITED;
};

#endif
