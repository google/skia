/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLVaryingHandler_DEFINED
#define GrGLVaryingHandler_DEFINED

#include "include/private/GrTypesPriv.h"
#include "src/gpu/gl/GrGLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLVarying.h"

class GrGLVaryingHandler : public GrGLSLVaryingHandler {
public:
    GrGLVaryingHandler(GrGLSLProgramBuilder* program) : INHERITED(program) {}

private:
    void onFinalize() override {}

    friend class GrGLProgramBuilder;

    using INHERITED = GrGLSLVaryingHandler;
};

#endif
