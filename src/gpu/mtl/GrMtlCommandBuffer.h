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
#include "src/gpu/mtl/GrMtlUtil.h"

class GrMtlGpu;
class GrMtlPipelineState;
class GrMtlOpsRenderPass;

class GrMtlCommandBuffer {
public:
    static GrMtlCommandBuffer* Create(id<MTLCommandQueue> queue);
    ~GrMtlCommandBuffer();

    void commit(bool waitUntilCompleted);

    id<MTLBlitCommandEncoder> getBlitCommandEncoder();
    id<MTLRenderCommandEncoder> getRenderCommandEncoder(MTLRenderPassDescriptor*,
                                                        const GrMtlPipelineState*,
                                                        GrMtlOpsRenderPass* opsRenderPass);

    void addCompletedHandler(MTLCommandBufferHandler block) {
        [fCmdBuffer addCompletedHandler:block];
    }

    void encodeSignalEvent(id<MTLEvent>, uint64_t value) API_AVAILABLE(macos(10.14), ios(12.0));
    void encodeWaitForEvent(id<MTLEvent>, uint64_t value) API_AVAILABLE(macos(10.14), ios(12.0));

private:
    GrMtlCommandBuffer(id<MTLCommandBuffer> cmdBuffer)
        : fCmdBuffer(cmdBuffer)
        , fPreviousRenderPassDescriptor(nil) {}

    void endAllEncoding();

    id<MTLCommandBuffer>        fCmdBuffer;
    id<MTLBlitCommandEncoder>   fActiveBlitCommandEncoder;
    id<MTLRenderCommandEncoder> fActiveRenderCommandEncoder;
    MTLRenderPassDescriptor*    fPreviousRenderPassDescriptor;
};

#endif
