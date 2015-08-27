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

    GrReorderCommandBuilder() : INHERITED() {}

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
                         const GrPipelineOptimizations&) override {
        SkFAIL("Unsupported\n");
        return nullptr;
    }

private:
    typedef GrCommandBuilder INHERITED;

};

#endif
