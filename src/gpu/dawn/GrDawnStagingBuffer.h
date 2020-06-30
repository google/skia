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

class GrDawnStagingBufferManager;

class GrDawnStagingBuffer : public GrStagingBuffer {
public:
    GrDawnStagingBuffer(GrDawnStagingBufferManager* manager,
                        wgpu::Buffer buffer,
                        size_t size,
                        void* mapPtr)
        : INHERITED(size, mapPtr), fManager(manager), fBuffer(buffer) {}
    void mapAsync();
    void unmap();
    void markAvailable(void* data);

    wgpu::Buffer buffer() const { return fBuffer; }

private:
    GrDawnStagingBufferManager* fManager;
    wgpu::Buffer   fBuffer;
    bool fNeedsToBeMapped = false;
    typedef GrStagingBuffer INHERITED;
};

#endif
