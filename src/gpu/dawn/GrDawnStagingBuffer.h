/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnStagingBuffer_DEFINED
#define GrDawnStagingBuffer_DEFINED

#include "dawn/webgpu_cpp.h"
#include "src/gpu/GrStagingBuffer.h"

class GrDawnStagingBuffer : public GrStagingBuffer {
public:
    GrDawnStagingBuffer(GrGpu* gpu, wgpu::Buffer buffer, size_t size, void* data)
        : INHERITED(gpu, size, data), fBuffer(buffer) {}
    ~GrDawnStagingBuffer() override {}
    void           mapAsync();
    wgpu::Buffer   buffer() const { return fBuffer; }
    GrDawnGpu*     getDawnGpu() const;

private:
    void           onUnmap() override;

    wgpu::Buffer   fBuffer;
    typedef GrStagingBuffer INHERITED;
};

class GrDawnTransferBuffer : public GrDawnBuffer {
public:
    GrDawnTransferBuffer(GrDawnGpu* gpu, size_t sizeInBytes, GrGpuBufferType type,
                         GrAccessPattern pattern);

    enum class MappedState {
        kNotMapped,
        kMapPending,
        kMapped,
    };

    void onMap() override;
    void mapWriteAsync();
    void mapReadAsync();

    void setMapPtr(void* mapPtr) {
        fMapPtr = mapPtr;
        fMappedState = MappedState::kMapped;
    }

    void onUnmap() override {
        GrDawnBuffer::onUnmap();
        fMappedState = MappedState::kNotMapped;
    }

    MappedState mappedState() const { return fMappedState; }

private:
    MappedState fMappedState = MappedState::kNotMapped;
};
#endif
