/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawBatch.h"

GrDrawBatch::GrDrawBatch(uint32_t classID) : INHERITED(classID), fPipelineInstalled(false) { }

GrDrawBatch::~GrDrawBatch() {
    if (fPipelineInstalled) {
        this->pipeline()->~GrPipeline();
    }
}

bool GrDrawBatch::installPipeline(const GrPipeline::CreateArgs& args) {
    GrPipelineOptimizations opts;
    void* location = fPipelineStorage.get();
    if (!GrPipeline::CreateAt(location, args, &opts)) {
        return false;
    }
    this->initBatchTracker(opts);
    fPipelineInstalled = true;
    return true;
}
