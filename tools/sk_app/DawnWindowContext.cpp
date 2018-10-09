
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBackendSurface.h"
#include "GrContext.h"
#include "SkAutoMalloc.h"
#include "SkSurface.h"
#include "DawnWindowContext.h"
#include "dawn/GrDawnBackendContext.h"

static void PrintDeviceError(const char* message, dawn::CallbackUserdata) {
    printf("Device error: %s\n", message);
    SkASSERT(false);
}

namespace sk_app {

DawnWindowContext::DawnWindowContext(const DisplayParams& params, dawn::TextureFormat swapChainFormat)
    : WindowContext(params)
    , fSwapChainFormat(swapChainFormat) {
}

void DawnWindowContext::initializeContext(int width, int height) {
    fWidth = width;
    fHeight = height;
    fBackendContext = onInitializeContext();
    fContext = GrContext::MakeDawn(fBackendContext, fDisplayParams.fGrContextOptions);

    if (!fContext) {
        return;
    }
    fDevice = dawn::Device(fBackendContext->fDevice);
    fSwapChainImplementation = this->createSwapChainImplementation(-1, -1, fDisplayParams);
    uint64_t impl = reinterpret_cast<int64_t>(&fSwapChainImplementation);
    fSwapChain = fDevice.CreateSwapChainBuilder().SetImplementation(impl).GetResult();
    if (!fSwapChain) {
        fContext.reset();
        return;
    }
    fSwapChain.Configure(fSwapChainFormat, dawn::TextureUsageBit::OutputAttachment, width, height);
    fDevice.SetErrorCallback(PrintDeviceError, 0);
}

DawnWindowContext::~DawnWindowContext() {
}

void DawnWindowContext::destroyContext() {
    if (!fBackendContext.get()) {
        return;
    }

    this->onDestroyContext();

    fContext.reset();
    fBackendContext.reset(nullptr);
}

sk_sp<SkSurface> DawnWindowContext::getBackbufferSurface() {
    GrDawnImageInfo imageInfo;
    imageInfo.fTexture = fSwapChain.GetNextTexture();
    imageInfo.fFormat = fSwapChainFormat;
    imageInfo.fLevelCount = 1; // FIXME
    GrBackendTexture backendTexture(fWidth, fHeight, imageInfo);
    fSurface = SkSurface::MakeFromBackendTextureAsRenderTarget(fContext.get(),
                                                               backendTexture,
#ifdef SK_DAWN_OPENGL
                                                               kBottomLeft_GrSurfaceOrigin,
#else
                                                               kTopLeft_GrSurfaceOrigin,
#endif
                                                               fDisplayParams.fMSAASampleCount,
                                                               fDisplayParams.fColorType,
                                                               fDisplayParams.fColorSpace,
                                                               &fDisplayParams.fSurfaceProps);
    return fSurface;
}

void DawnWindowContext::swapBuffers() {
    GrBackendRenderTarget backendRT = fSurface->getBackendRenderTarget(
        SkSurface::kFlushRead_BackendHandleAccess);
    GrDawnImageInfo imageInfo;
    SkAssertResult(backendRT.getDawnImageInfo(&imageInfo));

    fSwapChain.Present(imageInfo.fTexture);
    this->onSwapBuffers();
}

void DawnWindowContext::resize(int w, int h) {
    fWidth = w;
    fHeight = h;
    fSwapChainImplementation = this->createSwapChainImplementation(w, h, fDisplayParams);
    uint64_t impl = reinterpret_cast<int64_t>(&fSwapChainImplementation);
    fSwapChain = fDevice.CreateSwapChainBuilder().SetImplementation(impl).GetResult();
    if (!fSwapChain) {
        fContext.reset();
        return;
    }
    fSwapChain.Configure(fSwapChainFormat, dawn::TextureUsageBit::OutputAttachment, fWidth,
                         fHeight);
}

void DawnWindowContext::setDisplayParams(const DisplayParams& params) {
    fDisplayParams = params;
}

}   //namespace sk_app
