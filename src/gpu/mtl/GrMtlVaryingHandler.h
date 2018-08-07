/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrMtlVaryingHandler_DEFINED
#define GrMtlVaryingHandler_DEFINED

#include "glsl/GrGLSLVarying.h"

// TODO: this class is basically copy and pasted from GrVkVaryingHandler so that we can have
// some shaders working. The SkSL Metal code generator was written to work with GLSL generated for
// the Ganesh Vulkan backend, so it should all work. There might be better ways to do things in
// Metal and/or some Vulkan GLSLisms left in.
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
