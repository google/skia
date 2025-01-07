/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlBlitCommandEncoder_DEFINED
#define skgpu_graphite_MtlBlitCommandEncoder_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/ports/SkCFObject.h"
#include "src/gpu/graphite/Resource.h"

#import <Metal/Metal.h>

namespace skgpu::graphite {

/**
 * Wraps a MTLMtlBlitCommandEncoder object
 */
class MtlBlitCommandEncoder : public Resource {
public:
    static sk_sp<MtlBlitCommandEncoder> Make(const SharedContext* sharedContext,
                                             id<MTLCommandBuffer> commandBuffer) {
        @autoreleasepool {
            // Adding a retain here to keep our own ref separate from the autorelease pool
            sk_cfp<id<MTLBlitCommandEncoder>> encoder =
                    sk_ret_cfp<id<MTLBlitCommandEncoder>>([commandBuffer blitCommandEncoder]);
            return sk_sp<MtlBlitCommandEncoder>(new MtlBlitCommandEncoder(sharedContext,
                                                                          std::move(encoder)));
        }
    }

    const char* getResourceType() const override { return "Metal Blit Command Encoder"; }

    void pushDebugGroup(NSString* string) {
        [(*fCommandEncoder) pushDebugGroup:string];
    }
    void popDebugGroup() {
        [(*fCommandEncoder) popDebugGroup];
    }
#ifdef SK_BUILD_FOR_MAC
    void synchronizeResource(id<MTLBuffer> buffer) {
        [(*fCommandEncoder) synchronizeResource: buffer];
    }
#endif

    void fillBuffer(id<MTLBuffer> buffer, size_t bufferOffset, size_t bytes, uint8_t value) {
        [(*fCommandEncoder) fillBuffer:buffer
                                 range:NSMakeRange(bufferOffset, bytes)
                                 value:value];
    }

    void copyFromTexture(id<MTLTexture> texture,
                         SkIRect srcRect,
                         id<MTLBuffer> buffer,
                         size_t bufferOffset,
                         size_t bufferRowBytes) {
        [(*fCommandEncoder) copyFromTexture: texture
                                sourceSlice: 0
                                sourceLevel: 0
                               sourceOrigin: MTLOriginMake(srcRect.left(), srcRect.top(), 0)
                                 sourceSize: MTLSizeMake(srcRect.width(), srcRect.height(), 1)
                                   toBuffer: buffer
                          destinationOffset: bufferOffset
                     destinationBytesPerRow: bufferRowBytes
                   destinationBytesPerImage: bufferRowBytes * srcRect.height()];
    }

    void copyFromBuffer(id<MTLBuffer> buffer,
                        size_t bufferOffset,
                        size_t bufferRowBytes,
                        id<MTLTexture> texture,
                        SkIRect dstRect,
                        unsigned int dstLevel) {
        [(*fCommandEncoder) copyFromBuffer: buffer
                              sourceOffset: bufferOffset
                         sourceBytesPerRow: bufferRowBytes
                       sourceBytesPerImage: bufferRowBytes * dstRect.height()
                                sourceSize: MTLSizeMake(dstRect.width(), dstRect.height(), 1)
                                 toTexture: texture
                          destinationSlice: 0
                          destinationLevel: dstLevel
                         destinationOrigin: MTLOriginMake(dstRect.left(), dstRect.top(), 0)];
    }

    void copyTextureToTexture(id<MTLTexture> srcTexture,
                              SkIRect srcRect,
                              id<MTLTexture> dstTexture,
                              SkIPoint dstPoint,
                              int mipLevel) {
        [(*fCommandEncoder) copyFromTexture: srcTexture
                                sourceSlice: 0
                                sourceLevel: 0
                               sourceOrigin: MTLOriginMake(srcRect.x(), srcRect.y(), 0)
                                 sourceSize: MTLSizeMake(srcRect.width(), srcRect.height(), 1)
                                  toTexture: dstTexture
                           destinationSlice: 0
                           destinationLevel: mipLevel
                          destinationOrigin: MTLOriginMake(dstPoint.fX, dstPoint.fY, 0)];
    }

    void copyBufferToBuffer(id<MTLBuffer> srcBuffer,
                            size_t srcOffset,
                            id<MTLBuffer> dstBuffer,
                            size_t dstOffset,
                            size_t size) {
        [(*fCommandEncoder) copyFromBuffer: srcBuffer
                              sourceOffset: srcOffset
                                  toBuffer: dstBuffer
                         destinationOffset: dstOffset
                                      size: size];
    }

    void endEncoding() {
        [(*fCommandEncoder) endEncoding];
    }

private:
    MtlBlitCommandEncoder(const SharedContext* sharedContext,
                          sk_cfp<id<MTLBlitCommandEncoder>> encoder)
            : Resource(sharedContext,
                       Ownership::kOwned,
                       /*gpuMemorySize=*/0)
            , fCommandEncoder(std::move(encoder)) {}

    void freeGpuData() override {
        fCommandEncoder.reset();
    }

    sk_cfp<id<MTLBlitCommandEncoder>> fCommandEncoder;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlBlitCommandEncoder_DEFINED
