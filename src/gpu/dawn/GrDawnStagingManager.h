/*
 * Copyright 2018 Google Inc.
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
    GrDawnStagingBuffer* findOrCreateStagingBuffer(size_t size, dawn::BufferUsageBit usage);

    void addToReadyPool(GrDawnStagingBuffer* buffer);
    void mapBusyList();

private:
    typedef std::pair<size_t, dawn::BufferUsageBit>   ReadyKey;

    dawn::Device                                      fDevice;
    std::vector<std::unique_ptr<GrDawnStagingBuffer>>  fBuffers;
    std::multimap<ReadyKey, GrDawnStagingBuffer*>      fReadyPool;
    std::vector<GrDawnStagingBuffer*>                  fBusyList;
    int                                               fWaitingCount = 0;
};

struct GrDawnStagingBuffer {
    GrDawnStagingBuffer(GrDawnStagingManager* manager, dawn::Buffer buffer, size_t size,
                       dawn::BufferUsageBit usage)
        : fManager(manager), fBuffer(buffer), fSize(size), fUsage(usage), fData(nullptr) {}
    ~GrDawnStagingBuffer() {
        fManager = nullptr;
    }
    GrDawnStagingManager* fManager;
    dawn::Buffer         fBuffer;
    size_t               fSize;
    dawn::BufferUsageBit fUsage;
    void*                fData;
};

#endif
