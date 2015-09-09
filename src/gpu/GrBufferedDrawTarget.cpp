/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBufferedDrawTarget.h"

// We will use the reordering buffer, unless we have NVPR.
// TODO move NVPR to batch so we can reorder
static inline bool allow_reordering(const GrCaps* caps) {
    return caps && caps->shaderCaps() && !caps->shaderCaps()->pathRenderingSupport();
}

GrBufferedDrawTarget::GrBufferedDrawTarget(GrContext* context)
    : INHERITED(context)
    , fCommands(GrCommandBuilder::Create(context->getGpu(), allow_reordering(context->caps())))
    , fDrawID(0) {
}

GrBufferedDrawTarget::~GrBufferedDrawTarget() {
    this->reset();
}

void GrBufferedDrawTarget::onDrawBatch(GrBatch* batch) {
    fCommands->recordDrawBatch(batch, *this->caps());
}

void GrBufferedDrawTarget::onFlush() {
    fCommands->flush(this->getGpu(), this->getContext()->resourceProvider());
    ++fDrawID;
}

void GrBufferedDrawTarget::onReset() {
    fCommands->reset();
}
