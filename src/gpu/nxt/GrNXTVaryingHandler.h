/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNXTVaryingHandler_DEFINED
#define GrNXTVaryingHandler_DEFINED

#include "glsl/GrGLSLVarying.h"

class GrNXTVaryingHandler : public GrGLSLVaryingHandler {
public:
    GrNXTVaryingHandler(GrGLSLProgramBuilder* program) : INHERITED(program) {}

    typedef GrGLSLVaryingHandler::VarArray VarArray;

private:
    void onFinalize() override;

    friend class GrNXTPipelineStateBuilder;

    typedef GrGLSLVaryingHandler INHERITED;
};

#endif
