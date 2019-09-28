/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnBuffer_DEFINED
#define GrDawnBuffer_DEFINED

#include "src/gpu/GrGpuBuffer.h"
#include "dawn/dawncpp.h"

class GrDawnGpu;

class GrDawnBuffer : public GrGpuBuffer {
public:
    GrDawnBuffer(GrDawnGpu* gpu, size_t sizeInBytes, GrGpuBufferType tpye, GrAccessPattern pattern);
    ~GrDawnBuffer() override;

    void onMap() override;
    void onUnmap() override;
    bool onUpdateData(const void* src, size_t srcSizeInBytes) override;

    GrDawnGpu* getDawnGpu() const;
    dawn::Buffer get() const { return fBuffer; }

private:
    dawn::Buffer fBuffer;
    char* fData;          // Used only for map/unmap.
    typedef GrGpuBuffer INHERITED;
};

#endif
