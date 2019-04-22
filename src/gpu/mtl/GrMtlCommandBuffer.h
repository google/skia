/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlCommandBuffer_DEFINED
#define GrMtlCommandBuffer_DEFINED

#import <Metal/Metal.h>

#include <SkRefCnt.h>

class GrMtlGpu;

class GrMtlCommandBuffer {
public:
    static GrMtlCommandBuffer* Create(id<MTLCommandQueue> queue);
    ~GrMtlCommandBuffer();

    void commit(bool waitUntilCompleted);

    id<MTLBlitCommandEncoder> getBlitCommandEncoder();
    id<MTLRenderCommandEncoder> getRenderCommandEncoder(MTLRenderPassDescriptor*);

private:
    GrMtlCommandBuffer(id<MTLCommandBuffer> cmdBuffer) : fCmdBuffer(cmdBuffer) {}

    void endAllEncoding();

    id<MTLCommandBuffer> fCmdBuffer;
    id<MTLBlitCommandEncoder> fActiveBlitCommandEncoder;
    id<MTLRenderCommandEncoder> fActiveRenderCommandEncoder;
};

#endif
