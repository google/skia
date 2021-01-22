/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkMeshBuffer_DEFINED
#define GrVkMeshBuffer_DEFINED

#include "src/gpu/GrGpuBuffer.h"
#include "src/gpu/vk/GrVkBuffer.h"

class GrVkGpu;

class GrVkMeshBuffer : public GrGpuBuffer, public GrVkBuffer {
public:
    static sk_sp<GrVkMeshBuffer> Make(GrVkGpu* gpu, GrGpuBufferType, size_t size, bool dynamic);

protected:
    void onAbandon() override;
    void onRelease() override;

private:
    GrVkMeshBuffer(GrVkGpu*, GrGpuBufferType, const GrVkBuffer::Desc&, const GrVkBuffer::Resource*);

    void onMap() override;
    void onUnmap() override;
    bool onUpdateData(const void* src, size_t srcSizeInBytes) override;

    GrVkGpu* getVkGpu() const;

    using INHERITED = GrGpuBuffer;
};

#endif
