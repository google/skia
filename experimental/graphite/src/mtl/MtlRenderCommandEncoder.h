/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlRenderCommandEncoder_DEFINED
#define skgpu_MtlRenderCommandEncoder_DEFINED

#include "experimental/graphite/src/Resource.h"
#include "include/core/SkRefCnt.h"
#include "include/ports/SkCFObject.h"

#import <Metal/Metal.h>

namespace skgpu::mtl {

/**
 * Wraps a MTLRenderCommandEncoder object and associated tracked state
 */
class RenderCommandEncoder : public Resource {
public:
    static sk_sp<RenderCommandEncoder> Make(const Gpu* gpu,
                                            id<MTLCommandBuffer> commandBuffer,
                                            MTLRenderPassDescriptor* descriptor) {
        // Adding a retain here to keep our own ref separate from the autorelease pool
        sk_cfp<id<MTLRenderCommandEncoder>> encoder =
                 sk_ret_cfp([commandBuffer renderCommandEncoderWithDescriptor:descriptor]);
        return sk_sp<RenderCommandEncoder>(new RenderCommandEncoder(gpu, std::move(encoder)));
    }

    void setLabel(NSString* label) {
        [(*fCommandEncoder) setLabel:label];
    }

    void pushDebugGroup(NSString* string) {
        [(*fCommandEncoder) pushDebugGroup:string];
    }
    void popDebugGroup() {
        [(*fCommandEncoder) popDebugGroup];
    }
    void insertDebugSignpost(NSString* string) {
        [(*fCommandEncoder) insertDebugSignpost:string];
    }

    void setRenderPipelineState(id<MTLRenderPipelineState> pso) {
        if (fCurrentRenderPipelineState != pso) {
            [(*fCommandEncoder) setRenderPipelineState:pso];
            fCurrentRenderPipelineState = pso;
        }
    }

    void setTriangleFillMode(MTLTriangleFillMode fillMode) {
        if (fCurrentTriangleFillMode != fillMode) {
            [(*fCommandEncoder) setTriangleFillMode:fillMode];
            fCurrentTriangleFillMode = fillMode;
        }
    }

    void setFrontFacingWinding(MTLWinding winding) {
        [(*fCommandEncoder) setFrontFacingWinding:winding];
    }

    void setViewport(const MTLViewport& viewport) {
        [(*fCommandEncoder) setViewport:viewport];
    }

    void setVertexBuffer(id<MTLBuffer> buffer, NSUInteger offset, NSUInteger index) {
        SkASSERT(index < kMaxExpectedBuffers);
        if (@available(macOS 10.11, iOS 8.3, *)) {
            if (fCurrentVertexBuffer[index] == buffer) {
                this->setVertexBufferOffset(offset, index);
                return;
            }
        }
        if (fCurrentVertexBuffer[index] != buffer || fCurrentVertexOffset[index] != offset) {
            [(*fCommandEncoder) setVertexBuffer:buffer
                                         offset:offset
                                        atIndex:index];
            fCurrentVertexBuffer[index] = buffer;
            fCurrentVertexOffset[index] = offset;
        }
    }
    void setVertexBufferOffset(NSUInteger offset, NSUInteger index)
            SK_API_AVAILABLE(macos(10.11), ios(8.3)) {
        SkASSERT(index < kMaxExpectedBuffers);
        if (fCurrentVertexOffset[index] != offset) {
            [(*fCommandEncoder) setVertexBufferOffset:offset
                                              atIndex:index];
            fCurrentVertexOffset[index] = offset;
        }
    }

    void setFragmentBuffer(id<MTLBuffer> buffer, NSUInteger offset, NSUInteger index) {
        SkASSERT(index < kMaxExpectedBuffers);
        if (@available(macOS 10.11, iOS 8.3, *)) {
            if (fCurrentFragmentBuffer[index] == buffer) {
                this->setFragmentBufferOffset(offset, index);
                return;
            }
        }
        if (fCurrentFragmentBuffer[index] != buffer || fCurrentFragmentOffset[index] != offset) {
            [(*fCommandEncoder) setFragmentBuffer:buffer
                                           offset:offset
                                          atIndex:index];
            fCurrentFragmentBuffer[index] = buffer;
            fCurrentFragmentOffset[index] = offset;
        }
    }
    void setFragmentBufferOffset(NSUInteger offset, NSUInteger index)
            SK_API_AVAILABLE(macos(10.11), ios(8.3)) {
        SkASSERT(index < kMaxExpectedBuffers);
        if (fCurrentFragmentOffset[index] != offset) {
            [(*fCommandEncoder) setFragmentBufferOffset:offset
                                                atIndex:index];
            fCurrentFragmentOffset[index] = offset;
        }
    }

    void setVertexBytes(const void* bytes, NSUInteger length, NSUInteger index)
            SK_API_AVAILABLE(macos(10.11), ios(8.3)) {
        [(*fCommandEncoder) setVertexBytes:bytes
                                    length:length
                                   atIndex:index];
    }
    void setFragmentBytes(const void* bytes, NSUInteger length, NSUInteger index)
            SK_API_AVAILABLE(macos(10.11), ios(8.3)) {
        [(*fCommandEncoder) setFragmentBytes:bytes
                                      length:length
                                     atIndex:index];
    }

    void setFragmentTexture(id<MTLTexture> texture, NSUInteger index) {
        SkASSERT(index < kMaxExpectedTextures);
        if (fCurrentTexture[index] != texture) {
            [(*fCommandEncoder) setFragmentTexture:texture
                                           atIndex:index];
            fCurrentTexture[index] = texture;
        }
    }
    void setFragmentSamplerState(id<MTLSamplerState> sampler, NSUInteger index) {
        SkASSERT(index < kMaxExpectedTextures);
        if (fCurrentSampler[index] != sampler) {
            [(*fCommandEncoder) setFragmentSamplerState: sampler
                                                atIndex: index];
            fCurrentSampler[index] = sampler;
        }
    }

    void setBlendColor(float blendConst[4]) {
        [(*fCommandEncoder) setBlendColorRed: blendConst[0]
                                       green: blendConst[1]
                                        blue: blendConst[2]
                                       alpha: blendConst[3]];
    }

    void setStencilReferenceValue(uint32_t referenceValue) {
        if (referenceValue != fCurrentStencilReferenceValue) {
            [(*fCommandEncoder) setStencilReferenceValue:referenceValue];
            fCurrentStencilReferenceValue = referenceValue;
        }
    }
    void setDepthStencilState(id<MTLDepthStencilState> depthStencilState) {
        if (depthStencilState != fCurrentDepthStencilState) {
            [(*fCommandEncoder) setDepthStencilState:depthStencilState];
            fCurrentDepthStencilState = depthStencilState;
        }
    }

    void setScissorRect(const MTLScissorRect& scissorRect) {
        if (fCurrentScissorRect.x != scissorRect.x ||
            fCurrentScissorRect.y != scissorRect.y ||
            fCurrentScissorRect.width != scissorRect.width ||
            fCurrentScissorRect.height != scissorRect.height) {
            [(*fCommandEncoder) setScissorRect:scissorRect];
            fCurrentScissorRect = scissorRect;
        }
    }

    void drawPrimitives(MTLPrimitiveType primitiveType, NSUInteger vertexStart,
                        NSUInteger vertexCount) {
        [(*fCommandEncoder) drawPrimitives:primitiveType
                               vertexStart:vertexStart
                               vertexCount:vertexCount];
    }
    void drawPrimitives(MTLPrimitiveType primitiveType, NSUInteger vertexStart,
                        NSUInteger vertexCount, NSUInteger instanceCount,
                        NSUInteger baseInstance) SK_API_AVAILABLE(macos(10.11), ios(9.0)) {
        [(*fCommandEncoder) drawPrimitives:primitiveType
                            vertexStart:vertexStart
                            vertexCount:vertexCount
                          instanceCount:instanceCount
                           baseInstance:baseInstance];
    }
    void drawPrimitives(MTLPrimitiveType primitiveType, id<MTLBuffer> indirectBuffer,
                        NSUInteger indirectBufferOffset) SK_API_AVAILABLE(macos(10.11), ios(9.0)) {
        [(*fCommandEncoder) drawPrimitives:primitiveType
                            indirectBuffer:indirectBuffer
                      indirectBufferOffset:indirectBufferOffset];
    }

    void drawIndexedPrimitives(MTLPrimitiveType primitiveType, NSUInteger indexCount,
                               MTLIndexType indexType, id<MTLBuffer> indexBuffer,
                               NSUInteger indexBufferOffset) {
        [(*fCommandEncoder) drawIndexedPrimitives:primitiveType
                                    indexCount:indexCount
                                     indexType:indexType
                                   indexBuffer:indexBuffer
                             indexBufferOffset:indexBufferOffset];
    }
    void drawIndexedPrimitives(MTLPrimitiveType primitiveType, NSUInteger indexCount,
                               MTLIndexType indexType, id<MTLBuffer> indexBuffer,
                               NSUInteger indexBufferOffset,
                               NSUInteger instanceCount,
                               NSInteger baseVertex,
                               NSUInteger baseInstance) SK_API_AVAILABLE(macos(10.11), ios(9.0)) {
        [(*fCommandEncoder) drawIndexedPrimitives:primitiveType
                                    indexCount:indexCount
                                     indexType:indexType
                                   indexBuffer:indexBuffer
                             indexBufferOffset:indexBufferOffset
                                 instanceCount:instanceCount
                                    baseVertex:baseVertex
                                  baseInstance:baseInstance];
    }
    void drawIndexedPrimitives(MTLPrimitiveType primitiveType,
                               MTLIndexType indexType, id<MTLBuffer> indexBuffer,
                               NSUInteger indexBufferOffset, id<MTLBuffer> indirectBuffer,
                               NSUInteger indirectBufferOffset)
            SK_API_AVAILABLE(macos(10.11), ios(9.0)) {
        [(*fCommandEncoder) drawIndexedPrimitives:primitiveType
                                        indexType:indexType
                                      indexBuffer:indexBuffer
                                indexBufferOffset:indexBufferOffset
                                   indirectBuffer:indirectBuffer
                             indirectBufferOffset:indirectBufferOffset];
    }

    void endEncoding() {
        [(*fCommandEncoder) endEncoding];
    }

private:
    RenderCommandEncoder(const Gpu* gpu, sk_cfp<id<MTLRenderCommandEncoder>> encoder)
        : Resource(gpu), fCommandEncoder(std::move(encoder)) {}

    void onFreeGpuData() override {
        fCommandEncoder.reset();
    }

    sk_cfp<id<MTLRenderCommandEncoder>> fCommandEncoder;

    id<MTLRenderPipelineState> fCurrentRenderPipelineState = nil;
    id<MTLDepthStencilState> fCurrentDepthStencilState = nil;
    uint32_t fCurrentStencilReferenceValue = 0; // Metal default value

    inline static constexpr int kMaxExpectedBuffers = 5;
    id<MTLBuffer> fCurrentVertexBuffer[kMaxExpectedBuffers];
    NSUInteger fCurrentVertexOffset[kMaxExpectedBuffers];
    id<MTLBuffer> fCurrentFragmentBuffer[kMaxExpectedBuffers];
    NSUInteger fCurrentFragmentOffset[kMaxExpectedBuffers];

    inline static constexpr int kMaxExpectedTextures = 16;
    id<MTLTexture> fCurrentTexture[kMaxExpectedTextures];
    id<MTLSamplerState> fCurrentSampler[kMaxExpectedTextures];

    MTLScissorRect fCurrentScissorRect = { 0, 0, 0, 0 };
    MTLTriangleFillMode fCurrentTriangleFillMode = (MTLTriangleFillMode)-1;
};

} // namespace skgpu::mtl

#endif // skgpu_MtlRenderCommandEncoder_DEFINED
