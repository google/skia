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

    SkSurface_Gpu(GrRenderTarget*);
    virtual ~SkSurface_Gpu();

    virtual SkCanvas* onNewCanvas() SK_OVERRIDE;
    virtual SkSurface* onNewSurface(const SkImageInfo&) SK_OVERRIDE;
    virtual SkImage* onNewImageSnapshot() SK_OVERRIDE;
    virtual void onDraw(SkCanvas*, SkScalar x, SkScalar y,
                        const SkPaint*) SK_OVERRIDE;
    virtual void onCopyOnWrite(ContentChangeMode) SK_OVERRIDE;

private:
    SkGpuDevice* fDevice;

    typedef SkSurface_Base INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

SkSurface_Gpu::SkSurface_Gpu(GrRenderTarget* renderTarget)
        : INHERITED(renderTarget->width(), renderTarget->height()) {
    fDevice = SkNEW_ARGS(SkGpuDevice, (renderTarget->getContext(), renderTarget));

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

SkSurface* SkSurface_Gpu::onNewSurface(const SkImageInfo& info) {
    GrRenderTarget* rt = fDevice->accessRenderTarget();
    int sampleCount = rt->numSamples();
    return SkSurface::NewRenderTarget(fDevice->context(), info, sampleCount);
}

SkImage* SkSurface_Gpu::onNewImageSnapshot() {
    return SkImage::NewTexture(fDevice->accessBitmap(false));
}

void SkSurface_Gpu::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y,
                              const SkPaint* paint) {
    canvas->drawBitmap(fDevice->accessBitmap(false), x, y, paint);
}

// Create a new SkGpuDevice and, if necessary, copy the contents of the old
// device into it. Note that this flushes the SkGpuDevice but
// doesn't force an OpenGL flush.
void SkSurface_Gpu::onCopyOnWrite(ContentChangeMode mode) {
    GrRenderTarget* rt = fDevice->accessRenderTarget();
    // are we sharing our render target with the image?
    SkASSERT(NULL != this->getCachedImage());
    if (rt->asTexture() == SkTextureImageGetTexture(this->getCachedImage())) {
        // We call createCompatibleDevice because it uses the texture cache. This isn't
        // necessarily correct (http://skbug.com/2252), but never using the cache causes
        // a Chromium regression. (http://crbug.com/344020)
        SkGpuDevice* newDevice = static_cast<SkGpuDevice*>(
            fDevice->createCompatibleDevice(fDevice->imageInfo()));
        SkAutoTUnref<SkGpuDevice> aurd(newDevice);
        if (kRetain_ContentChangeMode == mode) {
            fDevice->context()->copyTexture(rt->asTexture(), newDevice->accessRenderTarget());
        }
        SkASSERT(NULL != this->getCachedCanvas());
        SkASSERT(this->getCachedCanvas()->getDevice() == fDevice);

        this->getCachedCanvas()->setRootDevice(newDevice);
        SkRefCnt_SafeAssign(fDevice, newDevice);
    }
}

///////////////////////////////////////////////////////////////////////////////

SkSurface* SkSurface::NewRenderTargetDirect(GrRenderTarget* target) {
    if (NULL == target) {
        return NULL;
    }
    return SkNEW_ARGS(SkSurface_Gpu, (target));
}

SkSurface* SkSurface::NewRenderTarget(GrContext* ctx, const SkImageInfo& info, int sampleCount) {
    if (NULL == ctx) {
        return NULL;
    }

    SkBitmap::Config config = SkImageInfoToBitmapConfig(info);

    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit | kCheckAllocation_GrTextureFlagBit;
    desc.fWidth = info.fWidth;
    desc.fHeight = info.fHeight;
    desc.fConfig = SkBitmapConfig2GrPixelConfig(config);
    desc.fSampleCnt = sampleCount;

    SkAutoTUnref<GrTexture> tex(ctx->createUncachedTexture(desc, NULL, 0));
    if (NULL == tex) {
        return NULL;
    }

    return SkNEW_ARGS(SkSurface_Gpu, (tex->asRenderTarget()));
}
