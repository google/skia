/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/GraphiteTestContext.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Recording.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "tools/gpu/FlushFinishTracker.h"

namespace skiatest::graphite {

GraphiteTestContext::GraphiteTestContext() {}

GraphiteTestContext::~GraphiteTestContext() {}

void GraphiteTestContext::submitRecordingAndWaitOnSync(skgpu::graphite::Context* context,
                                                       skgpu::graphite::Recording* recording) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    SkASSERT(context);
    SkASSERT(recording);

    if (fFinishTrackers[fCurrentFlushIdx]) {
        fFinishTrackers[fCurrentFlushIdx]->waitTillFinished([this] { tick(); });
    }

    fFinishTrackers[fCurrentFlushIdx].reset(new sk_gpu_test::FlushFinishTracker(context));

    // We add an additional ref to the current flush tracker here. This ref is owned by the finish
    // callback on the flush call. The finish callback will unref the tracker when called.
    fFinishTrackers[fCurrentFlushIdx]->ref();

    skgpu::graphite::InsertRecordingInfo info;
    info.fRecording = recording;
    info.fFinishedContext = fFinishTrackers[fCurrentFlushIdx].get();
    info.fFinishedProc = sk_gpu_test::FlushFinishTracker::FlushFinishedResult;
    context->insertRecording(info);

    context->submit(skgpu::graphite::SyncToCpu::kNo);

    fCurrentFlushIdx = (fCurrentFlushIdx + 1) % std::size(fFinishTrackers);
}

void GraphiteTestContext::syncedSubmit(skgpu::graphite::Context* context) {
    skgpu::graphite::SyncToCpu sync = context->priv().caps()->allowCpuSync()
                                              ? skgpu::graphite::SyncToCpu::kYes
                                              : skgpu::graphite::SyncToCpu::kNo;
    context->submit(sync);
    if (sync == skgpu::graphite::SyncToCpu::kNo) {
        while (context->hasUnfinishedGpuWork()) {
            this->tick();
            context->checkAsyncWorkCompletion();
        }
    }
}

}  // namespace skiatest::graphite
