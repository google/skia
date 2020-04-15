/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DVaryingHandler_DEFINED
#define GrD3DVaryingHandler_DEFINED

#include "src/gpu/glsl/GrGLSLVarying.h"

/**
 * NOTICE: This class is currently implemented with the assumption that we are generating spir-v.
 * This will need to be updated once we change to using HLSL or DXIL.
 */

class GrD3DVaryingHandler : public GrGLSLVaryingHandler {
public:
    GrD3DVaryingHandler(GrGLSLProgramBuilder* program) : INHERITED(program) {}

    typedef GrGLSLVaryingHandler::VarArray VarArray;

private:
    void onFinalize() override;

    friend class GrD3DPipelineStateBuilder;

    typedef GrGLSLVaryingHandler INHERITED;
};

#endif
