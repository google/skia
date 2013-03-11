/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSurface_Base.h"
#include "SkImagePriv.h"
#include "SkCanvas.h"
#include "SkGpuDevice.h"

class SkSurface_Gpu : public SkSurface_Base {
public:
    SK_DECLARE_INST_COUNT(SkSurface_Gpu)

    SkSurface_Gpu(GrContext*, const SkImage::Info&, int sampleCount);
    SkSurface_Gpu(GrContext*, GrRenderTarget*);
    virtual ~SkSurface_Gpu();

    virtual SkCanvas* onNewCanvas() SK_OVERRIDE;
    virtual SkSurface* onNewSurface(const SkImage::Info&) SK_OVERRIDE;
    virtual SkImage* onNewImageShapshot() SK_OVERRIDE;
    virtual void onDraw(SkCanvas*, SkScalar x, SkScalar y,
                        const SkPaint*) SK_OVERRIDE;
    virtual void onCopyOnWrite(SkImage*, SkCanvas*) SK_OVERRIDE;

private:
    SkGpuDevice* fDevice;

    typedef SkSurface_Base INHERITED;
};

SK_DEFINE_INST_COUNT(SkSurface_Gpu)

///////////////////////////////////////////////////////////////////////////////

SkSurface_Gpu::SkSurface_Gpu(GrContext* ctx, const SkImage::Info& info,
                             int sampleCount)
        : INHERITED(info.fWidth, info.fHeight) {
    bool isOpaque;
    SkBitmap::Config config = SkImageInfoToBitmapConfig(info, &isOpaque);

    fDevice = SkNEW_ARGS(SkGpuDevice, (ctx, config, info.fWidth, info.fHeight, sampleCount));

    if (!isOpaque) {
        fDevice->clear(0x0);
    }
}

SkSurface_Gpu::SkSurface_Gpu(GrContext* ctx, GrRenderTarget* renderTarget)
        : INHERITED(renderTarget->width(), renderTarget->height()) {
    fDevice = SkNEW_ARGS(SkGpuDevice, (ctx, renderTarget));

    if (kRGB_565_GrPixelConfig != renderTarget->config()) {
        fDevice->clear(0x0);
    }
}

SkSurface_Gpu::~SkSurface_Gpu() {
    SkSafeUnref(fDevice);
}

SkCanvas* SkSurface_Gpu::onNewCanvas() {
    return SkNEW_ARGS(SkCanvas, (fDevice));
}

SkSurface* SkSurface_Gpu::onNewSurface(const SkImage::Info& info) {
    GrRenderTarget* rt = (GrRenderTarget*) fDevice->accessRenderTarget();
    int sampleCount = rt->numSamples();
    return SkSurface::NewRenderTarget(fDevice->context(), info, sampleCount);
}

SkImage* SkSurface_Gpu::onNewImageShapshot() {

    GrRenderTarget* rt = (GrRenderTarget*) fDevice->accessRenderTarget();

    return SkImage::NewTexture(rt->asTexture());
}

void SkSurface_Gpu::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y,
                              const SkPaint* paint) {
    canvas->drawBitmap(fDevice->accessBitmap(false), x, y, paint);
}

// Copy the contents of the SkGpuDevice into a new texture and give that
// texture to the SkImage. Note that this flushes the SkGpuDevice but
// doesn't force an OpenGL flush.
void SkSurface_Gpu::onCopyOnWrite(SkImage* image, SkCanvas*) {
    GrRenderTarget* rt = (GrRenderTarget*) fDevice->accessRenderTarget();

    // are we sharing our render target with the image?
    if (rt->asTexture() == SkTextureImageGetTexture(image)) {
        GrTextureDesc desc;
        // copyTexture requires a render target as the destination
        desc.fFlags = kRenderTarget_GrTextureFlagBit;
        desc.fWidth = fDevice->width();
        desc.fHeight = fDevice->height();
        desc.fConfig = SkBitmapConfig2GrPixelConfig(fDevice->config());
        desc.fSampleCnt = 0;

        SkAutoTUnref<GrTexture> tex(fDevice->context()->createUncachedTexture(desc, NULL, 0));
        if (NULL == tex) {
            SkTextureImageSetTexture(image, NULL);
            return;
        }

        fDevice->context()->copyTexture(rt->asTexture(), tex->asRenderTarget());

        SkTextureImageSetTexture(image, tex);
    }
}

///////////////////////////////////////////////////////////////////////////////

SkSurface* SkSurface::NewRenderTargetDirect(GrContext* ctx,
                                            GrRenderTarget* target) {
    if (NULL == ctx || NULL == target) {
        return NULL;
    }

    return SkNEW_ARGS(SkSurface_Gpu, (ctx, target));
}

SkSurface* SkSurface::NewRenderTarget(GrContext* ctx, const SkImage::Info& info, int sampleCount) {
    if (NULL == ctx) {
        return NULL;
    }

    bool isOpaque;
    SkBitmap::Config config = SkImageInfoToBitmapConfig(info, &isOpaque);

    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit;
    desc.fWidth = info.fWidth;
    desc.fHeight = info.fHeight;
    desc.fConfig = SkBitmapConfig2GrPixelConfig(config);
    desc.fSampleCnt = sampleCount;

    SkAutoTUnref<GrTexture> tex(ctx->createUncachedTexture(desc, NULL, 0));
    if (NULL == tex) {
        return NULL;
    }

    return SkNEW_ARGS(SkSurface_Gpu, (ctx, tex->asRenderTarget()));
}
