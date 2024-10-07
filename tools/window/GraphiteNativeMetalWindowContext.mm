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
#include "include/gpu/graphite/mtl/MtlGraphiteTypes.h"
#include "include/gpu/graphite/mtl/MtlGraphiteUtils.h"
#include "src/base/SkMathPriv.h"
#include "src/gpu/graphite/ContextOptionsPriv.h"
#include "tools/GpuToolUtils.h"

using skwindow::DisplayParams;
using skwindow::internal::GraphiteMetalWindowContext;

namespace skwindow::internal {

GraphiteMetalWindowContext::GraphiteMetalWindowContext(const DisplayParams& params)
        : WindowContext(params), fValid(false), fDrawableHandle(nil) {
    // SkNextPow2 is undefined for 0, so handle that ourselves.
    if (fDisplayParams.fMSAASampleCount <= 1) {
        fDisplayParams.fMSAASampleCount = 1;
    } else {
        fDisplayParams.fMSAASampleCount = SkNextPow2(fDisplayParams.fMSAASampleCount);
    }
}

void GraphiteMetalWindowContext::initializeContext() {
    SkASSERT(!fContext);
    SkASSERT(!fGraphiteContext);

    fDevice.reset(MTLCreateSystemDefaultDevice());
    fQueue.reset([*fDevice newCommandQueue]);

    if (fDisplayParams.fMSAASampleCount > 1) {
        if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
            if (![*fDevice supportsTextureSampleCount:fDisplayParams.fMSAASampleCount]) {
                return;
            }
        } else {
            return;
        }
    }
    fSampleCount = fDisplayParams.fMSAASampleCount;
    fStencilBits = 8;

    fValid = this->onInitializeContext();

    skgpu::graphite::MtlBackendContext backendContext = {};
    backendContext.fDevice.retain((CFTypeRef)fDevice.get());
    backendContext.fQueue.retain((CFTypeRef)fQueue.get());

    fDisplayParams.fGraphiteTestOptions.fTestOptions.fContextOptions.fDisableCachedGlyphUploads =
            true;
    // Needed to make synchronous readPixels work:
    fDisplayParams.fGraphiteTestOptions.fPriv.fStoreContextRefInRecorder = true;
    fGraphiteContext = skgpu::graphite::ContextFactory::MakeMetal(
            backendContext, fDisplayParams.fGraphiteTestOptions.fTestOptions.fContextOptions);
    fGraphiteRecorder = fGraphiteContext->makeRecorder(ToolUtils::CreateTestingRecorderOptions());
    // TODO
    //    if (!fGraphiteContext && fDisplayParams.fMSAASampleCount > 1) {
    //        fDisplayParams.fMSAASampleCount /= 2;
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
                                             fDisplayParams.fColorSpace,
                                             &fDisplayParams.fSurfaceProps);
    fDrawableHandle = CFRetain((CFTypeRef)currentDrawable);

    return surface;
}

void GraphiteMetalWindowContext::onSwapBuffers() {
    this->snapRecordingAndSubmit();

    id<CAMetalDrawable> currentDrawable = (id<CAMetalDrawable>)fDrawableHandle;

    id<MTLCommandBuffer> commandBuffer([*fQueue commandBuffer]);
    commandBuffer.label = @"Present";

    [commandBuffer presentDrawable:currentDrawable];
    [commandBuffer commit];
    // ARC is off in sk_app, so we need to release the CF ref manually
    CFRelease(fDrawableHandle);
    fDrawableHandle = nil;
}

void GraphiteMetalWindowContext::setDisplayParams(const DisplayParams& params) {
    this->destroyContext();
    fDisplayParams = params;
    this->initializeContext();
}

void GraphiteMetalWindowContext::activate(bool isActive) {}

}  // namespace skwindow::internal
