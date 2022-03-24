/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_GpuWorkSubmission_DEFINED
#define skgpu_GpuWorkSubmission_DEFINED

#include "include/core/SkRefCnt.h"

namespace skgpu {
class CommandBuffer;
class Gpu;

class GpuWorkSubmission {
public:
    virtual ~GpuWorkSubmission();

    virtual bool isFinished() = 0;
    virtual void waitUntilFinished(const Gpu*) = 0;

protected:
    CommandBuffer* commandBuffer() { return fCommandBuffer.get(); }

    GpuWorkSubmission(sk_sp<CommandBuffer> cmdBuffer);

private:
    sk_sp<CommandBuffer> fCommandBuffer;
};

} // namespace skgpu

#endif // skgpu_GpuWorkSubmission_DEFINED
