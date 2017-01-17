/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_Gpu_DEFINED
#define SkImage_Gpu_DEFINED

#include "SkAtomics.h"
#include "GrTexture.h"
#include "GrGpuResourcePriv.h"
#include "SkBitmap.h"
#include "SkGr.h"
#include "SkImage_Base.h"
#include "SkImagePriv.h"
#include "SkSurface.h"

class SkImage_Gpu : public SkImage_Base {
public:
    /**
     *  An "image" can be a subset/window into a larger texture, so we explicit take the
     *  width and height.
     */
    SkImage_Gpu(GrContext* context, int w, int h, uint32_t uniqueID, SkAlphaType,
                sk_sp<GrSurfaceProxy>, sk_sp<SkColorSpace>,
                SkBudgeted);
    ~SkImage_Gpu() override;

    SkImageInfo onImageInfo() const override;
    SkAlphaType onAlphaType() const override { return fAlphaType; }

    void applyBudgetDecision() const {
#if 0
        if (SkBudgeted::kYes == fBudgeted) {
            fTexture->resourcePriv().makeBudgeted();
        } else {
            fTexture->resourcePriv().makeUnbudgeted();
        }
#endif
    }

    bool getROPixels(SkBitmap*, SkColorSpace* dstColorSpace, CachingHint) const override;
    GrTexture* asTextureRef(GrContext* ctx, const GrSamplerParams& params, SkColorSpace*,
                            sk_sp<SkColorSpace>*) const override;
    sk_sp<SkImage> onMakeSubset(const SkIRect&) const override;

    GrSurfaceProxy* peekProxy() const override { return fProxy.get(); }
    GrTexture* peekTexture1() const override { return nullptr; }
    sk_sp<GrTexture> refPinnedTexture(uint32_t* uniqueID) const override {
#if 0
        *uniqueID = this->uniqueID();
        return fTexture;
#else
        return nullptr;
#endif
    }
    bool onReadPixels(const SkImageInfo&, void* dstPixels, size_t dstRowBytes,
                      int srcX, int srcY, CachingHint) const override;

    GrContext* getContext() { return fContext; }

private:
    GrContext*             fContext;
    sk_sp<GrSurfaceProxy>  fProxy;
    const SkAlphaType      fAlphaType;
    const SkBudgeted       fBudgeted;
    sk_sp<SkColorSpace>    fColorSpace;
    mutable SkAtomic<bool> fAddedRasterVersionToCache;


    typedef SkImage_Base INHERITED;
};

#endif
