/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "InstancedRendering.h"

#include "InstancedOp.h"
#include "GrAppliedClip.h"
#include "GrCaps.h"
#include "GrGpu.h"
#include "GrPipeline.h"
#include "GrResourceProvider.h"

#include "instanced/InstanceProcessor.h"

namespace gr_instanced {


InstancedRendering::InstancedRendering(GrGpu* gpu)
    : fGpu(SkRef(gpu))
    SkDEBUGCODE(, fState(State::kRecordingDraws)) {
}

InstancedRendering::~InstancedRendering() {
    SkASSERT(State::kRecordingDraws == fState);
}

void InstancedRendering::beginFlush(GrResourceProvider* rp) {
#ifdef SK_DEBUG
    SkASSERT(State::kRecordingDraws == fState);
    fState = State::kFlushing;
#endif

    if (fTrackedOps.isEmpty()) {
        return;
    }

    if (!fVertexBuffer) {
        fVertexBuffer.reset(InstanceProcessor::FindOrCreateVertexBuffer(fGpu.get()));
        if (!fVertexBuffer) {
            return;
        }
    }

    if (!fIndexBuffer) {
      fIndexBuffer.reset(InstanceProcessor::FindOrCreateIndex8Buffer(fGpu.get()));
        if (!fIndexBuffer) {
            return;
        }
    }

    if (!fParams.empty()) {
        fParamsBuffer.reset(rp->createBuffer(fParams.count() * sizeof(ParamsTexel),
                                             kTexel_GrBufferType, kDynamic_GrAccessPattern,
                                             GrResourceProvider::kNoPendingIO_Flag |
                                             GrResourceProvider::kRequireGpuMemory_Flag,
                                             fParams.begin()));
        if (!fParamsBuffer) {
            return;
        }
    }

    this->onBeginFlush(rp);
}

void InstancedRendering::draw(const GrPipeline& pipeline,
                              OpInfo info,
                              const InstancedOp* baseOp) {
    InstanceProcessor instProc(info, fParamsBuffer.get());

    this->onDraw(pipeline, instProc, baseOp);
}

void InstancedRendering::endFlush() {
    // The caller is expected to delete all tracked ops (i.e. ops whose applyPipelineOptimizations
    // method has been called) before ending the flush.
    SkASSERT(fTrackedOps.isEmpty());
    fParams.reset();
    fParamsBuffer.reset();
    this->onEndFlush();
    SkDEBUGCODE(fState = State::kRecordingDraws;)
    // Hold on to the shape coords and index buffers.
}

void InstancedRendering::resetGpuResources(ResetType resetType) {
    fVertexBuffer.reset();
    fIndexBuffer.reset();
    fParamsBuffer.reset();
    this->onResetGpuResources(resetType);
}

int InstancedRendering::addOpParams(InstancedOp* op) {
    if (op->fParams.empty()) {
        return 0;
    }

    SkASSERT(fParams.count() < (int)kParamsIdx_InfoMask); // TODO: cleaner.
    int count = fParams.count();
    fParams.push_back_n(op->fParams.count(), op->fParams.begin());
    return count;
}
}
