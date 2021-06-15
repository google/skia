/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlRenderCommandEncoder_DEFINED
#define GrMtlRenderCommandEncoder_DEFINED

#include <memory>
#include "src/gpu/mtl/GrMtlSampler.h"
#include "src/gpu/mtl/GrMtlUtil.h"

class GrMtlSampler;

#import <Metal/Metal.h>

GR_NORETAIN_BEGIN

/**
 * Wraps a MTLRenderCommandEncoder object and associated tracked state
 */
class GrMtlRenderCommandEncoder {
public:
    static std::unique_ptr<GrMtlRenderCommandEncoder> Make(id<MTLRenderCommandEncoder> encoder) {
        return std::unique_ptr<GrMtlRenderCommandEncoder>(new GrMtlRenderCommandEncoder(encoder));
    }

    void setLabel(NSString* label) {
        [fCommandEncoder setLabel:label];
    }

    void pushDebugGroup(NSString* debugString) {
        [fCommandEncoder pushDebugGroup:debugString];
    }
    void popDebugGroup() {
        [fCommandEncoder popDebugGroup];
    }

    void setRenderPipelineState(id<MTLRenderPipelineState> pso) {
        [fCommandEncoder setRenderPipelineState:pso];
    }

    void setTriangleFillMode(MTLTriangleFillMode fillMode) {
        [fCommandEncoder setTriangleFillMode:fillMode];
    }

    void setFrontFacingWinding(MTLWinding winding) {
        [fCommandEncoder setFrontFacingWinding:winding];
    }

    void setViewport(const MTLViewport& viewport) {
        [fCommandEncoder setViewport:viewport];
    }

    void setVertexBuffer(id<MTLBuffer> buffer, NSUInteger offset, NSUInteger index) {
        [fCommandEncoder setVertexBuffer:buffer
                                  offset:offset
                                 atIndex:index];
    }
    void setVertexBufferOffset(NSUInteger offset, NSUInteger index) SK_API_AVAILABLE(ios(8.3)) {
        [fCommandEncoder setVertexBufferOffset:offset
                                       atIndex:index];
    }

    void setVertexBytes(const void* bytes, NSUInteger length, NSUInteger index)
            SK_API_AVAILABLE(ios(8.3)) {
        [fCommandEncoder setVertexBytes:bytes
                                 length:length
                                atIndex:index];
    }
    void setFragmentBytes(void* bytes, NSUInteger length, NSUInteger index)
            SK_API_AVAILABLE(ios(8.3)) {
        [fCommandEncoder setFragmentBytes:bytes
                                   length:length
                                  atIndex:index];
    }

    void setFragmentTexture(id<MTLTexture> texture, NSUInteger index) {
        [fCommandEncoder setFragmentTexture:texture
                                     atIndex:index];
    }
    void setFragmentSamplerState(GrMtlSampler* sampler, NSUInteger index) {
        [fCommandEncoder setFragmentSamplerState: sampler->mtlSampler()
                                     atIndex: index];
    }

    void setBlendColor(SkPMColor4f blendConst) {
        [fCommandEncoder setBlendColorRed: blendConst.fR
                                    green: blendConst.fG
                                     blue: blendConst.fB
                                    alpha: blendConst.fA];
    }

    void setStencilFrontBackReferenceValues(uint32_t frontReferenceValue,
                                            uint32_t backReferenceValue)
            SK_API_AVAILABLE(macos(10.11), ios(9.0)) {
        [fCommandEncoder
                setStencilFrontReferenceValue:frontReferenceValue
                           backReferenceValue:backReferenceValue];
    }
    void setStencilReferenceValue(uint32_t referenceValue) {
        [fCommandEncoder setStencilReferenceValue:referenceValue];
    }
    void setDepthStencilState(id<MTLDepthStencilState> depthStencilState) {
        [fCommandEncoder setDepthStencilState:depthStencilState];
    }

    void setScissorRect(const MTLScissorRect& scissorRect) {
        [fCommandEncoder setScissorRect:scissorRect];
    }

    void drawPrimitives(MTLPrimitiveType primitiveType, NSUInteger vertexStart,
                        NSUInteger vertexCount) {
        [fCommandEncoder drawPrimitives:primitiveType
                            vertexStart:vertexStart
                            vertexCount:vertexCount];
    }
    void drawPrimitives(MTLPrimitiveType primitiveType, NSUInteger vertexStart,
                        NSUInteger vertexCount, NSUInteger instanceCount,
                        NSUInteger baseInstance) SK_API_AVAILABLE(macos(10.11), ios(9.0)) {
        [fCommandEncoder drawPrimitives:primitiveType
                            vertexStart:vertexStart
                            vertexCount:vertexCount
                          instanceCount:instanceCount
                           baseInstance:baseInstance];
    }
    void drawPrimitives(MTLPrimitiveType primitiveType, id<MTLBuffer> indirectBuffer,
                        NSUInteger indirectBufferOffset) SK_API_AVAILABLE(macos(10.11), ios(9.0)) {
        [fCommandEncoder drawPrimitives:primitiveType
                         indirectBuffer:indirectBuffer
                   indirectBufferOffset:indirectBufferOffset];
    }

    void drawIndexedPrimitives(MTLPrimitiveType primitiveType, NSUInteger indexCount,
                               MTLIndexType indexType, id<MTLBuffer> indexBuffer,
                               NSUInteger indexBufferOffset) {
        [fCommandEncoder drawIndexedPrimitives:primitiveType
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
        [fCommandEncoder drawIndexedPrimitives:primitiveType
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
        [fCommandEncoder drawIndexedPrimitives:primitiveType
                                     indexType:indexType
                                   indexBuffer:indexBuffer
                             indexBufferOffset:indexBufferOffset
                                indirectBuffer:indirectBuffer
                          indirectBufferOffset:indirectBufferOffset];
    }

    void endEncoding() {
        [fCommandEncoder endEncoding];
    }

private:
    GrMtlRenderCommandEncoder(id<MTLRenderCommandEncoder> encoder)
        : fCommandEncoder(encoder) {}

    id<MTLRenderCommandEncoder> fCommandEncoder = nil;
};

GR_NORETAIN_END

#endif
