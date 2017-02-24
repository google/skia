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

bool GrDrawOp::installPipeline(const GrPipeline::InitArgs& args) {
    GrPipelineOptimizations optimizations;
    GrPipeline* pipeline = new (fPipelineStorage.get()) GrPipeline();
    if (!pipeline->init(args, &optimizations)) {
        return false;
    }
    fPipelineInstalled = true;
    this->applyPipelineOptimizations(optimizations);
    return true;
}
