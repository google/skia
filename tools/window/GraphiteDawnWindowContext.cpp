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
#include "include/gpu/graphite/dawn/DawnGraphiteTypes.h"
#include "src/gpu/graphite/ContextOptionsPriv.h"
#include "tools/ToolUtils.h"
#include "tools/graphite/GraphiteToolUtils.h"
#include "tools/graphite/TestOptions.h"
#include "tools/window/GraphiteDisplayParams.h"

#include "dawn/dawn_proc.h"

namespace skwindow::internal {

namespace {
    SkColorType ToSkColorType(wgpu::TextureFormat format) {
        if (format == wgpu::TextureFormat::RGBA8Unorm) {
            return kRGBA_8888_SkColorType;
        } else {
            return kBGRA_8888_SkColorType;
        }
    }
}
GraphiteDawnWindowContext::GraphiteDawnWindowContext(std::unique_ptr<const DisplayParams> params,
                                                     wgpu::TextureFormat surfaceFormat)
        : WindowContext(std::move(params)), fSurfaceFormat(surfaceFormat) {
    wgpu::InstanceDescriptor desc{};
    // need for WaitAny with timeout > 0
    static const auto kTimedWaitAny = wgpu::InstanceFeatureName::TimedWaitAny;
    desc.requiredFeatureCount = 1;
    desc.requiredFeatures = &kTimedWaitAny;
    fInstance = std::make_unique<dawn::native::Instance>(&desc);
}

void GraphiteDawnWindowContext::initializeContext(int width, int height) {
#if defined(SK_GANESH)
    SkASSERT(!fContext);
#endif

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

    SkASSERT(fDisplayParams->graphiteTestOptions());
    skwindow::GraphiteTestOptions opts = *fDisplayParams->graphiteTestOptions();

    // Needed to make synchronous readPixels work:
    opts.fPriv.fStoreContextRefInRecorder = true;
    fDisplayParams =
            GraphiteDisplayParamsBuilder(fDisplayParams.get()).graphiteTestOptions(opts).detach();

    fGraphiteContext = skgpu::graphite::ContextFactory::MakeDawn(backendContext,
                                                                 opts.fTestOptions.fContextOptions);
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
                                                  ToSkColorType(fSurfaceFormat),
                                                  fDisplayParams->colorSpace(),
                                                  &fDisplayParams->surfaceProps());
    SkASSERT(surface);
    return surface;
}

void GraphiteDawnWindowContext::onSwapBuffers() {
    this->submitToGpu();
    fSurface.Present();
}

void GraphiteDawnWindowContext::setDisplayParams(std::unique_ptr<const DisplayParams> params) {
    this->destroyContext();
    fDisplayParams = std::move(params);
    this->initializeContext(fWidth, fHeight);
}

wgpu::Device GraphiteDawnWindowContext::createDevice(wgpu::BackendType type) {
    DawnProcTable backendProcs = dawn::native::GetProcs();
    dawnProcSetProcs(&backendProcs);

    static constexpr const char* kToggles[] = {
#if defined(SK_DEBUG)
            // Setting labels on backend objects has performance overhead.
            "use_user_defined_labels_in_backend",
#else
            "skip_validation",
#endif
            "disable_lazy_clear_for_mapped_at_creation_buffer",  // matches Chromes toggles
            "allow_unsafe_apis",  // Needed for dual-source blending, BufferMapExtendedUsages.
            // Robustness impacts performance and is always disabled when running Graphite in
            // Chrome, so this keeps Skia's tests operating closer to real-use behavior.
            "disable_robustness",
    };
    wgpu::DawnTogglesDescriptor togglesDesc;
    togglesDesc.enabledToggleCount  = std::size(kToggles);
    togglesDesc.enabledToggles      = kToggles;

    wgpu::RequestAdapterOptions adapterOptions;
    adapterOptions.backendType = type;
    adapterOptions.featureLevel =
            type == wgpu::BackendType::OpenGL || type == wgpu::BackendType::OpenGLES
                    ? wgpu::FeatureLevel::Compatibility
                    : wgpu::FeatureLevel::Core;
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
    if (adapter.HasFeature(wgpu::FeatureName::TimestampQuery)) {
        features.push_back(wgpu::FeatureName::TimestampQuery);
    }
    if (adapter.HasFeature(wgpu::FeatureName::DawnTexelCopyBufferRowAlignment)) {
        features.push_back(wgpu::FeatureName::DawnTexelCopyBufferRowAlignment);
    }

    wgpu::DeviceDescriptor deviceDescriptor;
    deviceDescriptor.requiredFeatures = features.data();
    deviceDescriptor.requiredFeatureCount = features.size();

    wgpu::Limits limits = {};
    adapter.GetLimits(&limits);
    deviceDescriptor.requiredLimits = &limits;

    deviceDescriptor.nextInChain = &togglesDesc;
    deviceDescriptor.SetDeviceLostCallback(
            wgpu::CallbackMode::AllowSpontaneous,
            [](const wgpu::Device&, wgpu::DeviceLostReason reason, wgpu::StringView message) {
                if (reason == wgpu::DeviceLostReason::Unknown ||
                    reason == wgpu::DeviceLostReason::FailedCreation) {
                    SK_ABORT("Device lost: %.*s\n", static_cast<int>(message.length), message.data);
                }
            });
    deviceDescriptor.SetUncapturedErrorCallback(
            [](const wgpu::Device&, wgpu::ErrorType, wgpu::StringView message) {
                SkDebugf("Device error: %.*s\n", static_cast<int>(message.length), message.data);
                SkASSERT(false);
            });

    wgpu::DawnTogglesDescriptor deviceTogglesDesc;

    if (fDisplayParams->graphiteTestOptions()->fTestOptions.fDisableTintSymbolRenaming) {
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
            fDisplayParams->disableVsync() ? wgpu::PresentMode::Immediate : wgpu::PresentMode::Fifo;
    fSurface.Configure(&surfaceConfig);
}

}   //namespace skwindow::internal
