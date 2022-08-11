/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlComputeCommandEncoder_DEFINED
#define skgpu_graphite_MtlComputeCommandEncoder_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/ports/SkCFObject.h"
#include "src/gpu/graphite/ComputeTypes.h"
#include "src/gpu/graphite/Resource.h"

#import <Metal/Metal.h>

namespace skgpu::graphite {

/**
 * Wraps a MTLComputeCommandEncoder object and associated tracked state
 */
class MtlComputeCommandEncoder : public Resource {
public:
    static sk_sp<MtlComputeCommandEncoder> Make(const SharedContext* sharedContext,
                                                id<MTLCommandBuffer> commandBuffer) {
        // Adding a retain here to keep our own ref separate from the autorelease pool
        sk_cfp<id<MTLComputeCommandEncoder>> encoder =
                sk_ret_cfp([commandBuffer computeCommandEncoder]);

        // TODO(armansito): Support concurrent dispatch of compute passes using
        // MTLDispatchTypeConcurrent on macOS 10.14+ and iOS 12.0+.
        return sk_sp<MtlComputeCommandEncoder>(
                new MtlComputeCommandEncoder(sharedContext, std::move(encoder)));
    }

    void setLabel(NSString* label) { [(*fCommandEncoder) setLabel:label]; }

    void pushDebugGroup(NSString* string) { [(*fCommandEncoder) pushDebugGroup:string]; }
    void popDebugGroup() { [(*fCommandEncoder) popDebugGroup]; }
    void insertDebugSignpost(NSString* string) { [(*fCommandEncoder) insertDebugSignpost:string]; }

    void setComputePipelineState(id<MTLComputePipelineState> pso) {
        if (fCurrentComputePipelineState != pso) {
            [(*fCommandEncoder) setComputePipelineState:pso];
            fCurrentComputePipelineState = pso;
        }
    }

    void setBuffer(id<MTLBuffer> buffer, NSUInteger offset, NSUInteger index) {
        SkASSERT(buffer != nil);
        // TODO(skia:13580): As with the setVertexBufferOffset:atIndex: and
        // setFragmentBufferOffset:atIndex: methods of MTLRenderCommandEncoder,
        // Apple recommends using setBufferOffset:atIndex: to avoid rebinding a buffer when only
        // updating its offset. Consider tracking buffers/offsets by index and limiting calls to
        // setBuffer:offset:atIndex.
        [(*fCommandEncoder) setBuffer:buffer offset:offset atIndex:index];
    }

    void setTexture(id<MTLTexture> texture, NSUInteger index) {
        SkASSERT(texture != nil);
        [(*fCommandEncoder) setTexture:texture atIndex:index];
    }

    void setSamplerState(id<MTLSamplerState> sampler, NSUInteger index) {
        SkASSERT(sampler != nil);
        [(*fCommandEncoder) setSamplerState:sampler atIndex:index];
    }

    void dispatchThreadgroups(const WorkgroupSize& globalSize, const WorkgroupSize& localSize) {
        MTLSize threadgroupCount =
                MTLSizeMake(globalSize.fWidth, globalSize.fHeight, globalSize.fDepth);
        MTLSize threadsPerThreadgroup =
                MTLSizeMake(localSize.fWidth, localSize.fHeight, localSize.fDepth);
        [(*fCommandEncoder) dispatchThreadgroups:threadgroupCount
                           threadsPerThreadgroup:threadsPerThreadgroup];
    }

    void endEncoding() { [(*fCommandEncoder) endEncoding]; }

private:
    MtlComputeCommandEncoder(const SharedContext* sharedContext,
                             sk_cfp<id<MTLComputeCommandEncoder>> encoder)
            : Resource(sharedContext, Ownership::kOwned, SkBudgeted::kYes)
            , fCommandEncoder(std::move(encoder)) {}

    void freeGpuData() override { fCommandEncoder.reset(); }

    sk_cfp<id<MTLComputeCommandEncoder>> fCommandEncoder;

    id<MTLComputePipelineState> fCurrentComputePipelineState = nil;

    // TODO(skia:13580): Keep track of texture/sampler and buffer resources?
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_MtlComputeCommandEncoder_DEFINED
