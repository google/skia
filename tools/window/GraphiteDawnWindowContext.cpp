/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/window/GraphiteDawnWindowContext.h"

#include "include/core/SkSurface.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/dawn/DawnBackendContext.h"
#include "include/gpu/graphite/dawn/DawnUtils.h"
#include "include/private/gpu/graphite/ContextOptionsPriv.h"
#include "tools/ToolUtils.h"
#include "tools/GpuToolUtils.h"

#include "dawn/dawn_proc.h"

namespace skwindow::internal {

GraphiteDawnWindowContext::GraphiteDawnWindowContext(const DisplayParams& params,
                                                     wgpu::TextureFormat swapChainFormat)
    : WindowContext(params)
    , fSwapChainFormat(swapChainFormat)
    , fInstance(std::make_unique<dawn::native::Instance>()) {
}

void GraphiteDawnWindowContext::initializeContext(int width, int height) {
    SkASSERT(!fContext);

    fWidth = width;
    fHeight = height;

    if (!onInitializeContext())
        return;

    SkASSERT(fDevice);
    SkASSERT(fSurface);
    SkASSERT(fSwapChain);

    skgpu::graphite::DawnBackendContext backendContext;
    backendContext.fDevice = fDevice;
    backendContext.fQueue = fDevice.GetQueue();
    // Needed to make synchronous readPixels work:
    fDisplayParams.fGraphiteContextOptions.fPriv.fStoreContextRefInRecorder = true;
    fGraphiteContext = skgpu::graphite::ContextFactory::MakeDawn(
            backendContext, fDisplayParams.fGraphiteContextOptions.fOptions);
    if (!fGraphiteContext) {
        SkASSERT(false);
        return;
    }

    fGraphiteRecorder = fGraphiteContext->makeRecorder(ToolUtils::CreateTestingRecorderOptions());
    SkASSERT(fGraphiteRecorder);
}

GraphiteDawnWindowContext::~GraphiteDawnWindowContext() = default;

void GraphiteDawnWindowContext::destroyContext() {
    if (!fDevice.Get()) {
        return;
    }

    this->onDestroyContext();

    fGraphiteRecorder = nullptr;
    fGraphiteContext = nullptr;
    fSwapChain = nullptr;
    fSurface = nullptr;
    fDevice = nullptr;
}

sk_sp<SkSurface> GraphiteDawnWindowContext::getBackbufferSurface() {
    auto texture = fSwapChain.GetCurrentTexture();
    skgpu::graphite::DawnTextureInfo info(/*sampleCount=*/1,
                                          skgpu::Mipmapped::kNo,
                                          fSwapChainFormat,
                                          texture.GetUsage(),
                                          wgpu::TextureAspect::All);
    skgpu::graphite::BackendTexture backendTex(texture.Get());
    SkASSERT(this->graphiteRecorder());
    auto surface = SkSurfaces::WrapBackendTexture(this->graphiteRecorder(),
                                                  backendTex,
                                                  kBGRA_8888_SkColorType,
                                                  fDisplayParams.fColorSpace,
                                                  &fDisplayParams.fSurfaceProps);
    SkASSERT(surface);
    return surface;
}

void GraphiteDawnWindowContext::onSwapBuffers() {
    if (fGraphiteContext) {
        SkASSERT(fGraphiteRecorder);
        std::unique_ptr<skgpu::graphite::Recording> recording = fGraphiteRecorder->snap();
        if (recording) {
            skgpu::graphite::InsertRecordingInfo info;
            info.fRecording = recording.get();
            fGraphiteContext->insertRecording(info);
            fGraphiteContext->submit(skgpu::graphite::SyncToCpu::kNo);
        }
    }

    fSwapChain.Present();
}

void GraphiteDawnWindowContext::setDisplayParams(const DisplayParams& params) {
    this->destroyContext();
    fDisplayParams = params;
    this->initializeContext(fWidth, fHeight);
}

wgpu::Device GraphiteDawnWindowContext::createDevice(wgpu::BackendType type) {
    DawnProcTable backendProcs = dawn::native::GetProcs();
    dawnProcSetProcs(&backendProcs);

    wgpu::RequestAdapterOptions adapterOptions;
    adapterOptions.backendType = type;

    std::vector<dawn::native::Adapter> adapters = fInstance->EnumerateAdapters(&adapterOptions);
    if (adapters.empty()) {
        return nullptr;
    }

    std::vector<wgpu::FeatureName> requiredFeatures;
    requiredFeatures.push_back(wgpu::FeatureName::SurfaceCapabilities);

    wgpu::DeviceDescriptor deviceDescriptor;
    deviceDescriptor.requiredFeatures = requiredFeatures.data();
    deviceDescriptor.requiredFeatureCount = requiredFeatures.size();

    auto device = wgpu::Device::Acquire(adapters[0].CreateDevice(&deviceDescriptor));
    if (!device) {
        return nullptr;
    }

    device.SetUncapturedErrorCallback(
            [](WGPUErrorType type, const char* message, void*) {
                SkDebugf("Device error: %s\n", message);
                SkASSERT(false);
            },
            nullptr);
    return device;
}

wgpu::SwapChain GraphiteDawnWindowContext::createSwapChain() {
    wgpu::SwapChainDescriptor swapChainDesc;
    swapChainDesc.usage = wgpu::TextureUsage::RenderAttachment |
                          wgpu::TextureUsage::TextureBinding |
                          wgpu::TextureUsage::CopySrc |
                          wgpu::TextureUsage::CopyDst;
    swapChainDesc.format = fSwapChainFormat;
    swapChainDesc.width = fWidth;
    swapChainDesc.height = fHeight;
    swapChainDesc.presentMode =
            fDisplayParams.fDisableVsync ? wgpu::PresentMode::Immediate : wgpu::PresentMode::Fifo;
    auto swapChain = fDevice.CreateSwapChain(fSurface, &swapChainDesc);
    SkASSERT(swapChain);
    return swapChain;
}

}   //namespace skwindow::internal
