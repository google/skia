/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrInOrderCommandBuilder_DEFINED
#define GrInOrderCommandBuilder_DEFINED

#include "GrCommandBuilder.h"

class GrInOrderCommandBuilder : public GrCommandBuilder {
public:
    typedef GrCommandBuilder::Cmd Cmd;
    typedef GrCommandBuilder::State State;

    GrInOrderCommandBuilder(GrGpu* gpu) : INHERITED(gpu) { }

    Cmd* recordDrawBatch(State*, GrBatch*) override;
    Cmd* recordStencilPath(const GrPipelineBuilder&,
                           const GrPathProcessor*,
                           const GrPath*,
                           const GrScissorState&,
                           const GrStencilSettings&) override;
    Cmd* recordDrawPath(State*,
                        const GrPathProcessor*,
                        const GrPath*,
                        const GrStencilSettings&) override;
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
                         const GrDrawTarget::PipelineInfo&) override;

private:
    typedef GrCommandBuilder INHERITED;

};

#endif
