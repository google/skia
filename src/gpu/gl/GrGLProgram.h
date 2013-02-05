/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLProgram_DEFINED
#define GrGLProgram_DEFINED

#include "GrDrawState.h"
#include "GrGLEffect.h"
#include "GrGLContextInfo.h"
#include "GrGLSL.h"
#include "GrGLTexture.h"
#include "GrGLUniformManager.h"

#include "SkString.h"
#include "SkXfermode.h"

class GrBinHashKeyBuilder;
class GrGLEffect;
class GrGLShaderBuilder;

// optionally compile the experimental GS code. Set to GR_DEBUG
// so that debug build bots will execute the code.
#define GR_GL_EXPERIMENTAL_GS GR_DEBUG

/**
 * This class manages a GPU program and records per-program information.
 * We can specify the attribute locations so that they are constant
 * across our shaders. But the driver determines the uniform locations
 * at link time. We don't need to remember the sampler uniform location
 * because we will bind a texture slot to it and never change it
 * Uniforms are program-local so we can't rely on fHWState to hold the
 * previous uniform state after a program change.
 */
class GrGLProgram : public GrRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrGLProgram)

    struct Desc;

    static GrGLProgram* Create(const GrGLContextInfo& gl,
                               const Desc& desc,
                               const GrEffectStage* stages[]);

    virtual ~GrGLProgram();

    /**
     * Call to abandon GL objects owned by this program.
     */
    void abandon();

    /**
     * The shader may modify the blend coefficients. Params are in/out
     */
    void overrideBlend(GrBlendCoeff* srcCoeff, GrBlendCoeff* dstCoeff) const;

    const Desc& getDesc() { return fDesc; }

    /**
     * Attribute indices. These should not overlap.
     */
    static int PositionAttributeIdx() { return 0; }
    static int ColorAttributeIdx() { return 1; }
    static int CoverageAttributeIdx() { return 2; }
    static int EdgeAttributeIdx() { return 3; }
    static int TexCoordAttributeIdx(int tcIdx) { return 4 + tcIdx; }

    /**
     * This function uploads uniforms and calls each GrGLEffect's setData. It is called before a
     * draw occurs using the program after the program has already been bound. It also uses the
     * GrGpuGL object to bind the textures required by the GrGLEffects.
     */
    void setData(GrGpuGL*);

    // Parameters that affect code generation
    // This structs should be kept compact; it is input to an expensive hash key generator.
    struct Desc {
        Desc() {
            // since we use this as part of a key we can't have any uninitialized
            // padding
            memset(this, 0, sizeof(Desc));
        }

        // returns this as a uint32_t array to be used as a key in the program cache
        const uint32_t* asKey() const {
            return reinterpret_cast<const uint32_t*>(this);
        }

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

        // TODO: remove these two members when edge-aa can be rewritten as a GrEffect.
        GrDrawState::VertexEdgeType fVertexEdgeType;
        // should the FS discard if the edge-aa coverage is zero (to avoid stencil manipulation)
        bool                        fDiscardIfOutsideEdge;

        // stripped of bits that don't affect program generation
        GrVertexLayout              fVertexLayout;

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
    };
private:
    GrGLProgram(const GrGLContextInfo& gl,
                const Desc& desc,
                const GrEffectStage* stages[]);

    bool succeeded() const { return 0 != fProgramID; }

    /**
     *  This is the heavy initialization routine for building a GLProgram.
     */
    bool genProgram(const GrEffectStage* stages[]);

    void genInputColor(GrGLShaderBuilder* builder, SkString* inColor);

    void genGeometryShader(GrGLShaderBuilder* segments) const;

    typedef GrGLUniformManager::UniformHandle UniformHandle;

    void genUniformCoverage(GrGLShaderBuilder* segments, SkString* inOutCoverage);

    // generates code to compute coverage based on edge AA. Returns true if edge coverage was
    // inserted in which case coverageVar will be updated to refer to a scalar. Otherwise,
    // coverageVar is set to an empty string.
    bool genEdgeCoverage(SkString* coverageVar, GrGLShaderBuilder* builder) const;

    // Creates a GL program ID, binds shader attributes to GL vertex attrs, and links the program
    bool bindOutputsAttribsAndLinkProgram(const GrGLShaderBuilder& builder,
                                          SkString texCoordAttrNames[GrDrawState::kMaxTexCoords],
                                          bool bindColorOut,
                                          bool bindDualSrcOut);

    // Sets the texture units for samplers
    void initSamplerUniforms();

    bool compileShaders(const GrGLShaderBuilder& builder);

    const char* adjustInColor(const SkString& inColor) const;

    typedef SkSTArray<4, UniformHandle, true> SamplerUniSArray;

    struct UniformHandles {
        UniformHandle       fViewMatrixUni;
        UniformHandle       fColorUni;
        UniformHandle       fCoverageUni;
        UniformHandle       fColorFilterUni;
        // We use the render target height to provide a y-down frag coord when specifying
        // origin_upper_left is not supported.
        UniformHandle       fRTHeightUni;
        // An array of sampler uniform handles for each effect.
        SamplerUniSArray    fSamplerUnis[GrDrawState::kNumStages];

        UniformHandles() {
            fViewMatrixUni = GrGLUniformManager::kInvalidUniformHandle;
            fColorUni = GrGLUniformManager::kInvalidUniformHandle;
            fCoverageUni = GrGLUniformManager::kInvalidUniformHandle;
            fColorFilterUni = GrGLUniformManager::kInvalidUniformHandle;
            fRTHeightUni = GrGLUniformManager::kInvalidUniformHandle;
        }
    };

    // GL IDs
    GrGLuint                    fVShaderID;
    GrGLuint                    fGShaderID;
    GrGLuint                    fFShaderID;
    GrGLuint                    fProgramID;
    // The matrix sent to GL is determined by both the client's matrix and
    // the size of the viewport.
    SkMatrix                    fViewMatrix;
    SkISize                     fViewportSize;

    // these reflect the current values of uniforms (GL uniform values travel with program)
    GrColor                     fColor;
    GrColor                     fCoverage;
    GrColor                     fColorFilterColor;
    int                         fRTHeight;

    GrGLEffect*                 fEffects[GrDrawState::kNumStages];

    Desc                        fDesc;
    const GrGLContextInfo&      fContextInfo;

    GrGLUniformManager          fUniformManager;
    UniformHandles              fUniformHandles;

    friend class GrGpuGL; // TODO: remove this by adding getters and moving functionality.

    typedef GrRefCnt INHERITED;
};

#endif
