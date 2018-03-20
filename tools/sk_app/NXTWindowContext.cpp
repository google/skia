
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBackendSurface.h"
#include "GrContext.h"
#include "GrNXTBackendContext.h"
#include "SkAutoMalloc.h"
#include "SkSurface.h"
#include "NXTWindowContext.h"

static void PrintDeviceError(const char* message, nxt::CallbackUserdata) {
    printf("Device error: %s\n", message);
    SkASSERT(false);
}

namespace sk_app {

NXTWindowContext::NXTWindowContext(const DisplayParams& params, nxt::TextureFormat swapChainFormat)
    : WindowContext(params)
    , fDisplayParams(params)
    , fSwapChainFormat(swapChainFormat) {
}

void NXTWindowContext::initializeContext(int width, int height) {
    fWidth = width;
    fHeight = height;
    fBackendContext = onInitializeContext();
    fContext = GrContext::MakeNXT(fBackendContext, fDisplayParams.fGrContextOptions);

    if (!fContext) {
        return;
    }

    fSwapChainImplementation = this->createSwapChainImplementation(-1, -1, fDisplayParams);
    uint64_t impl = reinterpret_cast<int64_t>(&fSwapChainImplementation);
    fSwapChain = fBackendContext->fDevice.CreateSwapChainBuilder()
                                         .SetImplementation(impl).GetResult();
    if (!fSwapChain) {
        fContext.reset();
        return;
    }
    fSwapChain.Configure(fSwapChainFormat, nxt::TextureUsageBit::OutputAttachment, width, height);
    fBackendContext->fDevice.SetErrorCallback(PrintDeviceError, 0);
}

NXTWindowContext::~NXTWindowContext() {
}

void NXTWindowContext::destroyContext() {
    if (!fBackendContext.get()) {
        return;
    }

    this->onDestroyContext();

    fContext.reset();
    fBackendContext.reset(nullptr);
}

sk_sp<SkSurface> NXTWindowContext::getBackbufferSurface() {
    nxt::Texture texture = fSwapChain.GetNextTexture();
    GrNXTImageInfo imageInfo;
    imageInfo.fTexture = texture.Release();
    imageInfo.fFormat = fSwapChainFormat;
    imageInfo.fLevelCount = 1; // FIXME
    GrBackendTexture backendTexture(fWidth, fHeight, imageInfo);
    fSurface = SkSurface::MakeFromBackendTextureAsRenderTarget(fContext.get(),
                                                               backendTexture,
#ifdef SK_NXT_OPENGL
                                                               kBottomLeft_GrSurfaceOrigin,
#else
                                                               kTopLeft_GrSurfaceOrigin,
#endif
                                                               fDisplayParams.fMSAASampleCount,
                                                               fDisplayParams.fColorSpace,
                                                               &fSurfaceProps);
    return fSurface;
}

void NXTWindowContext::swapBuffers() {
    GrNXTImageInfo* imageInfo;
    fSurface->getRenderTargetHandle((GrBackendObject*)&imageInfo,
                                    SkSurface::kFlushRead_BackendHandleAccess);

    nxtTextureTransitionUsage(imageInfo->fTexture, NXT_TEXTURE_USAGE_BIT_PRESENT);
    fSwapChain.Present(imageInfo->fTexture);
    this->onSwapBuffers();
}

void NXTWindowContext::resize(int w, int h) {
    fWidth = w;
    fHeight = h;
    fSwapChainImplementation = this->createSwapChainImplementation(w, h, fDisplayParams);
    uint64_t impl = reinterpret_cast<int64_t>(&fSwapChainImplementation);
    fSwapChain = fBackendContext->fDevice.CreateSwapChainBuilder()
                                         .SetImplementation(impl).GetResult();
    if (!fSwapChain) {
        fContext.reset();
        return;
    }
    fSwapChain.Configure(fSwapChainFormat, nxt::TextureUsageBit::OutputAttachment, fWidth, fHeight);
}

void NXTWindowContext::setDisplayParams(const DisplayParams& params) {
    fDisplayParams = params;
}

}   //namespace sk_app
