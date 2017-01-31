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
    SkImage_Gpu(int w, int h, uint32_t uniqueID, SkAlphaType, sk_sp<GrTexture>, sk_sp<SkColorSpace>,
                SkBudgeted);
    ~SkImage_Gpu() override;

    SkImageInfo onImageInfo() const override;
    SkAlphaType onAlphaType() const override { return fAlphaType; }

    void applyBudgetDecision() const {
        if (SkBudgeted::kYes == fBudgeted) {
            fTexture->resourcePriv().makeBudgeted();
        } else {
            fTexture->resourcePriv().makeUnbudgeted();
        }
    }

    bool getROPixels(SkBitmap*, SkColorSpace* dstColorSpace, CachingHint) const override;
    GrTexture* asTextureRef(GrContext* ctx, const GrSamplerParams& params, SkColorSpace*,
                            sk_sp<SkColorSpace>*, SkScalar scaleAdjust[2]) const override;
    sk_sp<SkImage> onMakeSubset(const SkIRect&) const override;

    GrTexture* peekTexture() const override { return fTexture.get(); }
    sk_sp<GrTextureProxy> asTextureProxyRef() const override;
    sk_sp<GrTexture> refPinnedTexture(uint32_t* uniqueID) const override {
        *uniqueID = this->uniqueID();
        return fTexture;
    }
    bool onReadPixels(const SkImageInfo&, void* dstPixels, size_t dstRowBytes,
                      int srcX, int srcY, CachingHint) const override;

    GrContext* context() { return fTexture->getContext(); }
    sk_sp<SkColorSpace> refColorSpace() { return fColorSpace; }

private:
    sk_sp<GrTexture>       fTexture;
    const SkAlphaType      fAlphaType;
    const SkBudgeted       fBudgeted;
    sk_sp<SkColorSpace>    fColorSpace;
    mutable SkAtomic<bool> fAddedRasterVersionToCache;


    typedef SkImage_Base INHERITED;
};

#endif
