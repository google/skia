/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawOp.h"

#include "GrRenderTarget.h"

SkString GrDrawOp::DumpPipelineInfo(const GrPipeline& pipeline) {
    SkString string;
    string.appendf("RT: %d\n", pipeline.getRenderTarget()->uniqueID().asUInt());
    string.append("ColorStages:\n");
    for (int i = 0; i < pipeline.numColorFragmentProcessors(); i++) {
        string.appendf("\t\t%s\n\t\t%s\n",
                        pipeline.getColorFragmentProcessor(i).name(),
                        pipeline.getColorFragmentProcessor(i).dumpInfo().c_str());
    }
    string.append("CoverageStages:\n");
    for (int i = 0; i < pipeline.numCoverageFragmentProcessors(); i++) {
        string.appendf("\t\t%s\n\t\t%s\n",
                        pipeline.getCoverageFragmentProcessor(i).name(),
                        pipeline.getCoverageFragmentProcessor(i).dumpInfo().c_str());
    }
    string.appendf("XP: %s\n", pipeline.getXferProcessor().name());

    bool scissorEnabled = pipeline.getScissorState().enabled();
    string.appendf("Scissor: ");
    if (scissorEnabled) {
        string.appendf("[L: %d, T: %d, R: %d, B: %d]\n",
                        pipeline.getScissorState().rect().fLeft,
                        pipeline.getScissorState().rect().fTop,
                        pipeline.getScissorState().rect().fRight,
                        pipeline.getScissorState().rect().fBottom);
    } else {
        string.appendf("<disabled>\n");
    }
    return string;
}
