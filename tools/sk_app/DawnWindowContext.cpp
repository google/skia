/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "src/core/SkAutoMalloc.h"
#include "tools/sk_app/DawnWindowContext.h"

static void PrintDeviceError(DawnErrorType, const char* message, void*) {
    printf("Device error: %s\n", message);
    SkASSERT(false);
}

namespace sk_app {

DawnWindowContext::DawnWindowContext(const DisplayParams& params,
                                     dawn::TextureFormat swapChainFormat)
    : WindowContext(params)
    , fSwapChainFormat(swapChainFormat)
    , fInstance(std::make_unique<dawn_native::Instance>()) {
}

void DawnWindowContext::initializeContext(int width, int height) {
    fWidth = width;
    fHeight = height;
    fDevice = onInitializeContext();
    fContext = GrContext::MakeDawn(fDevice, fDisplayParams.fGrContextOptions);

    if (!fContext) {
        return;
    }
    fSwapChainImplementation = this->createSwapChainImplementation(-1, -1, fDisplayParams);
    dawn::SwapChainDescriptor swapChainDesc;
    swapChainDesc.implementation = reinterpret_cast<int64_t>(&fSwapChainImplementation);
    fSwapChain = fDevice.CreateSwapChain(&swapChainDesc);
    if (!fSwapChain) {
        fContext.reset();
        return;
    }
    fSwapChain.Configure(fSwapChainFormat, dawn::TextureUsage::OutputAttachment, width, height);
    fDevice.SetUncapturedErrorCallback(PrintDeviceError, 0);
}

DawnWindowContext::~DawnWindowContext() {
}

void DawnWindowContext::destroyContext() {
    if (!fDevice.Get()) {
        return;
    }

    this->onDestroyContext();

    fContext.reset();
    fDevice = nullptr;
}

sk_sp<SkSurface> DawnWindowContext::getBackbufferSurface() {
    GrDawnImageInfo imageInfo;
    imageInfo.fTexture = fSwapChain.GetNextTexture();
    imageInfo.fFormat = fSwapChainFormat;
    imageInfo.fLevelCount = 1; // FIXME
    GrBackendTexture backendTexture(fWidth, fHeight, imageInfo);
    fSurface = SkSurface::MakeFromBackendTextureAsRenderTarget(fContext.get(),
                                                               backendTexture,
                                                               this->getRTOrigin(),
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
    dawn::SwapChainDescriptor swapChainDesc;
    swapChainDesc.implementation = reinterpret_cast<int64_t>(&fSwapChainImplementation);
    fSwapChain = fDevice.CreateSwapChain(&swapChainDesc);
    if (!fSwapChain) {
        fContext.reset();
        return;
    }
    fSwapChain.Configure(fSwapChainFormat, dawn::TextureUsage::OutputAttachment, fWidth,
                         fHeight);
}

void DawnWindowContext::setDisplayParams(const DisplayParams& params) {
    fDisplayParams = params;
}

dawn::Device DawnWindowContext::createDevice(dawn_native::BackendType type) {
    fInstance->DiscoverDefaultAdapters();
    DawnProcTable backendProcs = dawn_native::GetProcs();
    dawnSetProcs(&backendProcs);

    std::vector<dawn_native::Adapter> adapters = fInstance->GetAdapters();
    for (dawn_native::Adapter adapter : adapters) {
        if (adapter.GetBackendType() == type) {
            return adapter.CreateDevice();
        }
    }
    return nullptr;
}

}   //namespace sk_app
