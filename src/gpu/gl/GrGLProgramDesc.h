/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLProgramDesc_DEFINED
#define GrGLProgramDesc_DEFINED

#include "GrGLEffect.h"
#include "GrDrawState.h"
#include "GrGLShaderBuilder.h"

class GrGpuGL;

// optionally compile the experimental GS code. Set to GR_DEBUG so that debug build bots will
// execute the code.
#define GR_GL_EXPERIMENTAL_GS GR_DEBUG


/** This class describes a program to generate. It also serves as a program cache key. Very little
    of this is GL-specific. There is the generation of GrGLEffect::EffectKeys and the dst-read part
    of the key set by GrGLShaderBuilder. If the interfaces that set those portions were abstracted
    to be API-neutral then so could this class. */
class GrGLProgramDesc {
public:
    GrGLProgramDesc() {
        // since we use this as part of a key we can't have any uninitialized padding
        memset(this, 0, sizeof(GrGLProgramDesc));
    }

    // Returns this as a uint32_t array to be used as a key in the program cache
    const uint32_t* asKey() const {
        return reinterpret_cast<const uint32_t*>(this);
    }

    // For unit testing.
    void setRandom(SkMWCRandom*,
                   const GrGpuGL* gpu,
                   const GrTexture* dummyDstTexture,
                   const GrEffectStage stages[GrDrawState::kNumStages],
                   int currAttribIndex);

    /**
     * Builds a program descriptor from a GrDrawState. Whether the primitive type is points, the
     * output of GrDrawState::getBlendOpts, and the caps of the GrGpuGL are also inputs.
     */
    static void Build(const GrDrawState&,
                      bool isPoints,
                      GrDrawState::BlendOptFlags,
                      GrBlendCoeff srcCoeff,
                      GrBlendCoeff dstCoeff,
                      const GrGpuGL* gpu,
                      const GrDeviceCoordTexture* dstCopy,
                      GrGLProgramDesc* outDesc);

private:
    // Specifies where the initial color comes from before the stages are applied.
    enum ColorInput {
        kSolidWhite_ColorInput,
        kTransBlack_ColorInput,
        kAttribute_ColorInput,
        kUniform_ColorInput,

        kColorInputCnt
    };
    // Dual-src blending makes use of a secondary output color that can be
    // used as a per-pixel blend coefficient. This controls whether a
    // secondary source is output and what value it holds.
    enum DualSrcOutput {
        kNone_DualSrcOutput,
        kCoverage_DualSrcOutput,
        kCoverageISA_DualSrcOutput,
        kCoverageISC_DualSrcOutput,

        kDualSrcOutputCnt
    };

    /** Non-zero if this stage has an effect */
    GrGLEffect::EffectKey       fEffectKeys[GrDrawState::kNumStages];

    // To enable experimental geometry shader code (not for use in
    // production)
#if GR_GL_EXPERIMENTAL_GS
    bool                     fExperimentalGS;
#endif

    GrGLShaderBuilder::DstReadKey fDstRead;             // set by GrGLShaderBuilder if there
                                                        // are effects that must read the dst.
                                                        // Otherwise, 0.

    // should the FS discard if the coverage is zero (to avoid stencil manipulation)
    SkBool8                     fDiscardIfZeroCoverage;

    uint8_t                     fColorInput;            // casts to enum ColorInput
    uint8_t                     fCoverageInput;         // casts to enum ColorInput
    uint8_t                     fDualSrcOutput;         // casts to enum DualSrcOutput

    int8_t                      fFirstCoverageStage;
    SkBool8                     fEmitsPointSize;
    uint8_t                     fColorFilterXfermode;   // casts to enum SkXfermode::Mode

    int8_t                      fPositionAttributeIndex;
    int8_t                      fLocalCoordAttributeIndex;
    int8_t                      fColorAttributeIndex;
    int8_t                      fCoverageAttributeIndex;

    // GrGLProgram and GrGLShaderBuilder read the private fields to generate code. TODO: Move all
    // code generation to GrGLShaderBuilder (and maybe add getters rather than friending).
    friend class GrGLProgram;
    friend class GrGLShaderBuilder;
};

#endif
