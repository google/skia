/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrReorderCommandBuilder_DEFINED
#define GrReorderCommandBuilder_DEFINED

#include "GrCommandBuilder.h"

class GrReorderCommandBuilder : public GrCommandBuilder {
public:
    typedef GrCommandBuilder::Cmd Cmd;
    typedef GrCommandBuilder::State State;

    GrReorderCommandBuilder(GrGpu* gpu) : INHERITED(gpu) {}

    Cmd* recordDrawBatch(State*, GrBatch*) override;
    Cmd* recordStencilPath(const GrPipelineBuilder&,
                           const GrPathProcessor*,
                           const GrPath*,
                           const GrScissorState&,
                           const GrStencilSettings&) override {
        SkFAIL("Unsupported\n");
        return NULL;
    }

    Cmd* recordDrawPath(State*,
                        const GrPathProcessor*,
                        const GrPath*,
                        const GrStencilSettings&) override {
        SkFAIL("Unsupported\n");
        return NULL;
    }

    Cmd* recordDrawPaths(State*,
                         GrBufferedDrawTarget*,
                         const GrPathProcessor*,
                         const GrPathRange*,
                         const void*,
                         GrDrawTarget::PathIndexType,
                         const float transformValues[],
                         GrDrawTarget::PathTransformType ,
                         int,
                         const GrStencilSettings&,
                         const GrDrawTarget::PipelineInfo&) override {
        SkFAIL("Unsupported\n");
        return NULL;
    }

private:
    typedef GrCommandBuilder INHERITED;

};

#endif
