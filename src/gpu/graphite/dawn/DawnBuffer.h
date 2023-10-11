/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnBuffer_DEFINED
#define skgpu_graphite_DawnBuffer_DEFINED

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

#include "include/core/SkRefCnt.h"
#include "include/gpu/graphite/dawn/DawnTypes.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"

namespace skgpu::graphite {

class DawnBuffer : public Buffer {
public:
    static sk_sp<Buffer> Make(const DawnSharedContext*,
                              size_t size,
                              BufferType type,
                              AccessPattern);

    const wgpu::Buffer& dawnBuffer() const { return fBuffer; }

private:
    DawnBuffer(const DawnSharedContext*,
               size_t size,
               wgpu::Buffer);

    void onMap() override;
    void onUnmap() override;

    void freeGpuData() override;

    const DawnSharedContext* dawnSharedContext() const {
        return static_cast<const DawnSharedContext*>(this->sharedContext());
    }

    wgpu::Buffer fBuffer;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnBuffer_DEFINED

