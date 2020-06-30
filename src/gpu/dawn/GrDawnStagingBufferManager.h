/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnStagingBufferManager_DEFINED
#define GrDawnStagingBufferManager_DEFINED

#include "src/gpu/GrStagingBufferManager.h"

#include <list>

class GrDawnGpu;

class GrDawnStagingBufferManager : public GrStagingBufferManager {
public:
    GrDawnStagingBufferManager(GrDawnGpu* gpu) : fGpu(gpu) {}

    // This will call map on all busy buffers so that we have a callback to know when they are
    //finished on the GPU and can be recycled.
    void mapStagingBuffers();

    std::unique_ptr<GrStagingBuffer> removeFromBusy(const GrStagingBuffer* buffer);

    bool hasBusyBuffers() { return !fBusyBuffers.empty(); }

private:
    void onDetachStagedBuffers() override;
    std::unique_ptr<GrStagingBuffer> allocateStagingBuffer(size_t size) override;

    void onRelease() override {
        // We should have waited up in the GrDawnGpu for all outstanding work to finish which would
        // have moved the buffers off of the busy list.
        SkASSERT(!this->hasBusyBuffers());
    }
    void onAbandon() override {
        // We should have waited up in the GrDawnGpu for all outstanding work to finish which would
        // have moved the buffers off of the busy list.
        SkASSERT(!this->hasBusyBuffers());
    }

    GrDawnGpu* fGpu;
    std::list<std::unique_ptr<GrStagingBuffer>> fBusyBuffers;
};

#endif

