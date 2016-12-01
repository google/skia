/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawOp.h"

GrDrawOp::GrDrawOp(uint32_t classID) : INHERITED(classID), fPipelineInstalled(false) { }

GrDrawOp::~GrDrawOp() {
    if (fPipelineInstalled) {
        this->pipeline()->~GrPipeline();
    }
}

void GrDrawOp::getPipelineOptimizations(GrPipelineOptimizations* opt) const {
    GrInitInvariantOutput color;
    GrInitInvariantOutput coverage;
    this->computePipelineOptimizations(&color, &coverage, &opt->fOverrides);
    opt->fColorPOI.initUsingInvariantOutput(color);
    opt->fCoveragePOI.initUsingInvariantOutput(coverage);
}

bool GrDrawOp::installPipeline(const GrPipeline::CreateArgs& args) {
    GrXPOverridesForBatch overrides;
    void* location = fPipelineStorage.get();
    if (!GrPipeline::CreateAt(location, args, &overrides)) {
        return false;
    }
    fPipelineInstalled = true;
    this->initBatchTracker(overrides);
    return true;
}
