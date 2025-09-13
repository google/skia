/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLColorSpaceXformHelper_DEFINED
#define GrGLSLColorSpaceXformHelper_DEFINED

#include "modules/skcms/skcms.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"

/**
 * Helper class to assist with using GrColorSpaceXform within an FP. This manages all of the
 * uniforms needed, and can be passed to shader builder functions to automatically generate the
 * correct color space transformation code.
 */
class GrGLSLColorSpaceXformHelper : public SkNoncopyable {
public:
    GrGLSLColorSpaceXformHelper() {}

    void emitCode(GrGLSLUniformHandler* uniformHandler, const GrColorSpaceXform* colorSpaceXform,
                  uint32_t visibility = kFragment_GrShaderFlag) {
        SkASSERT(uniformHandler);
        if (colorSpaceXform) {
            fFlags = colorSpaceXform->fSteps.fFlags;
            if (this->applySrcTF()) {
                fSrcTFVar = uniformHandler->addUniformArray(nullptr, visibility, SkSLType::kFloat,
                                                            "SrcTF", kNumTransferFnCoeffs);
                fSrcTFType = skcms_TransferFunction_getType(&colorSpaceXform->fSteps.fSrcTF);
            }
            if (this->applySrcOOTF()) {
                fSrcOOTFVar = uniformHandler->addUniform(nullptr, visibility, SkSLType::kFloat4,
                                                         "SrcOOTF");
            }
            if (this->applyGamutXform()) {
                fGamutXformVar = uniformHandler->addUniform(nullptr, visibility, SkSLType::kFloat3x3,
                                                            "ColorXform");
            }
            if (this->applyDstOOTF()) {
                fDstOOTFVar = uniformHandler->addUniform(nullptr, visibility, SkSLType::kFloat4,
                                                         "DstOOTF");
            }
            if (this->applyDstTF()) {
                fDstTFVar = uniformHandler->addUniformArray(nullptr, visibility, SkSLType::kFloat,
                                                            "DstTF", kNumTransferFnCoeffs);
                fDstTFType = skcms_TransferFunction_getType(&colorSpaceXform->fSteps.fDstTFInv);
            }
        }
    }

    void setData(const GrGLSLProgramDataManager& pdman, const GrColorSpaceXform* colorSpaceXform) {
        if (this->applySrcTF()) {
            pdman.set1fv(fSrcTFVar, kNumTransferFnCoeffs, &colorSpaceXform->fSteps.fSrcTF.g);
        }
        if (this->applySrcOOTF()) {
            pdman.set4fv(fSrcOOTFVar, 1, colorSpaceXform->fSteps.fSrcOotf);
        }
        if (this->applyGamutXform()) {
            pdman.setMatrix3f(fGamutXformVar, colorSpaceXform->fSteps.fSrcToDstMatrix);
        }
        if (this->applyDstOOTF()) {
            pdman.set4fv(fDstOOTFVar, 1, colorSpaceXform->fSteps.fDstOotf);
        }
        if (this->applyDstTF()) {
            pdman.set1fv(fDstTFVar, kNumTransferFnCoeffs, &colorSpaceXform->fSteps.fDstTFInv.g);
        }
    }

    bool isNoop() const { return (0 == fFlags.mask()); }

    bool applyUnpremul() const   { return fFlags.unpremul; }
    bool applySrcTF() const      { return fFlags.linearize; }
    bool applySrcOOTF() const    { return fFlags.src_ootf; }
    bool applyGamutXform() const { return fFlags.gamut_transform; }
    bool applyDstOOTF() const    { return fFlags.dst_ootf; }
    bool applyDstTF() const      { return fFlags.encode; }
    bool applyPremul() const     { return fFlags.premul; }

    skcms_TFType srcTFType() const { return fSrcTFType; }
    skcms_TFType dstTFType() const { return fDstTFType; }

    GrGLSLProgramDataManager::UniformHandle srcTFUniform() const { return fSrcTFVar; }
    GrGLSLProgramDataManager::UniformHandle srcOOTFUniform() const { return fSrcOOTFVar; }
    GrGLSLProgramDataManager::UniformHandle gamutXformUniform() const { return fGamutXformVar; }
    GrGLSLProgramDataManager::UniformHandle dstOOTFUniform() const { return fDstOOTFVar; }
    GrGLSLProgramDataManager::UniformHandle dstTFUniform() const { return fDstTFVar; }

private:
    static const int kNumTransferFnCoeffs = 7;

    GrGLSLProgramDataManager::UniformHandle fSrcTFVar;
    GrGLSLProgramDataManager::UniformHandle fSrcOOTFVar;
    GrGLSLProgramDataManager::UniformHandle fGamutXformVar;
    GrGLSLProgramDataManager::UniformHandle fDstOOTFVar;
    GrGLSLProgramDataManager::UniformHandle fDstTFVar;
    SkColorSpaceXformSteps::Flags fFlags;
    skcms_TFType fSrcTFType;
    skcms_TFType fDstTFType;
};

#endif
