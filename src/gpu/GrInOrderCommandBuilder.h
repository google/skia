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

    GrInOrderCommandBuilder() : INHERITED() { }

    Cmd* recordDrawBatch(GrBatch*, const GrCaps&) override;
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
                         const GrPipelineOptimizations&) override;

private:
    typedef GrCommandBuilder INHERITED;

};

#endif
