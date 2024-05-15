/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlBuffer_DEFINED
#define skgpu_graphite_MtlBuffer_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/graphite/mtl/MtlGraphiteTypes.h"
#include "src/gpu/graphite/Buffer.h"

#import <Metal/Metal.h>

namespace skgpu::graphite {
class MtlSharedContext;

class MtlBuffer : public Buffer {
public:
    static sk_sp<Buffer> Make(const MtlSharedContext*,
                              size_t size,
                              BufferType type,
                              AccessPattern,
                              std::string_view label);

    id<MTLBuffer> mtlBuffer() const { return fBuffer.get(); }

private:
    MtlBuffer(const MtlSharedContext*,
              size_t size,
              sk_cfp<id<MTLBuffer>>,
              std::string_view label);

    void onMap() override;
    void onUnmap() override;

    void freeGpuData() override;

    void setBackendLabel(char const* label) override;

    sk_cfp<id<MTLBuffer>> fBuffer;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlBuffer_DEFINED

