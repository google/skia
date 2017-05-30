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
 * Helper class to assist with using GrColorSpaceXform within an FP. This manages all of the
 * uniforms needed, and can be passed to shader builder functions to automatically generate the
 * correct color space transformation code.
 */
class GrGLSLColorSpaceXformHelper : public SkNoncopyable {
public:
    GrGLSLColorSpaceXformHelper() : fValid(false) {}

    void emitCode(GrGLSLUniformHandler* uniformHandler, GrColorSpaceXform* colorSpaceXform,
                  uint32_t visibility = kFragment_GrShaderFlag) {
        SkASSERT(uniformHandler);
        if (colorSpaceXform) {
            fGamutXformVar = uniformHandler->addUniform(visibility, kMat44f_GrSLType,
                                                        kDefault_GrSLPrecision, "ColorXform");
            fValid = true;
        }
    }

    void setData(const GrGLSLProgramDataManager& pdman, GrColorSpaceXform* colorSpaceXform) {
        pdman.setSkMatrix44(fGamutXformVar, colorSpaceXform->srcToDst());
    }

    bool isValid() const { return fValid; }
    GrGLSLProgramDataManager::UniformHandle const gamutXformUniform() { return fGamutXformVar; }

private:
    GrGLSLProgramDataManager::UniformHandle fGamutXformVar;
    bool fValid;
};

#endif
