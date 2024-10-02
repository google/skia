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
#include "include/gpu/graphite/dawn/DawnTypes.h"
#include "include/gpu/graphite/dawn/DawnUtils.h"
#include "src/gpu/graphite/ContextOptionsPriv.h"
#include "tools/GpuToolUtils.h"
#include "tools/ToolUtils.h"

#include "dawn/dawn_proc.h"

namespace skwindow::internal {

GraphiteDawnWindowContext::GraphiteDawnWindowContext(const DisplayParams& params,
                                                     wgpu::TextureFormat surfaceFormat)
    : WindowContext(params)
    , fSurfaceFormat(surfaceFormat) {
    WGPUInstanceDescriptor desc{};
    // need for WaitAny with timeout > 0
    desc.features.timedWaitAnyEnable = true;
    fInstance = std::make_unique<dawn::native::Instance>(&desc);
}

void GraphiteDawnWindowContext::initializeContext(int width, int height) {
    SkASSERT(!fContext);

    fWidth = width;
    fHeight = height;

    if (!this->onInitializeContext())
        return;

    SkASSERT(fDevice);
    SkASSERT(fSurface);

    skgpu::graphite::DawnBackendContext backendContext;
    backendContext.fInstance = wgpu::Instance(fInstance->Get());
    backendContext.fDevice = fDevice;
    backendContext.fQueue = fDevice.GetQueue();
    // Needed to make synchronous readPixels work:
    fDisplayParams.fGraphiteTestOptions.fPriv.fStoreContextRefInRecorder = true;
    fGraphiteContext = skgpu::graphite::ContextFactory::MakeDawn(
            backendContext, fDisplayParams.fGraphiteTestOptions.fTestOptions.fContextOptions);
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
    fSurface = nullptr;
    fDevice = nullptr;
}

sk_sp<SkSurface> GraphiteDawnWindowContext::getBackbufferSurface() {
    wgpu::SurfaceTexture surfaceTexture;
    fSurface.GetCurrentTexture(&surfaceTexture);
    SkASSERT(surfaceTexture.texture);
    auto texture = surfaceTexture.texture;

    skgpu::graphite::DawnTextureInfo info(/*sampleCount=*/1,
                                          skgpu::Mipmapped::kNo,
                                          fSurfaceFormat,
                                          texture.GetUsage(),
                                          wgpu::TextureAspect::All);
    auto backendTex = skgpu::graphite::BackendTextures::MakeDawn(texture.Get());
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
    fSurface.Present();
}

void GraphiteDawnWindowContext::setDisplayParams(const DisplayParams& params) {
    this->destroyContext();
    fDisplayParams = params;
    this->initializeContext(fWidth, fHeight);
}

wgpu::Device GraphiteDawnWindowContext::createDevice(wgpu::BackendType type) {
    DawnProcTable backendProcs = dawn::native::GetProcs();
    dawnProcSetProcs(&backendProcs);

    static constexpr const char* kToggles[] = {
        "allow_unsafe_apis",  // Needed for dual-source blending, BufferMapExtendedUsages.
        "use_user_defined_labels_in_backend",
        // Robustness impacts performance and is always disabled when running Graphite in Chrome,
        // so this keeps Skia's tests operating closer to real-use behavior.
        "disable_robustness",
        // Must be last to correctly respond to `fUseTintIR` option.
        "use_tint_ir",
    };
    wgpu::DawnTogglesDescriptor togglesDesc;
    togglesDesc.enabledToggleCount  = std::size(kToggles) -
        (fDisplayParams.fGraphiteTestOptions.fTestOptions.fUseTintIR ? 0 : 1);
    togglesDesc.enabledToggles      = kToggles;

    wgpu::RequestAdapterOptions adapterOptions;
    adapterOptions.backendType = type;
    adapterOptions.compatibilityMode =
            type == wgpu::BackendType::OpenGL || type == wgpu::BackendType::OpenGLES;
    adapterOptions.nextInChain = &togglesDesc;

    std::vector<dawn::native::Adapter> adapters = fInstance->EnumerateAdapters(&adapterOptions);
    if (adapters.empty()) {
        return nullptr;
    }

    wgpu::Adapter adapter = adapters[0].Get();

    std::vector<wgpu::FeatureName> features;
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
    if (adapter.HasFeature(wgpu::FeatureName::DawnPartialLoadResolveTexture)) {
        features.push_back(wgpu::FeatureName::DawnPartialLoadResolveTexture);
    }

    wgpu::DeviceDescriptor deviceDescriptor;
    deviceDescriptor.requiredFeatures = features.data();
    deviceDescriptor.requiredFeatureCount = features.size();
    deviceDescriptor.nextInChain = &togglesDesc;
    deviceDescriptor.SetDeviceLostCallback(
            wgpu::CallbackMode::AllowSpontaneous,
            [](const wgpu::Device&, wgpu::DeviceLostReason reason, const char* message) {
                if (reason != wgpu::DeviceLostReason::Destroyed &&
                    reason != wgpu::DeviceLostReason::InstanceDropped) {
                    SK_ABORT("Device lost: %s\n", message);
                }
            });
    deviceDescriptor.SetUncapturedErrorCallback(
            [](const wgpu::Device&, wgpu::ErrorType, const char* message) {
                SkDebugf("Device error: %s\n", message);
                SkASSERT(false);
            });

    wgpu::DawnTogglesDescriptor deviceTogglesDesc;

    if (fDisplayParams.fGraphiteTestOptions.fTestOptions.fDisableTintSymbolRenaming) {
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

    return device;
}

void GraphiteDawnWindowContext::configureSurface() {
    SkASSERT(fDevice);
    SkASSERT(fSurface);

    wgpu::SurfaceConfiguration surfaceConfig;
    surfaceConfig.device = fDevice;
    surfaceConfig.format = fSurfaceFormat;
    surfaceConfig.usage = wgpu::TextureUsage::RenderAttachment |
                          wgpu::TextureUsage::TextureBinding |
                          wgpu::TextureUsage::CopySrc |
                          wgpu::TextureUsage::CopyDst;
    surfaceConfig.width = fWidth;
    surfaceConfig.height = fHeight;
    surfaceConfig.presentMode =
            fDisplayParams.fDisableVsync ? wgpu::PresentMode::Immediate : wgpu::PresentMode::Fifo;
    fSurface.Configure(&surfaceConfig);
}

}   //namespace skwindow::internal
