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
#include "SkColorSpaceXformSteps.h"

/**
 * Helper class to assist with using GrColorSpaceXform within an FP. This manages all of the
 * uniforms needed, and can be passed to shader builder functions to automatically generate the
 * correct color space transformation code.
 */
class GrGLSLColorSpaceXformHelper : public SkNoncopyable {
public:
    GrGLSLColorSpaceXformHelper() {
        memset(&fFlags, 0, sizeof(fFlags));
    }

    void emitCode(GrGLSLUniformHandler* uniformHandler, const GrColorSpaceXform* colorSpaceXform,
                  uint32_t visibility = kFragment_GrShaderFlag) {
        SkASSERT(uniformHandler);
        if (colorSpaceXform) {
            fFlags = colorSpaceXform->fSteps.flags;
            if (this->applySrcTF()) {
                fSrcTFVar = uniformHandler->addUniformArray(visibility, kHalf_GrSLType,
                                                            "SrcTF", kNumTransferFnCoeffs);
            }
            if (this->applyGamutXform()) {
                fGamutXformVar = uniformHandler->addUniform(visibility, kHalf3x3_GrSLType,
                                                            "ColorXform");
            }
            if (this->applyDstTF()) {
                fDstTFVar = uniformHandler->addUniformArray(visibility, kHalf_GrSLType,
                                                            "DstTF", kNumTransferFnCoeffs);
            }
        }
    }

    void setData(const GrGLSLProgramDataManager& pdman, const GrColorSpaceXform* colorSpaceXform) {
        if (this->applySrcTF()) {
            pdman.set1fv(fSrcTFVar, kNumTransferFnCoeffs, &colorSpaceXform->fSteps.srcTF.g);
        }
        if (this->applyGamutXform()) {
            pdman.setMatrix3f(fGamutXformVar, colorSpaceXform->fSteps.src_to_dst_matrix);
        }
        if (this->applyDstTF()) {
            pdman.set1fv(fDstTFVar, kNumTransferFnCoeffs, &colorSpaceXform->fSteps.dstTFInv.g);
        }
    }

    bool isNoop() const { return (0 == fFlags.mask()); }

    bool applyUnpremul() const   { return fFlags.unpremul; }
    bool applySrcTF() const      { return fFlags.linearize; }
    bool applyGamutXform() const { return fFlags.gamut_transform; }
    bool applyDstTF() const      { return fFlags.encode; }
    bool applyPremul() const     { return fFlags.premul; }

    GrGLSLProgramDataManager::UniformHandle srcTFUniform() const { return fSrcTFVar; }
    GrGLSLProgramDataManager::UniformHandle gamutXformUniform() const { return fGamutXformVar; }
    GrGLSLProgramDataManager::UniformHandle dstTFUniform() const { return fDstTFVar; }

private:
    static const int kNumTransferFnCoeffs = 7;

    GrGLSLProgramDataManager::UniformHandle fSrcTFVar;
    GrGLSLProgramDataManager::UniformHandle fGamutXformVar;
    GrGLSLProgramDataManager::UniformHandle fDstTFVar;
    SkColorSpaceXformSteps::Flags fFlags;
};

#endif
