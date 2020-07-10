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
    ~GrDawnBuffer() override;

    bool onUpdateData(const void* src, size_t srcSizeInBytes) override;

    GrDawnGpu* getDawnGpu() const;
    wgpu::Buffer get() const { return fBuffer; }

protected:
    GrDawnBuffer(GrDawnGpu* gpu, size_t sizeInBytes, GrGpuBufferType type, GrAccessPattern pattern);

    wgpu::Buffer fBuffer;

private:
    typedef GrGpuBuffer INHERITED;
};

class GrDawnGpuBuffer : public GrDawnBuffer {
public:
    GrDawnGpuBuffer(GrDawnGpu* gpu,
                    size_t sizeInBytes,
                    GrGpuBufferType type,
                    GrAccessPattern pattern);

    void onMap() override;
    void onUnmap() override;

private:
    wgpu::Buffer fStagingBuffer;
    size_t fStagingOffset = 0;
};

class GrDawnTransferBuffer : public GrDawnBuffer {
public:
    GrDawnTransferBuffer(GrDawnGpu* gpu,
                         size_t sizeInBytes,
                         GrGpuBufferType type,
                         GrAccessPattern pattern);

    enum class MappedState {
        kNotMapped,
        kMapPending,
        kMapped,
    };

    void onMap() override;
    void onUnmap() override;

    void mapWriteAsync();
    void mapReadAsync();

    void setMapPtr(void* mapPtr) {
        fMapPtr = mapPtr;
        fMappedState = MappedState::kMapped;
    }

    MappedState mappedState() const { return fMappedState; }

private:
    MappedState fMappedState = MappedState::kNotMapped;
};

#endif
