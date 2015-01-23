/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage_Gpu.h"
#include "SkCanvas.h"
#include "GrContext.h"

SkImage_Gpu::SkImage_Gpu(const SkBitmap& bitmap, int sampleCountForNewSurfaces,
                         SkSurface::Budgeted budgeted)
    : INHERITED(bitmap.width(), bitmap.height(), NULL)
    , fBitmap(bitmap)
    , fSampleCountForNewSurfaces(sampleCountForNewSurfaces)
    , fBudgeted(budgeted)
{
    SkASSERT(fBitmap.getTexture());
}

SkShader* SkImage_Gpu::onNewShader(SkShader::TileMode tileX,
                                   SkShader::TileMode tileY,
                                   const SkMatrix* localMatrix) const
{
    return SkShader::CreateBitmapShader(fBitmap, tileX, tileY, localMatrix);
}

void SkImage_Gpu::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkPaint* paint) const {
    canvas->drawBitmap(fBitmap, x, y, paint);
}

void SkImage_Gpu::onDrawRect(SkCanvas* canvas, const SkRect* src, const SkRect& dst,
                                   const SkPaint* paint) const {
    canvas->drawBitmapRectToRect(fBitmap, src, dst, paint);
}

SkSurface* SkImage_Gpu::onNewSurface(const SkImageInfo& info, const SkSurfaceProps& props) const {
    GrContext* ctx = this->getTexture()->getContext();
    // TODO: Change signature of onNewSurface to take a budgeted param.
    static const SkSurface::Budgeted kBudgeted = SkSurface::kNo_Budgeted;
    return SkSurface::NewRenderTarget(ctx, kBudgeted, info, fSampleCountForNewSurfaces, &props);
}

GrTexture* SkImage_Gpu::onGetTexture() const {
    return fBitmap.getTexture();
}

bool SkImage_Gpu::getROPixels(SkBitmap* dst) const {
    return fBitmap.copyTo(dst, kN32_SkColorType);
}

bool SkImage_Gpu::isOpaque() const {
    return fBitmap.isOpaque();
}

///////////////////////////////////////////////////////////////////////////////

SkImage* SkNewImageFromBitmapTexture(const SkBitmap& bitmap, int sampleCountForNewSurfaces,
                                     SkSurface::Budgeted budgeted) {
    if (0 == bitmap.width() || 0 == bitmap.height() || NULL == bitmap.getTexture()) {
        return NULL;
    }
    return SkNEW_ARGS(SkImage_Gpu, (bitmap, sampleCountForNewSurfaces, budgeted));
}

GrTexture* SkTextureImageGetTexture(SkImage* image) {
    return ((SkImage_Gpu*)image)->getTexture();
}

extern void SkTextureImageApplyBudgetedDecision(SkImage* image) {
    ((SkImage_Gpu*)image)->applyBudgetDecision();
}
