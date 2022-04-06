/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlBuffer_DEFINED
#define skgpu_graphite_MtlBuffer_DEFINED

#include "experimental/graphite/include/mtl/MtlTypes.h"
#include "experimental/graphite/src/Buffer.h"
#include "include/core/SkRefCnt.h"

#import <Metal/Metal.h>

namespace skgpu::graphite {
class MtlGpu;

class MtlBuffer : public skgpu::Buffer {
public:
    static sk_sp<Buffer> Make(const MtlGpu*, size_t size, BufferType type, PrioritizeGpuReads);

    id<MTLBuffer> mtlBuffer() const { return fBuffer.get(); }

private:
    MtlBuffer(const MtlGpu*, size_t size, BufferType type, PrioritizeGpuReads,
              sk_cfp<id<MTLBuffer>>);

    void onMap() override;
    void onUnmap() override;

    void freeGpuData() override;

    sk_cfp<id<MTLBuffer>> fBuffer;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlBuffer_DEFINED

