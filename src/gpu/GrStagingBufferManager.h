/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStagingBufferManager_DEFINED
#define GrStagingBufferManager_DEFINED

#include "src/gpu/GrStagingBuffer.h"

#include <memory>
#include <vector>

class GrStagingBufferManager {
public:
    virtual ~GrStagingBufferManager() {}

    GrStagingBuffer::Slice allocateStagingBufferSlice(size_t size);

    // Backends should call this when they are submitting the buffers to the GPU. The implementation
    // should remove all buffers from the fStagedBuffers list.
    void detachStagedBuffers() {
        this->onDetachStagedBuffers();
#ifdef SK_DEBUG
        for (size_t i = 0; i < fStagedBuffers.size(); ++i) {
            SkASSERT(!fStagedBuffers[i]);
        }
#endif
        fStagedBuffers.clear();
    }

    // Once it is safe to reuse a detached buffer, the backend APIs should return it to this
    // manager by calling recycleStagingBuffer. The buffer should be mapped and reset to whatever
    // state is need to be immediately used.
    void recycleStagingBuffer(std::unique_ptr<GrStagingBuffer> buffer, void* mapPtr) {
        buffer->reset(mapPtr);
        fAvailableBuffers.push_back(std::move(buffer));
    }

    void release();
    void abandon();

protected:
    // Buffers that have data that will be copied from in the current frame
    std::vector<std::unique_ptr<GrStagingBuffer>> fStagedBuffers;

private:
    static constexpr size_t kMinStagingBufferSize = 32 * 1024;

    virtual void onDetachStagedBuffers() = 0;
    virtual std::unique_ptr<GrStagingBuffer> allocateStagingBuffer(size_t size) = 0;

    virtual void onRelease() = 0;
    virtual void onAbandon() = 0;

    // Unused cached buffers that can be used if we need a new staging buffer.
    std::vector<std::unique_ptr<GrStagingBuffer>> fAvailableBuffers;
};

#endif

