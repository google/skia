/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnStagingManager_DEFINED
#define GrDawnStagingManager_DEFINED

#include <map>
#include <memory>
#include <vector>

#include "dawn/webgpu_cpp.h"

class GrDawnStagingManager;

struct GrDawnStagingBuffer {
    GrDawnStagingBuffer(GrDawnStagingManager* manager, wgpu::Buffer buffer, size_t size,
                       void* data)
        : fManager(manager), fBuffer(buffer), fSize(size), fData(data) {}
    ~GrDawnStagingBuffer() {
        fManager = nullptr;
    }
    GrDawnStagingManager*  fManager;
    wgpu::Buffer           fBuffer;
    size_t                 fSize;
    size_t                 fOffset = 0;
    bool                   fBusy = false;
    void*                  fData;
    struct Slice {
        Slice(wgpu::Buffer buffer, int offset, void* data)
          : fBuffer(buffer), fOffset(offset), fData(data) {}
        wgpu::Buffer  fBuffer;
        int           fOffset;
        void*         fData;
    };
};

class GrDawnStagingManager {
public:
    GrDawnStagingManager(wgpu::Device device);
   ~GrDawnStagingManager();

    GrDawnStagingBuffer::Slice allocate(size_t size);
    void addToReadyPool(GrDawnStagingBuffer* buffer);
    void flush();
    void mapBusyList();

private:
    GrDawnStagingBuffer* find(size_t size);
    wgpu::Device                                       fDevice;
    std::vector<std::unique_ptr<GrDawnStagingBuffer>>  fBuffers;
    int                                                fWaitingCount = 0;
};

#endif
