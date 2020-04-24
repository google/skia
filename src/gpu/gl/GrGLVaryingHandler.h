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
    GrGLVaryingHandler(GrGLSLProgramBuilder* program)
        : INHERITED(program),
        fPathProcVaryingInfos(kVaryingsPerBlock) {}

    // This function is used by the NVPR PathProcessor to add a varying directly into the fragment
    // shader since there is no vertex shader.
    VaryingHandle addPathProcessingVarying(const char* name, GrGLSLVarying*);

private:
    void onFinalize() override;

    GrGLProgramDataManager::VaryingInfoArray fPathProcVaryingInfos;

    friend class GrGLProgramBuilder;

    typedef GrGLSLVaryingHandler INHERITED;
};

#endif
