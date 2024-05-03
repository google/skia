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
    , fSwapChainFormat(swapChainFormat) {
    WGPUInstanceDescriptor desc{};
    // need for WaitAny with timeout > 0
    desc.features.timedWaitAnyEnable = true;
    fInstance = std::make_unique<dawn::native::Instance>(&desc);
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
    backendContext.fInstance = wgpu::Instance(fInstance->Get());
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
    this->snapRecordingAndSubmit();
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

    static constexpr const char* kAdapterToggles[] = {
        "allow_unsafe_apis",  // Needed for dual-source blending, BufferMapExtendedUsages.
        "use_user_defined_labels_in_backend",
    };
    wgpu::DawnTogglesDescriptor adapterTogglesDesc;
    adapterTogglesDesc.enabledToggleCount  = std::size(kAdapterToggles);
    adapterTogglesDesc.enabledToggles      = kAdapterToggles;

    wgpu::RequestAdapterOptions adapterOptions;
    adapterOptions.backendType = type;
    adapterOptions.nextInChain = &adapterTogglesDesc;

    std::vector<dawn::native::Adapter> adapters = fInstance->EnumerateAdapters(&adapterOptions);
    if (adapters.empty()) {
        return nullptr;
    }

    wgpu::Adapter adapter = adapters[0].Get();

    std::vector<wgpu::FeatureName> features;
    features.push_back(wgpu::FeatureName::SurfaceCapabilities);
    if (adapter.HasFeature(wgpu::FeatureName::MSAARenderToSingleSampled)) {
        features.push_back(wgpu::FeatureName::MSAARenderToSingleSampled);
    }
    if (adapter.HasFeature(wgpu::FeatureName::TransientAttachments)) {
        features.push_back(wgpu::FeatureName::TransientAttachments);
    }
    if (adapter.HasFeature(wgpu::FeatureName::Unorm16TextureFormats)) {
        features.push_back(wgpu::FeatureName::Unorm16TextureFormats);
    }
    if (adapter.HasFeature(wgpu::FeatureName::DualSourceBlending)) {
        features.push_back(wgpu::FeatureName::DualSourceBlending);
    }
    if (adapter.HasFeature(wgpu::FeatureName::FramebufferFetch)) {
        features.push_back(wgpu::FeatureName::FramebufferFetch);
    }
    if (adapter.HasFeature(wgpu::FeatureName::BufferMapExtendedUsages)) {
        features.push_back(wgpu::FeatureName::BufferMapExtendedUsages);
    }
    if (adapter.HasFeature(wgpu::FeatureName::TextureCompressionETC2)) {
        features.push_back(wgpu::FeatureName::TextureCompressionETC2);
    }
    if (adapter.HasFeature(wgpu::FeatureName::TextureCompressionBC)) {
        features.push_back(wgpu::FeatureName::TextureCompressionBC);
    }
    if (adapter.HasFeature(wgpu::FeatureName::R8UnormStorage)) {
        features.push_back(wgpu::FeatureName::R8UnormStorage);
    }
    if (adapter.HasFeature(wgpu::FeatureName::DawnLoadResolveTexture)) {
        features.push_back(wgpu::FeatureName::DawnLoadResolveTexture);
    }

    wgpu::DeviceDescriptor deviceDescriptor;
    deviceDescriptor.requiredFeatures = features.data();
    deviceDescriptor.requiredFeatureCount = features.size();
    deviceDescriptor.deviceLostCallbackInfo.callback =
        [](WGPUDeviceImpl *const *, WGPUDeviceLostReason reason, const char* message, void*) {
            if (reason != WGPUDeviceLostReason_Destroyed) {
                SK_ABORT("Device lost: %s\n", message);
            }
        };

    wgpu::DawnTogglesDescriptor deviceTogglesDesc;

    if (fDisplayParams.fDisableTintSymbolRenaming) {
        static constexpr const char* kOptionalDeviceToggles[] = {
            "disable_symbol_renaming",
        };
        deviceTogglesDesc.enabledToggleCount = std::size(kOptionalDeviceToggles);
        deviceTogglesDesc.enabledToggles     = kOptionalDeviceToggles;

        // Insert the toggles descriptor ahead of any existing entries in the chain that might have
        // been added above.
        deviceTogglesDesc.nextInChain = deviceDescriptor.nextInChain;
        deviceDescriptor.nextInChain  = &deviceTogglesDesc;
    }

    auto device = adapter.CreateDevice(&deviceDescriptor);
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
