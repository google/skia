/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_Gpu_DEFINED
#define SkImage_Gpu_DEFINED

#include "GrTexture.h"
#include "GrGpuResourcePriv.h"
#include "SkBitmap.h"
#include "SkImage_Base.h"
#include "SkImagePriv.h"
#include "SkSurface.h"

class SkImage_Gpu : public SkImage_Base {
public:
    SK_DECLARE_INST_COUNT(SkImage_Gpu)

    SkImage_Gpu(const SkBitmap&, int sampleCountForNewSurfaces, SkSurface::Budgeted);

    void onDraw(SkCanvas*, SkScalar x, SkScalar y, const SkPaint*) const SK_OVERRIDE;
    void onDrawRect(SkCanvas*, const SkRect* src, const SkRect& dst,
                    const SkPaint*) const SK_OVERRIDE;
    SkSurface* onNewSurface(const SkImageInfo&, const SkSurfaceProps&) const SK_OVERRIDE;
    GrTexture* onGetTexture() const SK_OVERRIDE;
    bool getROPixels(SkBitmap*) const SK_OVERRIDE;

    GrTexture* getTexture() const { return fBitmap.getTexture(); }

    SkShader* onNewShader(SkShader::TileMode,
                                  SkShader::TileMode,
                                  const SkMatrix* localMatrix) const SK_OVERRIDE;

    bool isOpaque() const SK_OVERRIDE;

    void applyBudgetDecision() const {
        if (fBudgeted) {
            fBitmap.getTexture()->resourcePriv().makeBudgeted();
        } else {
            fBitmap.getTexture()->resourcePriv().makeUnbudgeted();
        }
    }

private:
    SkBitmap            fBitmap;
    const int           fSampleCountForNewSurfaces;   // 0 if we don't know
    SkSurface::Budgeted fBudgeted;

    typedef SkImage_Base INHERITED;
};

#endif
