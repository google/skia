/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/window/GraphiteNativeMetalWindowContext.h"

#include "include/core/SkSurface.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/mtl/MtlBackendContext.h"
#include "include/gpu/graphite/mtl/MtlGraphiteTypesUtils.h"
#include "include/gpu/graphite/mtl/MtlGraphiteUtils.h"
#include "src/base/SkMathPriv.h"
#include "src/gpu/graphite/ContextOptionsPriv.h"
#include "tools/graphite/GraphiteToolUtils.h"
#include "tools/graphite/TestOptions.h"
#include "tools/window/GraphiteDisplayParams.h"

using skwindow::DisplayParams;
using skwindow::internal::GraphiteMetalWindowContext;

namespace skwindow::internal {

GraphiteMetalWindowContext::GraphiteMetalWindowContext(std::unique_ptr<const DisplayParams> params)
        : WindowContext(DisplayParamsBuilder(params.get()).roundUpMSAA().build())
        , fValid(false)
        , fDrawableHandle(nil) {}

void GraphiteMetalWindowContext::initializeContext() {
    SkASSERT(!fContext);
    SkASSERT(!fGraphiteContext);

    fDevice.reset(MTLCreateSystemDefaultDevice());
    fQueue.reset([*fDevice newCommandQueue]);

    if (fDisplayParams->msaaSampleCount() > 1) {
        if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
            if (![*fDevice supportsTextureSampleCount:fDisplayParams->msaaSampleCount()]) {
                return;
            }
        } else {
            return;
        }
    }
    fSampleCount = fDisplayParams->msaaSampleCount();
    fStencilBits = 8;

    fValid = this->onInitializeContext();

    skgpu::graphite::MtlBackendContext backendContext = {};
    backendContext.fDevice.retain((CFTypeRef)fDevice.get());
    backendContext.fQueue.retain((CFTypeRef)fQueue.get());

    SkASSERT(fDisplayParams->graphiteTestOptions());
    skwindow::GraphiteTestOptions opts = *fDisplayParams->graphiteTestOptions();

    opts.fTestOptions.fContextOptions.fRequireOrderedRecordings = true;
    // Needed to make synchronous readPixels work:
    opts.fPriv.fStoreContextRefInRecorder = true;
    fDisplayParams =
            GraphiteDisplayParamsBuilder(fDisplayParams.get()).graphiteTestOptions(opts).build();
    fGraphiteContext = skgpu::graphite::ContextFactory::MakeMetal(
            backendContext, fDisplayParams->graphiteTestOptions()->fTestOptions.fContextOptions);
    fGraphiteRecorder = fGraphiteContext->makeRecorder(ToolUtils::CreateTestingRecorderOptions());
    // TODO
    //    if (!fGraphiteContext && fDisplayParams->msaaSampleCount() > 1) {
    //        fDisplayParams->msaaSampleCount() /= 2;
    //        this->initializeContext();
    //        return;
    //    }
}

void GraphiteMetalWindowContext::destroyContext() {
    if (fGraphiteContext) {
        fGraphiteRecorder.reset();
        fGraphiteContext.reset();
    }

    this->onDestroyContext();

    fMetalLayer = nil;
    fValid = false;

    fQueue.reset();
    fDevice.reset();
}

sk_sp<SkSurface> GraphiteMetalWindowContext::getBackbufferSurface() {
    sk_sp<SkSurface> surface;
    id<CAMetalDrawable> currentDrawable = [fMetalLayer nextDrawable];
    if (currentDrawable == nil) {
        return nullptr;
    }

    auto backendTex = skgpu::graphite::BackendTextures::MakeMetal(
            this->dimensions(), (CFTypeRef)currentDrawable.texture);

    surface = SkSurfaces::WrapBackendTexture(this->graphiteRecorder(),
                                             backendTex,
                                             kBGRA_8888_SkColorType,
                                             fDisplayParams->colorSpace(),
                                             &fDisplayParams->surfaceProps());
    fDrawableHandle = CFRetain((CFTypeRef)currentDrawable);

    return surface;
}

void GraphiteMetalWindowContext::onSwapBuffers() {
    this->submitToGpu();

    id<CAMetalDrawable> currentDrawable = (id<CAMetalDrawable>)fDrawableHandle;

    id<MTLCommandBuffer> commandBuffer([*fQueue commandBuffer]);
    commandBuffer.label = @"Present";

    [commandBuffer presentDrawable:currentDrawable];
    [commandBuffer commit];
    // ARC is off in sk_app, so we need to release the CF ref manually
    CFRelease(fDrawableHandle);
    fDrawableHandle = nil;
}

void GraphiteMetalWindowContext::setDisplayParams(std::unique_ptr<const DisplayParams> params) {
    this->destroyContext();
    fDisplayParams = std::move(params);
    this->initializeContext();
}

void GraphiteMetalWindowContext::activate(bool isActive) {}

}  // namespace skwindow::internal
