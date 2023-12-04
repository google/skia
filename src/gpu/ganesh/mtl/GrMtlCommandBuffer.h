/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlCommandBuffer_DEFINED
#define GrMtlCommandBuffer_DEFINED

#import <Metal/Metal.h>

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrTypes.h"
#include "src/gpu/GpuRefCnt.h"
#include "src/gpu/ganesh/GrBuffer.h"
#include "src/gpu/ganesh/GrManagedResource.h"
#include "src/gpu/ganesh/GrSurface.h"
#include "src/gpu/ganesh/mtl/GrMtlRenderCommandEncoder.h"
#include "src/gpu/ganesh/mtl/GrMtlUtil.h"

class GrMtlEvent;
class GrMtlGpu;
class GrMtlPipelineState;
class GrMtlOpsRenderPass;
class GrMtlRenderCommandEncoder;

GR_NORETAIN_BEGIN

class GrMtlCommandBuffer : public SkRefCnt {
public:
    static sk_sp<GrMtlCommandBuffer> Make(id<MTLCommandQueue> queue);
    ~GrMtlCommandBuffer() override;

    void releaseResources();

    bool commit(bool waitUntilCompleted);
    bool hasWork() { return fHasWork; }

    void addFinishedCallback(sk_sp<skgpu::RefCntedCallback> callback) {
        fFinishedCallbacks.push_back(std::move(callback));
    }

    id<MTLBlitCommandEncoder> getBlitCommandEncoder();
    // Tries to reuse current renderCommandEncoder if possible
    GrMtlRenderCommandEncoder* getRenderCommandEncoder(MTLRenderPassDescriptor*,
                                                       const GrMtlPipelineState*,
                                                       GrMtlOpsRenderPass* opsRenderPass);
    // Replaces current renderCommandEncoder with new one
    GrMtlRenderCommandEncoder* getRenderCommandEncoder(MTLRenderPassDescriptor*,
                                                       GrMtlOpsRenderPass*);

    void addCompletedHandler(MTLCommandBufferHandler block) {
        [fCmdBuffer addCompletedHandler:block];
    }

    void addResource(const sk_sp<const GrManagedResource>& resource) {
// Disable generic resource tracking for now
//        SkASSERT(resource);
//        fTrackedResources.push_back(std::move(resource));
    }

    void addGrBuffer(sk_sp<const GrBuffer> buffer) {
        fTrackedGrBuffers.push_back(std::move(buffer));
    }

    void addGrSurface(sk_sp<const GrSurface> surface) {
        fTrackedGrSurfaces.push_back(std::move(surface));
    }

    void encodeSignalEvent(sk_sp<GrMtlEvent>, uint64_t value);
    void encodeWaitForEvent(sk_sp<GrMtlEvent>, uint64_t value);

    void waitUntilCompleted() {
        [fCmdBuffer waitUntilCompleted];
    }
    bool isCompleted() {
        return fCmdBuffer.status == MTLCommandBufferStatusCompleted ||
               fCmdBuffer.status == MTLCommandBufferStatusError;
    }
    void callFinishedCallbacks() { fFinishedCallbacks.clear(); }

    void pushDebugGroup(NSString* string) {
        if (@available(macOS 10.13, iOS 11.0, tvOS 11.0, *)) {
            [fCmdBuffer pushDebugGroup:string];
        }
    }

    void popDebugGroup() {
        if (@available(macOS 10.13, iOS 11.0, tvOS 11.0, *)) {
            [fCmdBuffer popDebugGroup];
        }
    }

private:
    GrMtlCommandBuffer(id<MTLCommandBuffer> cmdBuffer)
        : fCmdBuffer(cmdBuffer)
        , fActiveBlitCommandEncoder(nil)
        , fActiveRenderCommandEncoder(nil)
        , fPreviousRenderPassDescriptor(nil)
        , fHasWork(false) {}

    void endAllEncoding();

    static const int kInitialTrackedResourcesCount = 32;

    skia_private::STArray<
        kInitialTrackedResourcesCount, sk_sp<const GrManagedResource>> fTrackedResources;
    skia_private::STArray<kInitialTrackedResourcesCount, sk_sp<const GrBuffer>> fTrackedGrBuffers;
    skia_private::STArray<16, gr_cb<const GrSurface>> fTrackedGrSurfaces;

    id<MTLCommandBuffer>        fCmdBuffer;
    id<MTLBlitCommandEncoder>   fActiveBlitCommandEncoder;
    std::unique_ptr<GrMtlRenderCommandEncoder> fActiveRenderCommandEncoder;
    MTLRenderPassDescriptor*    fPreviousRenderPassDescriptor;
    bool                        fHasWork;

    skia_private::TArray<sk_sp<skgpu::RefCntedCallback>> fFinishedCallbacks;

};

GR_NORETAIN_END

#endif
