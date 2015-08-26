/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPendingFragmentStage_DEFINED
#define GrPendingFragmentStage_DEFINED

#include "GrStagedProcessor.h"
#include "GrPendingProgramElement.h"

/**
 * This a baked variant of GrFragmentStage, as recorded in GrPipeline
 */
class GrPendingFragmentStage : public GrStagedProcessor<GrPendingProgramElement> {
public:
    GrPendingFragmentStage(const GrFragmentStage& stage) {
        INHERITED::fProc.reset(stage.processor());
    }

private:
    typedef GrStagedProcessor<GrPendingProgramElement> INHERITED;
};

#endif
