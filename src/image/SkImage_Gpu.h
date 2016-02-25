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
#include "SkImage_Base.h"
#include "SkImagePriv.h"
#include "SkSurface.h"

class SkImage_Gpu : public SkImage_Base {
public:
    /**
     *  An "image" can be a subset/window into a larger texture, so we explicit take the
     *  width and height.
     */
    SkImage_Gpu(int w, int h, uint32_t uniqueID, SkAlphaType, GrTexture*, SkSurface::Budgeted);
    ~SkImage_Gpu() override;

    void applyBudgetDecision() const {
        GrTexture* tex = this->getTexture();
        SkASSERT(tex);
        if (fBudgeted) {
            tex->resourcePriv().makeBudgeted();
        } else {
            tex->resourcePriv().makeUnbudgeted();
        }
    }

    bool getROPixels(SkBitmap*, CachingHint) const override;
    GrTexture* asTextureRef(GrContext* ctx, const GrTextureParams& params) const override;
    SkImage* onNewSubset(const SkIRect&) const override;

    GrTexture* peekTexture() const override { return fTexture; }
    bool isOpaque() const override;
    bool onReadPixels(const SkImageInfo&, void* dstPixels, size_t dstRowBytes,
                      int srcX, int srcY, CachingHint) const override;

    SkSurface* onNewSurface(const SkImageInfo& info) const override {
        return SkSurface::NewRenderTarget(fTexture->getContext(), SkSurface::kNo_Budgeted, info);
    }

    bool asBitmapForImageFilters(SkBitmap* bitmap) const override;

private:
    SkAutoTUnref<GrTexture>     fTexture;
    const SkAlphaType           fAlphaType;
    const SkSurface::Budgeted   fBudgeted;
    mutable SkAtomic<bool>      fAddedRasterVersionToCache;


    typedef SkImage_Base INHERITED;
};

#endif
