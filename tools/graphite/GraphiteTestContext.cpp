/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/GraphiteTestContext.h"

#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/include/GraphiteTypes.h"
#include "experimental/graphite/include/Recording.h"
#include "src/core/SkTraceEvent.h"
#include "tools/gpu/FlushFinishTracker.h"

namespace skiatest::graphite {

GraphiteTestContext::GraphiteTestContext() {}

GraphiteTestContext::~GraphiteTestContext() {}

void GraphiteTestContext::submitRecordingAndWaitOnSync(skgpu::Context* context,
                                                       skgpu::Recording* recording) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    SkASSERT(context);
    SkASSERT(recording);

    if (fFinishTrackers[fCurrentFlushIdx]) {
        fFinishTrackers[fCurrentFlushIdx]->waitTillFinished();
    }

    fFinishTrackers[fCurrentFlushIdx].reset(new sk_gpu_test::FlushFinishTracker(context));

    // We add an additional ref to the current flush tracker here. This ref is owned by the finish
    // callback on the flush call. The finish callback will unref the tracker when called.
    fFinishTrackers[fCurrentFlushIdx]->ref();

    skgpu::InsertRecordingInfo info;
    info.fRecording = recording;
    info.fFinishedContext = fFinishTrackers[fCurrentFlushIdx].get();
    info.fFinishedProc = sk_gpu_test::FlushFinishTracker::FlushFinished;
    context->insertRecording(info);

    context->submit(skgpu::SyncToCpu::kNo);

    fCurrentFlushIdx = (fCurrentFlushIdx + 1) % SK_ARRAY_COUNT(fFinishTrackers);
}

}  // namespace skiatest::graphite
