/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnVaryingHandler_DEFINED
#define GrDawnVaryingHandler_DEFINED

#include "src/gpu/glsl/GrGLSLVarying.h"

class GrDawnVaryingHandler : public GrGLSLVaryingHandler {
public:
    GrDawnVaryingHandler(GrGLSLProgramBuilder* program) : INHERITED(program) {}

    typedef GrGLSLVaryingHandler::VarArray VarArray;

private:
    void onFinalize() override;

    friend class GrDawnPipelineStateBuilder;

    typedef GrGLSLVaryingHandler INHERITED;
};

#endif
