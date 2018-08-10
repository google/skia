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

#include "dawn/dawncpp.h"

struct GrNXTStagingBuffer;

class GrNXTStagingManager {
public:
    GrNXTStagingManager(dawn::Device device);
   ~GrNXTStagingManager();
    GrNXTStagingBuffer* findOrCreateStagingBuffer(size_t size, dawn::BufferUsageBit usage);

    void addToReadyPool(GrNXTStagingBuffer* buffer);
    void mapBusyList();

private:
    typedef std::pair<size_t, dawn::BufferUsageBit>   ReadyKey;

    dawn::Device                                      fDevice;
    std::vector<std::unique_ptr<GrNXTStagingBuffer>>  fBuffers;
    std::multimap<ReadyKey, GrNXTStagingBuffer*>      fReadyPool;
    std::vector<GrNXTStagingBuffer*>                  fBusyList;
    int                                               fWaitingCount = 0;
};

struct GrNXTStagingBuffer {
    GrNXTStagingBuffer(GrNXTStagingManager* manager, dawn::Buffer buffer, size_t size,
                       dawn::BufferUsageBit usage)
        : fManager(manager), fBuffer(buffer.Clone()), fSize(size), fUsage(usage), fData(nullptr) {}
    ~GrNXTStagingBuffer() {
        fManager = nullptr;
    }
    GrNXTStagingManager* fManager;
    dawn::Buffer         fBuffer;
    size_t               fSize;
    dawn::BufferUsageBit fUsage;
    void*                fData;
};

#endif
