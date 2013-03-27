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

class GrGpuGL;

// optionally compile the experimental GS code. Set to GR_DEBUG so that debug build bots will
// execute the code.
#define GR_GL_EXPERIMENTAL_GS GR_DEBUG


/** This class describes a program to generate. It also serves as a program cache key. The only
    thing GL-specific about this is the generation of GrGLEffect::EffectKeys. With some refactoring
    it could be made backend-neutral. */
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
                   const GrEffectStage stages[GrDrawState::kNumStages]);

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

    // should the FS discard if the coverage is zero (to avoid stencil manipulation)
    bool                        fDiscardIfZeroCoverage;

    // stripped of bits that don't affect program generation
    GrAttribBindings            fAttribBindings;

    /** Non-zero if this stage has an effect */
    GrGLEffect::EffectKey       fEffectKeys[GrDrawState::kNumStages];

    // To enable experimental geometry shader code (not for use in
    // production)
#if GR_GL_EXPERIMENTAL_GS
    bool                        fExperimentalGS;
#endif
    uint8_t                     fColorInput;            // casts to enum ColorInput
    uint8_t                     fCoverageInput;         // casts to enum ColorInput
    uint8_t                     fDualSrcOutput;         // casts to enum DualSrcOutput
    int8_t                      fFirstCoverageStage;
    SkBool8                     fEmitsPointSize;
    uint8_t                     fColorFilterXfermode;   // casts to enum SkXfermode::Mode

    int8_t                      fPositionAttributeIndex;
    int8_t                      fColorAttributeIndex;
    int8_t                      fCoverageAttributeIndex;
    int8_t                      fLocalCoordsAttributeIndex;

    // GrGLProgram reads the private fields to generate code.
    friend class GrGLProgram;
};

#endif
