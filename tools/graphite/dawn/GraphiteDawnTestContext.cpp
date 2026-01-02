/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/dawn/GraphiteDawnTestContext.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/dawn/DawnBackendContext.h"
#include "include/gpu/graphite/dawn/DawnGraphiteTypes.h"
#include "include/private/base/SkOnce.h"
#include "src/gpu/graphite/ContextOptionsPriv.h"
#include "tools/gpu/ContextType.h"
#include "tools/graphite/TestOptions.h"
#include "tools/graphite/dawn/GraphiteDawnToggles.h"

#include "dawn/dawn_proc.h"

#define LOG_ADAPTER 0

namespace skiatest::graphite {

// TODO: http://crbug.com/dawn/2450 - Currently manually setting the device to null and calling
//       tick/process events one last time to ensure that the device is lost accordingly at
//       destruction. Once device lost is, by default, a spontaneous event, remove this.
DawnTestContext::~DawnTestContext() {
    fBackendContext.fDevice = nullptr;
    tick();
}

std::unique_ptr<GraphiteTestContext> DawnTestContext::Make(wgpu::BackendType backend) {
    static std::unique_ptr<dawn::native::Instance> sInstance;
    static SkOnce sOnce;

    // Creation of Instance is cheap but calling EnumerateAdapters can be expensive the first time,
    // but then the results are cached on the Instance object. So save the Instance here so we can
    // avoid the overhead of EnumerateAdapters on every test.
    sOnce([&] {
        DawnProcTable backendProcs = dawn::native::GetProcs();
        dawnProcSetProcs(&backendProcs);
        wgpu::InstanceDescriptor desc{};
        // need for WaitAny with timeout > 0
        static const auto kTimedWaitAny = wgpu::InstanceFeatureName::TimedWaitAny;
        desc.requiredFeatureCount = 1;
        desc.requiredFeatures = &kTimedWaitAny;
        wgpu::DawnTogglesDescriptor togglesDesc = GetInstanceToggles();
        desc.nextInChain = &togglesDesc;
        sInstance = std::make_unique<dawn::native::Instance>(&desc);
    });

    dawn::native::Adapter matchedAdaptor;

    wgpu::RequestAdapterOptions options;
    options.featureLevel =
            backend == wgpu::BackendType::OpenGL || backend == wgpu::BackendType::OpenGLES
                    ? wgpu::FeatureLevel::Compatibility
                    : wgpu::FeatureLevel::Core;
    wgpu::DawnTogglesDescriptor togglesDesc = GetAdapterToggles();
    options.nextInChain = &togglesDesc;
    std::vector<dawn::native::Adapter> adapters = sInstance->EnumerateAdapters(&options);
    SkASSERT(!adapters.empty());
    // Sort adapters by adapterType(DiscreteGPU, IntegratedGPU, CPU) and
    // backendType(WebGPU, D3D11, D3D12, Metal, Vulkan, OpenGL, OpenGLES).
    std::sort(
            adapters.begin(), adapters.end(), [](dawn::native::Adapter a, dawn::native::Adapter b) {
                wgpu::Adapter wgpuA = a.Get();
                wgpu::Adapter wgpuB = b.Get();
                wgpu::AdapterInfo infoA;
                wgpu::AdapterInfo infoB;
                wgpuA.GetInfo(&infoA);
                wgpuB.GetInfo(&infoB);
                return std::tuple(infoA.adapterType, infoA.backendType) <
                       std::tuple(infoB.adapterType, infoB.backendType);
            });

    for (const auto& adapter : adapters) {
        wgpu::Adapter wgpuAdapter = adapter.Get();
        wgpu::AdapterInfo props;
        wgpuAdapter.GetInfo(&props);
        if (backend == props.backendType) {
            matchedAdaptor = adapter;
            break;
        }
    }

    if (!matchedAdaptor) {
        return nullptr;
    }

#if LOG_ADAPTER
{
    wgpu::AdapterInfo debugInfo;
    wgpu::Adapter debugAdapter = matchedAdaptor.Get();
    debugAdapter.GetInfo(&debugInfo);
    SkDebugf("GPU: %s\nDriver: %s\n", debugInfo.device.data, debugInfo.description.data);
}
#endif

    std::vector<wgpu::FeatureName> features;
    wgpu::Adapter adapter = matchedAdaptor.Get();
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
    if (adapter.HasFeature(wgpu::FeatureName::TextureFormatsTier1)) {
        features.push_back(wgpu::FeatureName::TextureFormatsTier1);
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
    if (adapter.HasFeature(wgpu::FeatureName::ImplicitDeviceSynchronization)) {
        features.push_back(wgpu::FeatureName::ImplicitDeviceSynchronization);
    }

    wgpu::DeviceDescriptor desc;
    desc.requiredFeatureCount  = features.size();
    desc.requiredFeatures      = features.data();
    desc.nextInChain           = &togglesDesc;

    wgpu::Limits limits = {};
    adapter.GetLimits(&limits);
    desc.requiredLimits = &limits;

    desc.SetDeviceLostCallback(
            wgpu::CallbackMode::AllowSpontaneous,
            [](const wgpu::Device&, wgpu::DeviceLostReason reason, wgpu::StringView message) {
                if (reason != wgpu::DeviceLostReason::Destroyed) {
                    SK_ABORT("Device lost: %.*s\n", static_cast<int>(message.length), message.data);
                }
            });
    desc.SetUncapturedErrorCallback([](const wgpu::Device&, wgpu::ErrorType,
                                       wgpu::StringView message) {
        SkDebugf("Device error: %.*s\n", static_cast<int>(message.length), message.data);
    });

    wgpu::Device device = adapter.CreateDevice(&desc);
    SkASSERT(device);

    skgpu::graphite::DawnBackendContext backendContext;
    backendContext.fInstance = wgpu::Instance(sInstance->Get());
    backendContext.fDevice = device;
    backendContext.fQueue  = device.GetQueue();
    return std::unique_ptr<GraphiteTestContext>(new DawnTestContext(backendContext));
}

skgpu::ContextType DawnTestContext::contextType() {
    wgpu::AdapterInfo info;
    fBackendContext.fDevice.GetAdapter().GetInfo(&info);
    switch (info.backendType) {
        case wgpu::BackendType::D3D11:
            return skgpu::ContextType::kDawn_D3D11;

        case wgpu::BackendType::D3D12:
            return skgpu::ContextType::kDawn_D3D12;

        case wgpu::BackendType::Metal:
            return skgpu::ContextType::kDawn_Metal;

        case wgpu::BackendType::Vulkan:
            return skgpu::ContextType::kDawn_Vulkan;

        case wgpu::BackendType::OpenGL:
            return skgpu::ContextType::kDawn_OpenGL;

        case wgpu::BackendType::OpenGLES:
            return skgpu::ContextType::kDawn_OpenGLES;
        default:
            SK_ABORT("unexpected Dawn backend");
            return skgpu::ContextType::kMock;
    }
}

std::unique_ptr<skgpu::graphite::Context> DawnTestContext::makeContext(const TestOptions& options) {
    skgpu::graphite::ContextOptions revisedContextOptions(options.fContextOptions);
    skgpu::graphite::ContextOptionsPriv contextOptionsPriv;
    if (!options.fContextOptions.fOptionsPriv) {
        revisedContextOptions.fOptionsPriv = &contextOptionsPriv;
    }
    // Needed to make synchronous readPixels work
    revisedContextOptions.fOptionsPriv->fStoreContextRefInRecorder = true;

    auto backendContext = fBackendContext;
    if (options.fNeverYieldToWebGPU) {
        backendContext.fTick = nullptr;
    }

    return skgpu::graphite::ContextFactory::MakeDawn(backendContext, revisedContextOptions);
}

void DawnTestContext::tick() { fBackendContext.fTick(fBackendContext.fInstance); }

}  // namespace skiatest::graphite
