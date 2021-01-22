/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnBuffer_DEFINED
#define GrDawnBuffer_DEFINED

#include "src/gpu/GrGpuBuffer.h"
#include "dawn/webgpu_cpp.h"

class GrDawnGpu;

class GrDawnBuffer : public GrGpuBuffer {
public:
    GrDawnBuffer(GrDawnGpu* gpu, size_t sizeInBytes, GrGpuBufferType type, GrAccessPattern pattern);

    ~GrDawnBuffer() override;

    void onMap() override;
    void onUnmap() override;
    bool onUpdateData(const void* src, size_t srcSizeInBytes) override;

    GrDawnGpu* getDawnGpu() const;
    wgpu::Buffer get() const { return fBuffer; }

    void mapWriteAsync();
    void mapReadAsync();

    void setMapPtr(void* mapPtr) {
        fMapPtr = mapPtr;
    }

private:
    wgpu::Buffer fBuffer;

    enum class Mappable {
        kNot,
        kReadOnly,
        kWriteOnly,
    };
    Mappable fMappable = Mappable::kNot;

    wgpu::Buffer fStagingBuffer;
    size_t fStagingOffset = 0;

    using INHERITED = GrGpuBuffer;
};

#endif
