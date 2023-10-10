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
            : SurfaceManager(config, colorInfo)
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

std::unique_ptr<SurfaceManager> makeGLESSurfaceManager(std::string config,
                                                       SurfaceOptions surfaceOptions,
                                                       GrContextOptions grContextOptions) {
    if (surfaceOptions.modifyGrContextOptions) {
        surfaceOptions.modifyGrContextOptions(&grContextOptions);
    }
    auto contextFactory = std::make_unique<sk_gpu_test::GrContextFactory>(grContextOptions);
    sk_gpu_test::ContextInfo contextInfo = contextFactory.get()->getContextInfo(
            skgpu::ContextType::kGLES, sk_gpu_test::GrContextFactory::ContextOverrides::kNone);
    GrDirectContext* context = contextInfo.directContext();
    SkASSERT_RELEASE(context);

    SkColorInfo colorInfo(kRGBA_8888_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGB());
    SkImageInfo info = SkImageInfo::Make({surfaceOptions.width, surfaceOptions.height}, colorInfo);
    SkSurfaceProps props(/* flags= */ 0, kRGB_H_SkPixelGeometry);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(
            context, skgpu::Budgeted::kNo, info, /* sampleCount= */ 1, &props);
    SkASSERT_RELEASE(surface);

    return std::make_unique<GaneshGLSurfaceManager>(
            std::move(contextFactory), contextInfo, context, surface, config, colorInfo);
}

std::unique_ptr<SurfaceManager> SurfaceManager::FromConfig(std::string config,
                                                           SurfaceOptions surfaceOptions) {
    if (config == "gles") {
        return makeGLESSurfaceManager(config, surfaceOptions, GrContextOptions());
    }
    if (config == "gles_msaa4") {
        GrContextOptions grContextOptions;
        grContextOptions.fInternalMultisampleCount = 4;
        return makeGLESSurfaceManager(config, surfaceOptions, grContextOptions);
    }
    if (config == "gles_msaa8") {
        GrContextOptions grContextOptions;
        grContextOptions.fInternalMultisampleCount = 8;
        return makeGLESSurfaceManager(config, surfaceOptions, grContextOptions);
    }
    if (config == "gles_msaa8_noReduceOpsTaskSplitting") {
        GrContextOptions grContextOptions;
        grContextOptions.fInternalMultisampleCount = 8;
        grContextOptions.fReduceOpsTaskSplitting = GrContextOptions::Enable::kNo;
        return makeGLESSurfaceManager(config, surfaceOptions, grContextOptions);
    }
    return nullptr;
}
