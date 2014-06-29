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

    SkSurface_Gpu(GrRenderTarget*, bool cached, TextRenderMode trm, 
                  SkSurface::RenderTargetFlags flags);
    virtual ~SkSurface_Gpu();

    virtual SkCanvas* onNewCanvas() SK_OVERRIDE;
    virtual SkSurface* onNewSurface(const SkImageInfo&) SK_OVERRIDE;
    virtual SkImage* onNewImageSnapshot() SK_OVERRIDE;
    virtual void onDraw(SkCanvas*, SkScalar x, SkScalar y,
                        const SkPaint*) SK_OVERRIDE;
    virtual void onCopyOnWrite(ContentChangeMode) SK_OVERRIDE;
    virtual void onDiscard() SK_OVERRIDE;

private:
    SkGpuDevice* fDevice;

    typedef SkSurface_Base INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

SkSurface_Gpu::SkSurface_Gpu(GrRenderTarget* renderTarget, bool cached, TextRenderMode trm,
                             SkSurface::RenderTargetFlags flags)
        : INHERITED(renderTarget->width(), renderTarget->height()) {
    int deviceFlags = 0;
    deviceFlags |= cached ? SkGpuDevice::kCached_Flag : 0;
    deviceFlags |= (kDistanceField_TextRenderMode == trm) ? SkGpuDevice::kDFFonts_Flag : 0;
    fDevice = SkGpuDevice::Create(renderTarget, deviceFlags);

    if (kRGB_565_GrPixelConfig != renderTarget->config() && 
        !(flags & kDontClear_RenderTargetFlag)) {
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
    } else if (kDiscard_ContentChangeMode == mode) {
        this->SkSurface_Gpu::onDiscard();
    }
}

void SkSurface_Gpu::onDiscard() {
    fDevice->accessRenderTarget()->discard();
}

///////////////////////////////////////////////////////////////////////////////

SkSurface* SkSurface::NewRenderTargetDirect(GrRenderTarget* target, TextRenderMode trm,
                                            RenderTargetFlags flags) {
    if (NULL == target) {
        return NULL;
    }
    return SkNEW_ARGS(SkSurface_Gpu, (target, false, trm, flags));
}

SkSurface* SkSurface::NewRenderTarget(GrContext* ctx, const SkImageInfo& info, int sampleCount,
                                      TextRenderMode trm, RenderTargetFlags flags) {
    if (NULL == ctx) {
        return NULL;
    }

    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit | kCheckAllocation_GrTextureFlagBit;
    desc.fWidth = info.width();
    desc.fHeight = info.height();
    desc.fConfig = SkImageInfo2GrPixelConfig(info);
    desc.fSampleCnt = sampleCount;

    SkAutoTUnref<GrTexture> tex(ctx->createUncachedTexture(desc, NULL, 0));
    if (NULL == tex) {
        return NULL;
    }

    return SkNEW_ARGS(SkSurface_Gpu, (tex->asRenderTarget(), false, trm, flags));
}

SkSurface* SkSurface::NewScratchRenderTarget(GrContext* ctx, const SkImageInfo& info,
                                             int sampleCount, TextRenderMode trm,
                                             RenderTargetFlags flags) {
    if (NULL == ctx) {
        return NULL;
    }

    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit | kCheckAllocation_GrTextureFlagBit;
    desc.fWidth = info.width();
    desc.fHeight = info.height();
    desc.fConfig = SkImageInfo2GrPixelConfig(info);
    desc.fSampleCnt = sampleCount;

    SkAutoTUnref<GrTexture> tex(ctx->lockAndRefScratchTexture(desc, GrContext::kExact_ScratchTexMatch));

    if (NULL == tex) {
        return NULL;
    }

    return SkNEW_ARGS(SkSurface_Gpu, (tex->asRenderTarget(), true, trm, flags));
}
