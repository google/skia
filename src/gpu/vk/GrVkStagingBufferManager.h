/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkStagingBufferManager_DEFINED
#define GrVkStagingBufferManager_DEFINED

#include "src/gpu/GrStagingBufferManager.h"

#include "include/core/SkRefCnt.h"

class GrVkGpu;
class GrVkStagingBufferManager;
class GrVkTransferBuffer;

class GrVkStagingBuffer : public GrStagingBuffer {
public:
    static std::unique_ptr<GrVkStagingBuffer> Make(GrVkGpu* gpu, size_t size);

    void recycle();

    GrVkTransferBuffer* buffer() { return fBuffer.get(); }

private:
    GrVkStagingBuffer(GrVkStagingBufferManager*, size_t size, sk_sp<GrVkTransferBuffer>,
                      void* mapPtr);

    GrVkStagingBufferManager* fManager;
    sk_sp<GrVkTransferBuffer> fBuffer;
};

class GrVkStagingBufferManager : public GrStagingBufferManager {
public:
    GrVkStagingBufferManager(GrVkGpu* gpu) : fGpu(gpu) {}

private:
    void onDetachStagedBuffers() override;
    std::unique_ptr<GrStagingBuffer> allocateStagingBuffer(size_t size) override;

    void onRelease() override {}
    void onAbandon() override {}

    GrVkGpu* fGpu;
};

#endif


