/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNXTStagingManager_DEFINED
#define GrNXTStagingManager_DEFINED

#include <map>
#include <memory>
#include <vector>

#include "nxt/nxtcpp.h"

struct GrNXTStagingBuffer;

class GrNXTStagingManager {
public:
    GrNXTStagingManager(nxt::Device device);
   ~GrNXTStagingManager();
    GrNXTStagingBuffer* findOrCreateStagingBuffer(size_t size, nxt::BufferUsageBit usage);

    void addToReadyPool(GrNXTStagingBuffer* buffer);
    void mapBusyList();

private:
    typedef std::pair<size_t, nxt::BufferUsageBit>    ReadyKey;

    nxt::Device                                       fDevice;
    std::vector<std::unique_ptr<GrNXTStagingBuffer>>  fBuffers;
    std::multimap<ReadyKey, GrNXTStagingBuffer*>      fReadyPool;
    std::vector<GrNXTStagingBuffer*>                  fBusyList;
    int                                               fWaitingCount = 0;
};

struct GrNXTStagingBuffer {
    GrNXTStagingBuffer(GrNXTStagingManager* manager, nxt::Buffer buffer, size_t size,
                       nxt::BufferUsageBit usage)
        : fManager(manager), fBuffer(buffer.Clone()), fSize(size), fUsage(usage), fData(nullptr) {}
    ~GrNXTStagingBuffer() {
        fManager = nullptr;
    }
    GrNXTStagingManager* fManager;
    nxt::Buffer          fBuffer;
    size_t               fSize;
    nxt::BufferUsageBit  fUsage;
    void*                fData;
};

#endif
