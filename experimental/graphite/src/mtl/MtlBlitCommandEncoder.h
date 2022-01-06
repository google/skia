/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlBlitCommandEncoder_DEFINED
#define skgpu_MtlBlitCommandEncoder_DEFINED

#include "experimental/graphite/src/Resource.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/ports/SkCFObject.h"

#import <Metal/Metal.h>

namespace skgpu::mtl {

/**
 * Wraps a MTLBlitCommandEncoder object
 */
class BlitCommandEncoder : public Resource {
public:
    static sk_sp<BlitCommandEncoder> Make(const skgpu::Gpu* gpu,
                                          id<MTLCommandBuffer> commandBuffer) {
        // Adding a retain here to keep our own ref separate from the autorelease pool
        sk_cfp<id<MTLBlitCommandEncoder>> encoder =
                sk_ret_cfp<id<MTLBlitCommandEncoder>>([commandBuffer blitCommandEncoder]);
        return sk_sp<BlitCommandEncoder>(new BlitCommandEncoder(gpu, std::move(encoder)));
    }

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

    void endEncoding() {
        [(*fCommandEncoder) endEncoding];
    }

private:
    BlitCommandEncoder(const skgpu::Gpu* gpu, sk_cfp<id<MTLBlitCommandEncoder>> encoder)
        : Resource(gpu), fCommandEncoder(std::move(encoder)) {}

    void onFreeGpuData() override {
        fCommandEncoder.reset();
    }

    sk_cfp<id<MTLBlitCommandEncoder>> fCommandEncoder;
};

} // namespace skgpu::mtl

#endif // skgpu_MtlBlitCommandEncoder_DEFINED
