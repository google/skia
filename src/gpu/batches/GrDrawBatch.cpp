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

void GrDrawBatch::getPipelineOptimizations(GrPipelineOptimizations* opt) const {
    GrInitInvariantOutput color;
    GrInitInvariantOutput coverage;
    this->computePipelineOptimizations(&color, &coverage, &opt->fOverrides);
    opt->fColorPOI.initUsingInvariantOutput(color);
    opt->fCoveragePOI.initUsingInvariantOutput(coverage);
}

bool GrDrawBatch::installPipeline(const GrPipeline::CreateArgs& args) {
    GrXPOverridesForBatch overrides;
    void* location = fPipelineStorage.get();
    if (!GrPipeline::CreateAt(location, args, &overrides)) {
        return false;
    }
    this->initBatchTracker(overrides);
    fPipelineInstalled = true;
    return true;
}
