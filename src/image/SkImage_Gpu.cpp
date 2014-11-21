/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage_Base.h"
#include "SkImagePriv.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkSurface.h"
#include "GrContext.h"
#include "GrTexture.h"

class SkImage_Gpu : public SkImage_Base {
public:
    SK_DECLARE_INST_COUNT(SkImage_Gpu)

    SkImage_Gpu(const SkBitmap&, int sampleCount);

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

private:
    SkBitmap    fBitmap;
    const int   fSampleCount;   // 0 if we weren't built from a surface

    typedef SkImage_Base INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

SkImage_Gpu::SkImage_Gpu(const SkBitmap& bitmap, int sampleCount)
    : INHERITED(bitmap.width(), bitmap.height(), NULL)
    , fBitmap(bitmap)
    , fSampleCount(sampleCount)
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
    return SkSurface::NewRenderTarget(ctx, info, fSampleCount, &props);
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

SkImage* SkNewImageFromBitmapTexture(const SkBitmap& bitmap, int sampleCount) {
    if (NULL == bitmap.getTexture()) {
        return NULL;
    }
    return SkNEW_ARGS(SkImage_Gpu, (bitmap, sampleCount));
}

SkImage* SkImage::NewTexture(const SkBitmap& bitmap) {
    return SkNewImageFromBitmapTexture(bitmap, 0);
}

GrTexture* SkTextureImageGetTexture(SkImage* image) {
    return ((SkImage_Gpu*)image)->getTexture();
}

