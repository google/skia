/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/base/SkAutoMalloc.h"
#include "tools/window/DawnWindowContext.h"

#include "dawn/dawn_proc.h"

static wgpu::TextureUsage kUsage = wgpu::TextureUsage::RenderAttachment |
                                   wgpu::TextureUsage::CopySrc;

static void PrintDeviceError(WGPUErrorType, const char* message, void*) {
    printf("Device error: %s\n", message);
    SkASSERT(false);
}

static wgpu::SwapChainDescriptor CreateSwapChainDesc(int width,
                                                     int height,
                                                     wgpu::TextureFormat format) {
    wgpu::SwapChainDescriptor desc;
    desc.format = format;
    desc.usage = kUsage;
    desc.width = width;
    desc.height = height;
    desc.presentMode = wgpu::PresentMode::Mailbox;
    return desc;
}

namespace skwindow::internal {

DawnWindowContext::DawnWindowContext(const DisplayParams& params,
                                     wgpu::TextureFormat swapChainFormat)
    : WindowContext(params)
    , fSwapChainFormat(swapChainFormat)
    , fInstance(std::make_unique<dawn::native::Instance>()) {
}

void DawnWindowContext::initializeContext(int width, int height) {
    SkASSERT(!fContext);

    fWidth = width;
    fHeight = height;
    fDevice = onInitializeContext();

    fContext = GrDirectContext::MakeDawn(fDevice, fDisplayParams.fGrContextOptions);
    if (!fContext) {
        return;
    }

    wgpu::SwapChainDescriptor swapChainDesc =
        CreateSwapChainDesc(width, height, fSwapChainFormat);
    fSwapChain = fDevice.CreateSwapChain(fDawnSurface, &swapChainDesc);
    if (!fSwapChain) {
        fContext.reset();
        return;
    }

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
    GrDawnRenderTargetInfo rtInfo;
    rtInfo.fTextureView = fSwapChain.GetCurrentTextureView();
    rtInfo.fFormat = fSwapChainFormat;
    rtInfo.fLevelCount = 1; // FIXME
    GrBackendRenderTarget backendRenderTarget(fWidth, fHeight, fDisplayParams.fMSAASampleCount, 8,
                                              rtInfo);
    fSurface = SkSurfaces::WrapBackendRenderTarget(fContext.get(),
                                                   backendRenderTarget,
                                                   this->getRTOrigin(),
                                                   fDisplayParams.fColorType,
                                                   fDisplayParams.fColorSpace,
                                                   &fDisplayParams.fSurfaceProps);
    return fSurface;
}

void DawnWindowContext::onSwapBuffers() {
    fSwapChain.Present();
}

void DawnWindowContext::resize(int w, int h) {
    fWidth = w;
    fHeight = h;
    wgpu::SwapChainDescriptor swapChainDesc =
        CreateSwapChainDesc(w, h, fSwapChainFormat);
    fSwapChain = fDevice.CreateSwapChain(fDawnSurface, &swapChainDesc);
    if (!fSwapChain) {
        fContext.reset();
        return;
    }
}

void DawnWindowContext::setDisplayParams(const DisplayParams& params) {
    fDisplayParams = params;
}

wgpu::Device DawnWindowContext::createDevice(wgpu::BackendType type) {
    fInstance->DiscoverDefaultPhysicalDevices();
    DawnProcTable backendProcs = dawn::native::GetProcs();
    dawnProcSetProcs(&backendProcs);

    std::vector<dawn::native::Adapter> adapters = fInstance->GetAdapters();
    for (dawn::native::Adapter adapter : adapters) {
        wgpu::AdapterProperties properties;
        adapter.GetProperties(&properties);
        if (properties.backendType == type) {
            return adapter.CreateDevice();
        }
    }
    return nullptr;
}

}   //namespace skwindow::internal
