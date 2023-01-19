/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSurface.h"
#include "src/base/SkMathPriv.h"
#include "tools/sk_app/GraphiteMetalWindowContext.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/mtl/MtlBackendContext.h"
#include "include/gpu/graphite/mtl/MtlTypes.h"
#include "include/gpu/graphite/mtl/MtlUtils.h"
#include "tools/ToolUtils.h"

using sk_app::DisplayParams;
using sk_app::GraphiteMetalWindowContext;

namespace sk_app {

GraphiteMetalWindowContext::GraphiteMetalWindowContext(const DisplayParams& params)
        : WindowContext(params)
        , fValid(false)
        , fDrawableHandle(nil) {
    fDisplayParams.fMSAASampleCount = GrNextPow2(fDisplayParams.fMSAASampleCount);
}

void GraphiteMetalWindowContext::initializeContext() {
    SkASSERT(!fContext);
    SkASSERT(!fGraphiteContext);

    fDevice.reset(MTLCreateSystemDefaultDevice());
    fQueue.reset([*fDevice newCommandQueue]);

    if (fDisplayParams.fMSAASampleCount > 1) {
        if (@available(macOS 10.11, iOS 9.0, *)) {
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
    backendContext.fDevice.retain((skgpu::graphite::MtlHandle)fDevice.get());
    backendContext.fQueue.retain((skgpu::graphite::MtlHandle)fQueue.get());

    skgpu::graphite::ContextOptions contextOptions;
    contextOptions.fStoreContextRefInRecorder = true;
    fGraphiteContext = skgpu::graphite::ContextFactory::MakeMetal(backendContext, contextOptions);
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

    skgpu::graphite::BackendTexture backendTex(this->dimensions(),
                                               (skgpu::graphite::MtlHandle)currentDrawable.texture);

    surface = SkSurface::MakeGraphiteFromBackendTexture(this->graphiteRecorder(),
                                                        backendTex,
                                                        kBGRA_8888_SkColorType,
                                                        fDisplayParams.fColorSpace,
                                                        &fDisplayParams.fSurfaceProps);

    fDrawableHandle = CFRetain((skgpu::graphite::MtlHandle) currentDrawable);

    return surface;
}

void GraphiteMetalWindowContext::swapBuffers() {
    // This chunk of code should not be in this class but higher up either in Window or
    // WindowContext
    std::unique_ptr<skgpu::graphite::Recording> recording = fGraphiteRecorder->snap();
    if (recording) {
        skgpu::graphite::InsertRecordingInfo info;
        info.fRecording = recording.get();
        fGraphiteContext->insertRecording(info);
        fGraphiteContext->submit(skgpu::graphite::SyncToCpu::kNo);
    }

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

}   //namespace sk_app
