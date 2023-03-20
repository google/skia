/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/sk_app/GraphiteDawnWindowContext.h"

#include "include/core/SkSurface.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/dawn/DawnBackendContext.h"
#include "include/gpu/graphite/dawn/DawnUtils.h"
#include "tools/ToolUtils.h"

#include "dawn/dawn_proc.h"

namespace sk_app {

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
    skgpu::graphite::ContextOptions contextOptions;
    contextOptions.fStoreContextRefInRecorder = true; // Needed to make synchronous readPixels work
    fGraphiteContext = skgpu::graphite::ContextFactory::MakeDawn(backendContext,
                                                                 contextOptions);
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
    fInstance = nullptr;
}

sk_sp<SkSurface> GraphiteDawnWindowContext::getBackbufferSurface() {
    auto textureView = fSwapChain.GetCurrentTextureView();
    skgpu::graphite::DawnTextureInfo info(/*sampleCount=*/1,
                                          skgpu::Mipmapped::kNo,
                                          fSwapChainFormat,
                                          kTextureUsage);
    skgpu::graphite::BackendTexture backendTex(this->dimensions(),
                                               info,
                                               std::move(textureView));
    SkASSERT(this->graphiteRecorder());
    auto surface = SkSurface::MakeGraphiteFromBackendTexture(
        this->graphiteRecorder(),
        backendTex,
        kBGRA_8888_SkColorType,
        fDisplayParams.fColorSpace,
        &fDisplayParams.fSurfaceProps);
    SkASSERT(surface);
    return surface;
}

void GraphiteDawnWindowContext::swapBuffers() {
    // This chunk of code should not be in this class but higher up either in Window or
    // WindowContext
    std::unique_ptr<skgpu::graphite::Recording> recording = fGraphiteRecorder->snap();
    if (recording) {
        skgpu::graphite::InsertRecordingInfo info;
        info.fRecording = recording.get();
        fGraphiteContext->insertRecording(info);
        fGraphiteContext->submit(skgpu::graphite::SyncToCpu::kNo);
    }

    fSwapChain.Present();
    this->onSwapBuffers();
}

void GraphiteDawnWindowContext::setDisplayParams(const DisplayParams& params) {
    fDisplayParams = params;
}

wgpu::Device GraphiteDawnWindowContext::createDevice(wgpu::BackendType type) {
    fInstance->DiscoverDefaultAdapters();
    DawnProcTable backendProcs = dawn::native::GetProcs();
    dawnProcSetProcs(&backendProcs);

    std::vector<dawn::native::Adapter> adapters = fInstance->GetAdapters();
    for (dawn::native::Adapter adapter : adapters) {
        wgpu::AdapterProperties properties;
        adapter.GetProperties(&properties);
        if (properties.backendType != type) {
            continue;
        }
        auto device = wgpu::Device::Acquire(adapter.CreateDevice());
        device.SetUncapturedErrorCallback(
                [](WGPUErrorType type, const char* message, void*) {
                    SkDebugf("Device error: %s\n", message);
                    SkASSERT(false);
                },
                0);
        return device;
    }
    return nullptr;
}

wgpu::SwapChain GraphiteDawnWindowContext::createSwapChain() {
    wgpu::SwapChainDescriptor swapChainDesc;
    swapChainDesc.usage = kTextureUsage;
    swapChainDesc.format = fSwapChainFormat;
    swapChainDesc.width = fWidth;
    swapChainDesc.height = fHeight;
    swapChainDesc.presentMode = wgpu::PresentMode::Fifo;
    swapChainDesc.implementation = 0;
    auto swapChain = fDevice.CreateSwapChain(fSurface, &swapChainDesc);
    SkASSERT(swapChain);
    return swapChain;
}

}   //namespace sk_app
