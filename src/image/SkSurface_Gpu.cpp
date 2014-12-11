/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSurface_Base.h"
#include "SkImagePriv.h"
#include "SkImage_Base.h"
#include "SkCanvas.h"
#include "SkGpuDevice.h"
#include "SkSurface_Gpu.h"

#if SK_SUPPORT_GPU

///////////////////////////////////////////////////////////////////////////////

SkSurface_Gpu::SkSurface_Gpu(GrRenderTarget* renderTarget, const SkSurfaceProps* props,
                             bool doClear)
        : INHERITED(renderTarget->width(), renderTarget->height(), props) {
    int deviceFlags = 0;
    deviceFlags |= this->props().isUseDistanceFieldFonts() ? SkGpuDevice::kDFText_Flag : 0;
    fDevice = SkGpuDevice::Create(renderTarget, this->props(), deviceFlags);

    if (kRGB_565_GrPixelConfig != renderTarget->config() && doClear) {
        fDevice->clearAll();
    }
}

SkSurface_Gpu::~SkSurface_Gpu() {
    SkSafeUnref(fDevice);
}

SkCanvas* SkSurface_Gpu::onNewCanvas() {
    SkCanvas::InitFlags flags = SkCanvas::kDefault_InitFlags;
    // When we think this works...
//    flags |= SkCanvas::kConservativeRasterClip_InitFlag;

    return SkNEW_ARGS(SkCanvas, (fDevice, &this->props(), flags));
}

SkSurface* SkSurface_Gpu::onNewSurface(const SkImageInfo& info) {
    GrRenderTarget* rt = fDevice->accessRenderTarget();
    int sampleCount = rt->numSamples();
    return SkSurface::NewRenderTarget(fDevice->context(), info, sampleCount, &this->props());
}

SkImage* SkSurface_Gpu::onNewImageSnapshot() {
    const int sampleCount = fDevice->accessRenderTarget()->numSamples();
    SkImage* image = SkNewImageFromBitmapTexture(fDevice->accessBitmap(false), sampleCount);
    if (image) {
        as_IB(image)->initWithProps(this->props());
    }
    return image;
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
    SkASSERT(this->getCachedImage());
    if (rt->asTexture() == SkTextureImageGetTexture(this->getCachedImage())) {
        // We call createCompatibleDevice because it uses the texture cache. This isn't
        // necessarily correct (http://skbug.com/2252), but never using the cache causes
        // a Chromium regression. (http://crbug.com/344020)
        SkGpuDevice* newDevice = fDevice->cloneDevice(this->props());
        SkAutoTUnref<SkGpuDevice> aurd(newDevice);
        if (kRetain_ContentChangeMode == mode) {
            fDevice->context()->copySurface(newDevice->accessRenderTarget(), rt->asTexture());
        }
        SkASSERT(this->getCachedCanvas());
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

SkSurface* SkSurface::NewRenderTargetDirect(GrRenderTarget* target, const SkSurfaceProps* props) {
    if (NULL == target) {
        return NULL;
    }
    return SkNEW_ARGS(SkSurface_Gpu, (target, props, false));
}

SkSurface* SkSurface::NewRenderTarget(GrContext* ctx, const SkImageInfo& info, int sampleCount,
                                      const SkSurfaceProps* props) {
    if (NULL == ctx) {
        return NULL;
    }

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag | kCheckAllocation_GrSurfaceFlag;
    desc.fWidth = info.width();
    desc.fHeight = info.height();
    desc.fConfig = SkImageInfo2GrPixelConfig(info);
    desc.fSampleCnt = sampleCount;

    SkAutoTUnref<GrTexture> tex(ctx->createUncachedTexture(desc, NULL, 0));
    if (NULL == tex) {
        return NULL;
    }

    return SkNEW_ARGS(SkSurface_Gpu, (tex->asRenderTarget(), props, true));
}

SkSurface* SkSurface::NewScratchRenderTarget(GrContext* ctx, const SkImageInfo& info,
                                             int sampleCount, const SkSurfaceProps* props) {
    if (NULL == ctx) {
        return NULL;
    }

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag | kCheckAllocation_GrSurfaceFlag;
    desc.fWidth = info.width();
    desc.fHeight = info.height();
    desc.fConfig = SkImageInfo2GrPixelConfig(info);
    desc.fSampleCnt = sampleCount;

    SkAutoTUnref<GrTexture> tex(ctx->refScratchTexture(desc, GrContext::kExact_ScratchTexMatch));

    if (NULL == tex) {
        return NULL;
    }

    return SkNEW_ARGS(SkSurface_Gpu, (tex->asRenderTarget(), props, true));
}

#endif
