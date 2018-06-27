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
    GrGLSLColorSpaceXformHelper() : fFlags(0) {}

    void emitCode(GrGLSLUniformHandler* uniformHandler, const GrColorSpaceXform* colorSpaceXform,
                  uint32_t visibility = kFragment_GrShaderFlag) {
        SkASSERT(uniformHandler);
        if (colorSpaceXform) {
            fFlags = colorSpaceXform->fFlags;
            if (this->applyGamutXform()) {
                fGamutXformVar = uniformHandler->addUniform(visibility,
                                                            kHalf4x4_GrSLType,
                                                            "ColorXform");
            }
            if (this->applyTransferFn()) {
                fTransferFnVar = uniformHandler->addUniformArray(visibility,
                                                                 kHalf_GrSLType,
                                                                 "TransferFn",
                                                                 kNumTransferFnCoeffs);
            }
        }
    }

    void setData(const GrGLSLProgramDataManager& pdman, const GrColorSpaceXform* colorSpaceXform) {
        if (this->applyGamutXform()) {
            pdman.setSkMatrix44(fGamutXformVar, colorSpaceXform->gamutXform());
        }
        if (this->applyTransferFn()) {
            pdman.set1fv(fTransferFnVar, kNumTransferFnCoeffs, colorSpaceXform->transferFnCoeffs());
        }
    }

    bool isValid() const { return (0 != fFlags); }
    bool applyInverseSRGB() const {
        return SkToBool(fFlags & GrColorSpaceXform::kApplyInverseSRGB_Flag);
    }
    bool applyTransferFn() const {
        return SkToBool(fFlags & GrColorSpaceXform::kApplyTransferFn_Flag);
    }
    bool applyGamutXform() const {
        return SkToBool(fFlags & GrColorSpaceXform::kApplyGamutXform_Flag);
    }

    GrGLSLProgramDataManager::UniformHandle gamutXformUniform() const { return fGamutXformVar; }
    GrGLSLProgramDataManager::UniformHandle transferFnUniform() const { return fTransferFnVar; }

private:
    static const int kNumTransferFnCoeffs = 7;

    GrGLSLProgramDataManager::UniformHandle fGamutXformVar;
    GrGLSLProgramDataManager::UniformHandle fTransferFnVar;
    uint32_t fFlags;
};

#endif
