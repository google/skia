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

void GrDrawOp::initPipelineAnalysis(GrPipelineAnalysis* analysis) const {
    GrPipelineInput color;
    GrPipelineInput coverage;
    GrPipelineAnalysisDrawOpInput input(&color, &coverage);
    this->getPipelineAnalysisInput(&input);
    analysis->fColorPOI.reset(color);
    analysis->fCoveragePOI.reset(coverage);
    analysis->fUsesPLSDstRead = input.usesPLSDstRead();
}

bool GrDrawOp::installPipeline(const GrPipeline::CreateArgs& args) {
    GrPipelineOptimizations optimizations;
    void* location = fPipelineStorage.get();
    if (!GrPipeline::CreateAt(location, args, &optimizations)) {
        return false;
    }
    fPipelineInstalled = true;
    this->applyPipelineOptimizations(optimizations);
    return true;
}
