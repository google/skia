/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSpirvVaryingHandler_DEFINED
#define GrSpirvVaryingHandler_DEFINED

#include "src/gpu/glsl/GrGLSLVarying.h"

 /*
  * This class can be used for basic spir-v varying handling. Each added varying just takes the
  * next available location slot.
  */
class GrSpirvVaryingHandler : public GrGLSLVaryingHandler {
public:
    GrSpirvVaryingHandler(GrGLSLProgramBuilder* program) : INHERITED(program) {}

    typedef GrGLSLVaryingHandler::VarArray VarArray;

private:
    void onFinalize() override;

    typedef GrGLSLVaryingHandler INHERITED;
};

#endif
