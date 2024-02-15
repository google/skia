/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/base/SkAssert.h"
#include "tools/gpu/BackendSurfaceFactory.h"
#include "tools/gpu/GrContextFactory.h"
#include "tools/testrunners/common/surface_manager/SurfaceManager.h"

#include <string>

class GaneshGLSurfaceManager : public SurfaceManager {
public:
    GaneshGLSurfaceManager(std::unique_ptr<sk_gpu_test::GrContextFactory> contextFactory,
                           sk_gpu_test::ContextInfo contextInfo,
                           GrDirectContext* context,
                           sk_sp<SkSurface> surface,
                           std::string config,
                           SkColorInfo colorInfo)
            : SurfaceManager(config, colorInfo, CpuOrGpu::kGPU)
            , fContextFactory(std::move(contextFactory))
            , fContextInfo(contextInfo)
            , fContext(context)
            , fSurface(surface) {}

    sk_sp<SkSurface> getSurface() override { return fSurface; }

    void flush() override { fContext->flushAndSubmit(fSurface.get(), GrSyncCpu::kYes); }

    sk_gpu_test::ContextInfo* getGaneshContextInfo() override { return &fContextInfo; }

private:
    // The GL context is destroyed when the context factory is destroyed. We prevent early
    // destruction of the context by grabbing a reference to the context factory. See the
    // GrContextFactory class documentation for details.
    std::unique_ptr<sk_gpu_test::GrContextFactory> fContextFactory;
    sk_gpu_test::ContextInfo fContextInfo;
    GrDirectContext* fContext;
    sk_sp<SkSurface> fSurface;
};

enum class SurfaceType { kDefault, kBackendTexture, kBackendRenderTarget };

std::unique_ptr<SurfaceManager> makeGLESSurfaceManager(
        std::string config,
        SurfaceOptions surfaceOptions,
        GrContextOptions grContextOptions,
        sk_gpu_test::GrContextFactory::ContextOverrides contextOverrides,
        SkColorInfo colorInfo,
        SurfaceType surfaceType,
        uint32_t surfaceFlags,
        int sampleCount) {
    if (surfaceOptions.modifyGrContextOptions) {
        surfaceOptions.modifyGrContextOptions(&grContextOptions);
    }

    // Based on
    // https://skia.googlesource.com/skia/+/8da85ea79d1ba2b3f32d25178eb21f2ebda83437/dm/DMSrcSink.cpp#1579.
    auto contextFactory = std::make_unique<sk_gpu_test::GrContextFactory>(grContextOptions);
    sk_gpu_test::ContextInfo contextInfo =
            contextFactory.get()->getContextInfo(skgpu::ContextType::kGLES, contextOverrides);
    GrDirectContext* context = contextInfo.directContext();
    SkASSERT_RELEASE(context);

    // Based on
    // https://skia.googlesource.com/skia/+/8da85ea79d1ba2b3f32d25178eb21f2ebda83437/dm/DMSrcSink.cpp#1524.
    SkImageInfo imageInfo = SkImageInfo::Make({surfaceOptions.width, surfaceOptions.height}, colorInfo);
    SkSurfaceProps surfaceProps(surfaceFlags, kRGB_H_SkPixelGeometry);
    sk_sp<SkSurface> surface;
    switch (surfaceType) {
        default:
        case SurfaceType::kDefault:
            surface = SkSurfaces::RenderTarget(
                    context, skgpu::Budgeted::kNo, imageInfo, sampleCount, &surfaceProps);
            break;

        case SurfaceType::kBackendTexture:
            surface = sk_gpu_test::MakeBackendTextureSurface(context,
                                                             imageInfo,
                                                             kTopLeft_GrSurfaceOrigin,
                                                             sampleCount,
                                                             skgpu::Mipmapped::kNo,
                                                             skgpu::Protected::kNo,
                                                             &surfaceProps);
            break;

        case SurfaceType::kBackendRenderTarget:
            surface = sk_gpu_test::MakeBackendRenderTargetSurface(context,
                                                                  imageInfo,
                                                                  kBottomLeft_GrSurfaceOrigin,
                                                                  sampleCount,
                                                                  skgpu::Protected::kNo,
                                                                  &surfaceProps);
            break;
    }
    SkASSERT_RELEASE(surface);

    return std::make_unique<GaneshGLSurfaceManager>(
            std::move(contextFactory), contextInfo, context, surface, config, colorInfo);
}

// Based on the configurations defined here[1], the configuration parsing logic here[2], and the
// sink selection logic here[3].
//
// [1]
// https://skia.googlesource.com/skia/+/8da85ea79d1ba2b3f32d25178eb21f2ebda83437/tools/flags/CommonFlagsConfig.cpp#40
// [2]
// https://skia.googlesource.com/skia/+/8da85ea79d1ba2b3f32d25178eb21f2ebda83437/tools/flags/CommonFlagsConfig.cpp#610
// [3]
// https://skia.googlesource.com/skia/+/8da85ea79d1ba2b3f32d25178eb21f2ebda83437/dm/DM.cpp#1017
std::unique_ptr<SurfaceManager> SurfaceManager::FromConfig(std::string config,
                                                           SurfaceOptions surfaceOptions) {
    if (config == "gles") {
        return makeGLESSurfaceManager(
                config,
                surfaceOptions,
                GrContextOptions(),
                sk_gpu_test::GrContextFactory::ContextOverrides::kNone,
                SkColorInfo(kRGBA_8888_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGB()),
                SurfaceType::kDefault,
                SkSurfaceProps::kDefault_Flag,
                /* sampleCount= */ 1);
    }
    if (config == "gles_msaa4") {
        return makeGLESSurfaceManager(
                config,
                surfaceOptions,
                GrContextOptions(),
                sk_gpu_test::GrContextFactory::ContextOverrides::kNone,
                SkColorInfo(kRGBA_8888_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGB()),
                SurfaceType::kDefault,
                SkSurfaceProps::kDefault_Flag,
                /* sampleCount= */ 4);
    }
    if (config == "gles_msaa8") {
        return makeGLESSurfaceManager(
                config,
                surfaceOptions,
                GrContextOptions(),
                sk_gpu_test::GrContextFactory::ContextOverrides::kNone,
                SkColorInfo(kRGBA_8888_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGB()),
                SurfaceType::kDefault,
                SkSurfaceProps::kDefault_Flag,
                /* sampleCount= */ 8);
    }
    if (config == "gles_msaa8_noReduceOpsTaskSplitting") {
        GrContextOptions grContextOptions;
        grContextOptions.fReduceOpsTaskSplitting = GrContextOptions::Enable::kNo;
        return makeGLESSurfaceManager(
                config,
                surfaceOptions,
                grContextOptions,
                sk_gpu_test::GrContextFactory::ContextOverrides::kNone,
                SkColorInfo(kRGBA_8888_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGB()),
                SurfaceType::kDefault,
                SkSurfaceProps::kDefault_Flag,
                /* sampleCount= */ 8);
    }
    return nullptr;
}
