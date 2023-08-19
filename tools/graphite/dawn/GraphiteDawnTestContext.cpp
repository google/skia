/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/dawn/GraphiteDawnTestContext.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/dawn/DawnTypes.h"
#include "include/gpu/graphite/dawn/DawnUtils.h"
#include "include/private/base/SkOnce.h"

#include "dawn/dawn_proc.h"

#define LOG_ADAPTER 0

namespace skiatest::graphite {

std::unique_ptr<GraphiteTestContext> DawnTestContext::Make(std::optional<wgpu::BackendType> backend) {
    static std::unique_ptr<dawn::native::Instance> gInstance;
    static dawn::native::Adapter gAdapter;
    static SkOnce gOnce;

    gOnce([&]{
        DawnProcTable backendProcs = dawn::native::GetProcs();
        dawnProcSetProcs(&backendProcs);

        gInstance = std::make_unique<dawn::native::Instance>();
        std::vector<dawn::native::Adapter> adapters = gInstance->EnumerateAdapters();
        SkASSERT(!adapters.empty());
        // Sort adapters by adapterType(DiscreteGPU, IntegratedGPU, CPU) and
        // backendType(WebGPU, D3D11, D3D12, Metal, Vulkan, OpenGL, OpenGLES).
        std::sort(
            adapters.begin(), adapters.end(),
            [](dawn::native::Adapter a, dawn::native::Adapter b) {
                wgpu::AdapterProperties propA;
                wgpu::AdapterProperties propB;
                a.GetProperties(&propA);
                b.GetProperties(&propB);
                return std::tuple(propA.adapterType, propA.backendType) <
                       std::tuple(propB.adapterType, propB.backendType);
            });

        for (auto adapter : adapters) {
            wgpu::AdapterProperties props;
            adapter.GetProperties(&props);
            if (backend.has_value() && backend.value() == props.backendType) {
                gAdapter = adapter;
                break;
            }
            // Skip Dawn D3D11 backend for now.
            if (props.backendType != wgpu::BackendType::D3D11) {
                gAdapter = adapter;
                break;
            }
        }
        SkASSERT(gAdapter);

#if LOG_ADAPTER
        wgpu::AdapterProperties properties;
        gAdapter.GetProperties(&properties);
        SkDebugf("GPU: %s\nDriver: %s\n", properties.name, properties.driverDescription);
#endif
    });

    std::vector<wgpu::FeatureName> features;
    wgpu::Adapter adapter = gAdapter.Get();
    if (adapter.HasFeature(wgpu::FeatureName::MSAARenderToSingleSampled)) {
        features.push_back(wgpu::FeatureName::MSAARenderToSingleSampled);
    }
    if (adapter.HasFeature(wgpu::FeatureName::TransientAttachments)) {
        features.push_back(wgpu::FeatureName::TransientAttachments);
    }

    wgpu::DeviceDescriptor desc;
#ifdef WGPU_BREAKING_CHANGE_COUNT_RENAME
    desc.requiredFeatureCount = features.size();
#else
    desc.requiredFeaturesCount = features.size();
#endif
    desc.requiredFeatures      = features.data();

#if !defined(SK_DEBUG)
        wgpu::DawnTogglesDescriptor deviceTogglesDesc;
        std::array<const char*, 1> toggles = {
            "skip_validation",
        };
#ifdef WGPU_BREAKING_CHANGE_COUNT_RENAME
        deviceTogglesDesc.enabledToggleCount = toggles.size();
#else
        deviceTogglesDesc.enabledTogglesCount = toggles.size();
#endif
        deviceTogglesDesc.enabledToggles      = toggles.data();
        desc.nextInChain                      = &deviceTogglesDesc;
#endif
    auto device = wgpu::Device::Acquire(gAdapter.CreateDevice(&desc));
    SkASSERT(device);
    device.SetUncapturedErrorCallback(
            [](WGPUErrorType type, const char* message, void*) {
                SkDebugf("Device error: %s\n", message);
            },
            0);
    device.SetDeviceLostCallback(
            [](WGPUDeviceLostReason reason, const char* message, void*) {
                if (reason != WGPUDeviceLostReason_Destroyed) {
                    SK_ABORT("Device lost: %s\n", message);
                }
            },
            0);

    skgpu::graphite::DawnBackendContext backendContext;
    backendContext.fDevice = device;
    backendContext.fQueue  = device.GetQueue();
    return std::unique_ptr<GraphiteTestContext>(new DawnTestContext(backendContext));
}

std::unique_ptr<skgpu::graphite::Context> DawnTestContext::makeContext(
        const skgpu::graphite::ContextOptions& options) {
    skgpu::graphite::ContextOptions revisedOptions(options);
    revisedOptions.fStoreContextRefInRecorder = true; // Needed to make synchronous readPixels work

    return skgpu::graphite::ContextFactory::MakeDawn(fBackendContext, revisedOptions);
}

}  // namespace skiatest::graphite
