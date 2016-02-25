/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSurface_Gpu.h"

#include "GrResourceProvider.h"
#include "SkCanvas.h"
#include "SkGpuDevice.h"
#include "SkImage_Base.h"
#include "SkImage_Gpu.h"
#include "SkImagePriv.h"
#include "SkSurface_Base.h"

#if SK_SUPPORT_GPU

SkSurface_Gpu::SkSurface_Gpu(SkGpuDevice* device)
    : INHERITED(device->width(), device->height(), &device->surfaceProps())
    , fDevice(SkRef(device)) {
}

SkSurface_Gpu::~SkSurface_Gpu() {
    fDevice->unref();
}

static GrRenderTarget* prepare_rt_for_external_access(SkSurface_Gpu* surface,
                                                      SkSurface::BackendHandleAccess access) {
    switch (access) {
        case SkSurface::kFlushRead_BackendHandleAccess:
            break;
        case SkSurface::kFlushWrite_BackendHandleAccess:
        case SkSurface::kDiscardWrite_BackendHandleAccess:
            // for now we don't special-case on Discard, but we may in the future.
            surface->notifyContentWillChange(SkSurface::kRetain_ContentChangeMode);
            // legacy: need to dirty the bitmap's genID in our device (curse it)
            surface->getDevice()->accessBitmap(false).notifyPixelsChanged();
            break;
    }

    // Grab the render target *after* firing notifications, as it may get switched if CoW kicks in.
    GrRenderTarget* rt = surface->getDevice()->accessRenderTarget();
    rt->prepareForExternalIO();
    return rt;
}

GrBackendObject SkSurface_Gpu::onGetTextureHandle(BackendHandleAccess access) {
    GrRenderTarget* rt = prepare_rt_for_external_access(this, access);
    GrTexture* texture = rt->asTexture();
    if (texture) {
        return texture->getTextureHandle();
    }
    return 0;
}

bool SkSurface_Gpu::onGetRenderTargetHandle(GrBackendObject* obj, BackendHandleAccess access) {
    GrRenderTarget* rt = prepare_rt_for_external_access(this, access);
    *obj = rt->getRenderTargetHandle();
    return true;
}

SkCanvas* SkSurface_Gpu::onNewCanvas() {
    SkCanvas::InitFlags flags = SkCanvas::kDefault_InitFlags;
    flags = static_cast<SkCanvas::InitFlags>(flags | SkCanvas::kConservativeRasterClip_InitFlag);

    return new SkCanvas(fDevice, flags);
}

SkSurface* SkSurface_Gpu::onNewSurface(const SkImageInfo& info) {
    GrRenderTarget* rt = fDevice->accessRenderTarget();
    int sampleCount = rt->numColorSamples();
    // TODO: Make caller specify this (change virtual signature of onNewSurface).
    static const SkBudgeted kBudgeted = SkBudgeted::kNo;
    return SkSurface::NewRenderTarget(fDevice->context(), kBudgeted, info, sampleCount,
                                      &this->props());
}

SkImage* SkSurface_Gpu::onNewImageSnapshot(SkBudgeted budgeted, ForceCopyMode forceCopyMode) {
    GrRenderTarget* rt = fDevice->accessRenderTarget();
    SkASSERT(rt);
    GrTexture* tex = rt->asTexture();
    SkAutoTUnref<GrTexture> copy;
    // TODO: Force a copy when the rt is an external resource.
    if (kYes_ForceCopyMode == forceCopyMode || !tex) {
        GrSurfaceDesc desc = fDevice->accessRenderTarget()->desc();
        GrContext* ctx = fDevice->context();
        desc.fFlags = desc.fFlags & ~kRenderTarget_GrSurfaceFlag;
        copy.reset(ctx->textureProvider()->createTexture(desc, budgeted));
        if (!copy) {
            return nullptr;
        }
        if (!ctx->copySurface(copy, rt)) {
            return nullptr;
        }
        tex = copy;
    }
    const SkImageInfo info = fDevice->imageInfo();
    SkImage* image = nullptr;
    if (tex) {
        image = new SkImage_Gpu(info.width(), info.height(), kNeedNewImageUniqueID,
                                info.alphaType(), tex, budgeted);
    }
    return image;
}

// Create a new render target and, if necessary, copy the contents of the old
// render target into it. Note that this flushes the SkGpuDevice but
// doesn't force an OpenGL flush.
void SkSurface_Gpu::onCopyOnWrite(ContentChangeMode mode) {
    GrRenderTarget* rt = fDevice->accessRenderTarget();
    // are we sharing our render target with the image? Note this call should never create a new
    // image because onCopyOnWrite is only called when there is a cached image.
    SkAutoTUnref<SkImage> image(this->refCachedImage(SkBudgeted::kNo, kNo_ForceUnique));
    SkASSERT(image);
    if (rt->asTexture() == as_IB(image)->getTexture()) {
        this->fDevice->replaceRenderTarget(SkSurface::kRetain_ContentChangeMode == mode);
        SkTextureImageApplyBudgetedDecision(image);
    } else if (kDiscard_ContentChangeMode == mode) {
        this->SkSurface_Gpu::onDiscard();
    }
}

void SkSurface_Gpu::onDiscard() {
    fDevice->accessRenderTarget()->discard();
}

void SkSurface_Gpu::onPrepareForExternalIO() {
    fDevice->accessRenderTarget()->prepareForExternalIO();
}

///////////////////////////////////////////////////////////////////////////////

SkSurface* SkSurface::NewRenderTargetDirect(GrRenderTarget* target, const SkSurfaceProps* props) {
    SkAutoTUnref<SkGpuDevice> device(
        SkGpuDevice::Create(target, props, SkGpuDevice::kUninit_InitContents));
    if (!device) {
        return nullptr;
    }
    return new SkSurface_Gpu(device);
}

SkSurface* SkSurface::NewRenderTarget(GrContext* ctx, SkBudgeted budgeted, const SkImageInfo& info,
                                      int sampleCount, const SkSurfaceProps* props,
                                      GrTextureStorageAllocator customAllocator) {
    SkAutoTUnref<SkGpuDevice> device(SkGpuDevice::Create(
            ctx, budgeted, info, sampleCount, props, SkGpuDevice::kClear_InitContents,
            customAllocator));
    if (!device) {
        return nullptr;
    }
    return new SkSurface_Gpu(device);
}

SkSurface* SkSurface::NewFromBackendTexture(GrContext* context, const GrBackendTextureDesc& desc,
                                            const SkSurfaceProps* props) {
    if (nullptr == context) {
        return nullptr;
    }
    if (!SkToBool(desc.fFlags & kRenderTarget_GrBackendTextureFlag)) {
        return nullptr;
    }
    SkAutoTUnref<GrSurface> surface(context->textureProvider()->wrapBackendTexture(desc,
                                    kBorrow_GrWrapOwnership));
    if (!surface) {
        return nullptr;
    }
    SkAutoTUnref<SkGpuDevice> device(SkGpuDevice::Create(surface->asRenderTarget(), props,
                                                         SkGpuDevice::kUninit_InitContents));
    if (!device) {
        return nullptr;
    }
    return new SkSurface_Gpu(device);
}

SkSurface* SkSurface::NewFromBackendRenderTarget(GrContext* context,
                                                 const GrBackendRenderTargetDesc& desc,
                                                 const SkSurfaceProps* props) {
    if (nullptr == context) {
        return nullptr;
    }
    SkAutoTUnref<GrRenderTarget> rt(context->textureProvider()->wrapBackendRenderTarget(desc));
    if (!rt) {
        return nullptr;
    }
    SkAutoTUnref<SkGpuDevice> device(SkGpuDevice::Create(rt, props,
                                                         SkGpuDevice::kUninit_InitContents));
    if (!device) {
        return nullptr;
    }
    return new SkSurface_Gpu(device);
}

SkSurface* SkSurface::NewFromBackendTextureAsRenderTarget(GrContext* context,
                                                          const GrBackendTextureDesc& desc,
                                                          const SkSurfaceProps* props) {
    if (nullptr == context) {
        return nullptr;
    }
    SkAutoTUnref<GrRenderTarget> rt(
            context->resourceProvider()->wrapBackendTextureAsRenderTarget(desc));
    if (!rt) {
        return nullptr;
    }
    SkAutoTUnref<SkGpuDevice> device(SkGpuDevice::Create(rt, props,
                                                         SkGpuDevice::kUninit_InitContents));
    if (!device) {
        return nullptr;
    }
    return new SkSurface_Gpu(device);
}

#endif
