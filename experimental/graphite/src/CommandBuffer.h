/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_CommandBuffer_DEFINED
#define skgpu_CommandBuffer_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkTArray.h"

namespace skgpu {
class Gpu;

class CommandBuffer : public SkRefCnt {
public:
    ~CommandBuffer() override {
        this->releaseResources();
    }

    bool hasWork() { return fHasWork; }

protected:
    CommandBuffer();

    void trackResource(sk_sp<SkRefCnt> resource) {
        fTrackedResources.push_back(std::move(resource));
    }
    void releaseResources();

    bool fHasWork = false;

private:
    static const int kInitialTrackedResourcesCount = 32;
    SkSTArray<kInitialTrackedResourcesCount, sk_sp<SkRefCnt>> fTrackedResources;

};

} // namespace skgpu

#endif // skgpu_CommandBuffer_DEFINED
