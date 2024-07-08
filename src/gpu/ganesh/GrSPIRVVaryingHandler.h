/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSPIRVVaryingHandler_DEFINED
#define GrSPIRVVaryingHandler_DEFINED

#include "src/gpu/ganesh/glsl/GrGLSLVarying.h"

class GrGLSLProgramBuilder;
/*
 * This class can be used for basic SPIR-V varying handling. Each added varying just takes the
 * next available location slot.
 */
class GrSPIRVVaryingHandler : public GrGLSLVaryingHandler {
public:
    GrSPIRVVaryingHandler(GrGLSLProgramBuilder* program) : INHERITED(program) {}

    typedef GrGLSLVaryingHandler::VarArray VarArray;

private:
    void onFinalize() override;

    using INHERITED = GrGLSLVaryingHandler;
};

#endif
