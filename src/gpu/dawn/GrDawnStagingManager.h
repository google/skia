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

#include "dawn/dawncpp.h"

struct GrDawnStagingBuffer;

class GrDawnStagingManager {
public:
    GrDawnStagingManager(dawn::Device device);
   ~GrDawnStagingManager();
    GrDawnStagingBuffer* findOrCreateStagingBuffer(size_t size);

    void addToReadyPool(GrDawnStagingBuffer* buffer);
    void mapBusyList();

private:
    dawn::Device                                       fDevice;
    std::vector<std::unique_ptr<GrDawnStagingBuffer>>  fBuffers;
    std::multimap<size_t, GrDawnStagingBuffer*>        fReadyPool;
    std::vector<GrDawnStagingBuffer*>                  fBusyList;
    int                                                fWaitingCount = 0;
};

struct GrDawnStagingBuffer {
    GrDawnStagingBuffer(GrDawnStagingManager* manager, dawn::Buffer buffer, size_t size,
                       void* data)
        : fManager(manager), fBuffer(buffer), fSize(size), fData(data) {}
    ~GrDawnStagingBuffer() {
        fManager = nullptr;
    }
    GrDawnStagingManager*  fManager;
    dawn::Buffer           fBuffer;
    size_t                 fSize;
    void*                  fData;
};

#endif
