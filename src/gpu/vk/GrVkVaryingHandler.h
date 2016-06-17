/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkVaryingHandler_DEFINED
#define GrVkVaryingHandler_DEFINED

#include "glsl/GrGLSLVarying.h"

class GrVkVaryingHandler : public GrGLSLVaryingHandler {
public:
    GrVkVaryingHandler(GrGLSLProgramBuilder* program) : INHERITED(program) {}

    typedef GrGLSLVaryingHandler::VarArray VarArray;

private:
    void onFinalize() override;

    friend class GrVkPipelineStateBuilder;

    typedef GrGLSLVaryingHandler INHERITED;
};

#endif
