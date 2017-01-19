/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPipelineBuilder.h"

#include "GrBlend.h"
#include "GrPaint.h"
#include "GrPipeline.h"
#include "GrProcOptInfo.h"
#include "GrXferProcessor.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "ops/GrOp.h"

GrPipelineBuilder::GrPipelineBuilder(GrPaint&& paint, GrAAType aaType)
        : fFlags(0x0)
        , fDrawFace(GrDrawFace::kBoth)
        , fUserStencilSettings(&GrUserStencilSettings::kUnused)
        , fProcessors(std::move(paint)) {
    if (GrAATypeIsHW(aaType)) {
        fFlags |= GrPipeline::kHWAntialias_Flag;
    }
}

bool GrPipelineBuilder::willXPNeedDstTexture(const GrCaps& caps,
                                             const GrPipelineAnalysis& analysis) const {
    if (fProcessors.xpFactory()) {
        return fProcessors.xpFactory()->willNeedDstTexture(caps, analysis);
    }
    return GrPorterDuffXPFactory::SrcOverWillNeedDstTexture(caps, analysis);
}
