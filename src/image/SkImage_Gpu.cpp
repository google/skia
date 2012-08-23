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
#include "SkData.h"
#include "SkDataPixelRef.h"

class SkImage_Gpu : public SkImage_Base {
public:
    static bool ValidArgs(GrContext* context,
                          const GrPlatformTextureDesc& desc) {
        if (0 == desc.fTextureHandle) {
            return false;
        }
        if (desc.fWidth < 0 || desc.fHeight < 0) {
            return false;
        }
        return true;
    }

    SkImage_Gpu(GrContext* context, const GrPlatformTextureDesc& desc);
    virtual ~SkImage_Gpu();

    virtual void onDraw(SkCanvas*, SkScalar, SkScalar, const SkPaint*) SK_OVERRIDE;

private:
    SkBitmap    fBitmap;

    typedef SkImage_Base INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

SkImage_Gpu::SkImage_Gpu(GrContext* context, const GrPlatformTextureDesc& desc)
        : INHERITED(desc.fWidth, desc.fHeight) {
#if 0
    bool isOpaque;
    SkBitmap::Config config = SkImageInfoToBitmapConfig(info, &isOpaque);

    fBitmap.setConfig(config, info.fWidth, info.fHeight, rowBytes);
    fBitmap.setPixelRef(SkNEW_ARGS(SkDataPixelRef, (data)))->unref();
    fBitmap.setIsOpaque(isOpaque);
    fBitmap.setImmutable();
#endif
}

SkImage_Gpu::~SkImage_Gpu() {}

void SkImage_Gpu::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkPaint* paint) {
    canvas->drawBitmap(fBitmap, x, y, paint);
}

///////////////////////////////////////////////////////////////////////////////

SkImage* SkImage::NewRasterCopy(NewTexture(GrContext* context,
                                           const GrPlatformTextureDesc& desc) {
    if (NULL == context) {
        return NULL;
    }
    if (!SkImage_Gpu::ValidArgs(context, desc)) {
        return NULL;
    }

    return SkNEW_ARGS(SkImage_Gpu, (context, desc));
}

