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
    SkImage_Gpu(int w, int h, uint32_t uniqueID, SkAlphaType, GrTexture*,
                int sampleCountForNewSurfaces, SkSurface::Budgeted);
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

    bool getROPixels(SkBitmap*) const override;
    GrTexture* getTexture() const override { return fTexture; }
    SkShader* onNewShader(SkShader::TileMode,
                          SkShader::TileMode,
                          const SkMatrix* localMatrix) const override;
    bool isOpaque() const override;
    SkSurface* onNewSurface(const SkImageInfo&, const SkSurfaceProps&) const override;
    bool onReadPixels(const SkImageInfo&, void* dstPixels, size_t dstRowBytes,
                      int srcX, int srcY) const override;

private:
    SkAutoTUnref<GrTexture>     fTexture;
    const int                   fSampleCountForNewSurfaces;   // 0 if we don't know
    const SkAlphaType           fAlphaType;
    const SkSurface::Budgeted   fBudgeted;
    mutable SkAtomic<bool>      fAddedRasterVersionToCache;


    typedef SkImage_Base INHERITED;
};

#endif
