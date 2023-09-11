/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlQueueManager_DEFINED
#define skgpu_graphite_MtlQueueManager_DEFINED

#include "include/ports/SkCFObject.h"
#include "src/gpu/graphite/QueueManager.h"

#import <Metal/Metal.h>

namespace skgpu::graphite {

class MtlSharedContext;

class MtlQueueManager : public QueueManager {
public:
    MtlQueueManager(sk_cfp<id<MTLCommandQueue>> queue, const SharedContext*);
    ~MtlQueueManager() override {}

private:
    const MtlSharedContext* mtlSharedContext() const;

    std::unique_ptr<CommandBuffer> getNewCommandBuffer(ResourceProvider*) override;
    OutstandingSubmission onSubmitToGpu() override;

#if defined(GRAPHITE_TEST_UTILS)
    void startCapture() override;
    void stopCapture() override;
#endif

    sk_cfp<id<MTLCommandQueue>> fQueue;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlQueueManager_DEFINED
