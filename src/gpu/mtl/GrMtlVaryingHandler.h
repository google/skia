/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrMtlVaryingHandler_DEFINED
#define GrMtlVaryingHandler_DEFINED

#include "src/gpu/glsl/GrGLSLVarying.h"

class GrMtlVaryingHandler : public GrGLSLVaryingHandler {
public:
    GrMtlVaryingHandler(GrGLSLProgramBuilder* program) : INHERITED(program) {}

    typedef GrGLSLVaryingHandler::VarArray VarArray;

private:
    void onFinalize() override;

    friend class GrMtlPipelineStateBuilder;

    typedef GrGLSLVaryingHandler INHERITED;
};

#endif
