/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLColorSpaceXformHelper_DEFINED
#define GrGLSLColorSpaceXformHelper_DEFINED

#include "GrColorSpaceXform.h"
#include "GrGLSLUniformHandler.h"

/**
 * Stack helper class to assist with using GrColorSpaceXform within an FP's emitCode function.
 * This injects the uniform declaration, and stores the information needed to generate correct
 * gamut-transformation shader code.
 */
class GrGLSLColorSpaceXformHelper : public SkNoncopyable {
public:
    GrGLSLColorSpaceXformHelper(GrGLSLUniformHandler* uniformHandler,
                                GrColorSpaceXform* colorSpaceXform,
                                GrGLSLProgramDataManager::UniformHandle* handle) {
        SkASSERT(uniformHandler && handle);
        if (colorSpaceXform) {
            *handle = uniformHandler->addUniform(kFragment_GrShaderFlag, kMat44f_GrSLType,
                                                 kDefault_GrSLPrecision, "ColorXform",
                                                 &fXformMatrix);
        } else {
            fXformMatrix = nullptr;
        }
    }

    const char* getXformMatrix() const { return fXformMatrix; }

private:
    const char* fXformMatrix;
};

#endif
