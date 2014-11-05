
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTest.h"

#include "GrInOrderDrawBuffer.h"
#include "GrResourceCache.h"

void GrTestTarget::init(GrContext* ctx, GrDrawTarget* target) {
    SkASSERT(!fContext);

    fContext.reset(SkRef(ctx));
    fDrawTarget.reset(SkRef(target));

    SkNEW_IN_TLAZY(&fASR, GrDrawTarget::AutoStateRestore, (target, GrDrawTarget::kReset_ASRInit));
    SkNEW_IN_TLAZY(&fACR, GrDrawTarget::AutoClipRestore, (target));
    SkNEW_IN_TLAZY(&fAGP, GrDrawTarget::AutoGeometryPush, (target));
}

void GrContext::getTestTarget(GrTestTarget* tar) {
    this->flush();
    // We could create a proxy GrDrawTarget that passes through to fGpu until ~GrTextTarget() and
    // then disconnects. This would help prevent test writers from mixing using the returned
    // GrDrawTarget and regular drawing. We could also assert or fail in GrContext drawing methods
    // until ~GrTestTarget().
    tar->init(this, fDrawBuffer);
}

///////////////////////////////////////////////////////////////////////////////

void GrContext::setMaxTextureSizeOverride(int maxTextureSizeOverride) {
    fMaxTextureSizeOverride = maxTextureSizeOverride;
}

void GrContext::purgeAllUnlockedResources() {
    fResourceCache->purgeAllUnlocked();
}

///////////////////////////////////////////////////////////////////////////////
// Code for the mock context. It's built on a mock GrGpu class that does nothing.
////

#include "GrBufferAllocPool.h"
#include "GrInOrderDrawBuffer.h"
#include "GrGpu.h"

class MockGpu : public GrGpu {
public:
    MockGpu(GrContext* context) : INHERITED(context) { fCaps.reset(SkNEW(GrDrawTargetCaps)); }
    virtual ~MockGpu() { }
    virtual bool canWriteTexturePixels(const GrTexture*,
                                       GrPixelConfig srcConfig) const SK_OVERRIDE {
        return true;
    }

    virtual bool readPixelsWillPayForYFlip(GrRenderTarget* renderTarget,
                                           int left, int top,
                                           int width, int height,
                                           GrPixelConfig config,
                                           size_t rowBytes) const SK_OVERRIDE { return false; }
    virtual void buildProgramDesc(const GrOptDrawState&,
                                  const GrProgramDesc::DescInfo&,
                                  GrGpu::DrawType,
                                  const GrDeviceCoordTexture* dstCopy,
                                  GrProgramDesc* desc) SK_OVERRIDE { }

    virtual void discard(GrRenderTarget*) SK_OVERRIDE { }

private:
    virtual void onResetContext(uint32_t resetBits) { };
    virtual GrTexture* onCreateTexture(const GrSurfaceDesc& desc,
                                       const void* srcData,
                                       size_t rowBytes)  SK_OVERRIDE {
        return NULL;
    }

    virtual GrTexture* onCreateCompressedTexture(const GrSurfaceDesc& desc,
                                                 const void* srcData)  SK_OVERRIDE {
        return NULL;
    }

    virtual GrTexture* onWrapBackendTexture(const GrBackendTextureDesc&)  SK_OVERRIDE {
        return NULL;
    }

    virtual GrRenderTarget* onWrapBackendRenderTarget(
                                    const GrBackendRenderTargetDesc&) SK_OVERRIDE {
        return NULL;
    }

    virtual GrVertexBuffer* onCreateVertexBuffer(size_t size, bool dynamic)  SK_OVERRIDE {
        return NULL;
    }

    virtual GrIndexBuffer* onCreateIndexBuffer(size_t size, bool dynamic)  SK_OVERRIDE {
        return NULL;
    }

    virtual void onGpuClear(GrRenderTarget*, const SkIRect* rect, GrColor color,
                            bool canIgnoreRect)  SK_OVERRIDE { }

    virtual void onClearStencilClip(GrRenderTarget*,
                                    const SkIRect& rect,
                                    bool insideClip)  SK_OVERRIDE { }

                                    virtual void onGpuDraw(const DrawInfo&)  SK_OVERRIDE { }
    virtual bool onReadPixels(GrRenderTarget* target,
                              int left, int top, int width, int height,
                              GrPixelConfig,
                              void* buffer,
                              size_t rowBytes)  SK_OVERRIDE {
        return false;
    }

    virtual bool onWriteTexturePixels(GrTexture* texture,
                                      int left, int top, int width, int height,
                                      GrPixelConfig config, const void* buffer,
                                      size_t rowBytes)  SK_OVERRIDE {
        return false;
    }

    virtual void onResolveRenderTarget(GrRenderTarget* target)  SK_OVERRIDE {
        return;
    }

    virtual bool createStencilBufferForRenderTarget(GrRenderTarget*, int width,
                                                    int height) SK_OVERRIDE {
        return false;
    }

    virtual bool attachStencilBufferToRenderTarget(GrStencilBuffer*, GrRenderTarget*)  SK_OVERRIDE {
        return false;
    }

    virtual bool flushGraphicsState(DrawType,
                                    const GrClipMaskManager::ScissorState&,
                                    const GrDeviceCoordTexture* dstCopy)  SK_OVERRIDE {
        return false;
    }

    virtual void clearStencil(GrRenderTarget* target)  SK_OVERRIDE  { }

    virtual void didAddGpuTraceMarker() SK_OVERRIDE { }
    virtual void didRemoveGpuTraceMarker() SK_OVERRIDE { }

    typedef GrGpu INHERITED;
};

GrContext* GrContext::CreateMockContext() {
    GrContext* context = SkNEW_ARGS(GrContext, (Options()));

    context->initMockContext();
    return context;
}

void GrContext::initMockContext() {
    SkASSERT(NULL == fGpu);
    fGpu = SkNEW_ARGS(MockGpu, (this));
    SkASSERT(fGpu);
    this->initCommon();

    // We delete these because we want to test the cache starting with zero resources. Also, none of
    // these objects are required for any of tests that use this context. TODO: make stop allocating
    // resources in the buffer pools.
    SkDELETE(fDrawBuffer);
    SkDELETE(fDrawBufferVBAllocPool);
    SkDELETE(fDrawBufferIBAllocPool);

    fDrawBuffer = NULL;
    fDrawBufferVBAllocPool = NULL;
    fDrawBufferIBAllocPool = NULL;
}
