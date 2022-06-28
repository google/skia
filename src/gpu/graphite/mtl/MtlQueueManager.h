/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlQueueManager_DEFINED
#define skgpu_graphite_MtlQueueManager_DEFINED

#include "src/gpu/graphite/QueueManager.h"

namespace skgpu::graphite {

class Gpu;
class MtlGpu;

class MtlQueueManager : public QueueManager {
public:
    MtlQueueManager(Gpu*);
    ~MtlQueueManager() override {}

private:
    MtlGpu* mtlGpu() const;

    sk_sp<CommandBuffer> getNewCommandBuffer() override;
    OutstandingSubmission onSubmitToGpu() override;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlQueueManager_DEFINED
