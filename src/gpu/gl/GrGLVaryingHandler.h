/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLVaryingHandler_DEFINED
#define GrGLVaryingHandler_DEFINED

#include "glsl/GrGLSLVarying.h"
#include "GrTypesPriv.h"
#include "gl/GrGLProgramDataManager.h"

class GrGLVaryingHandler : public GrGLSLVaryingHandler {
public:
    GrGLVaryingHandler(GrGLSLProgramBuilder* program)
        : INHERITED(program),
        fPathProcVaryingInfos(kVaryingsPerBlock) {}

    // This function is used by the NVPR PathProcessor to add a varying directly into the fragment
    // shader since there is no vertex shader.
    VaryingHandle addPathProcessingVarying(const char* name, GrGLSLVertToFrag*,
                                           GrSLPrecision fsPrecision = kDefault_GrSLPrecision);

private:
    typedef GrGLProgramDataManager::VaryingInfo VaryingInfo;
    typedef GrGLProgramDataManager::VaryingInfoArray VaryingInfoArray;

    VaryingInfoArray fPathProcVaryingInfos;

    friend class GrGLProgramBuilder;

    typedef GrGLSLVaryingHandler INHERITED; 
};

#endif

