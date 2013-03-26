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
#include "GrGLContext.h"
#include "GrGLSL.h"
#include "GrGLTexture.h"
#include "GrGLUniformManager.h"

#include "SkString.h"
#include "SkXfermode.h"

class GrBinHashKeyBuilder;
class GrGLEffect;
class GrGLShaderBuilder;
class SkMWCRandom;

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

    class Desc;

    /**
     * Builds a program descriptor from a GrDrawState. Whether the primitive type is points, the
     * output of GrDrawState::getBlendOpts, and the caps of the GrGpuGL are also inputs.
     */
    static void BuildDesc(const GrDrawState&,
                          bool isPoints,
                          GrDrawState::BlendOptFlags,
                          GrBlendCoeff srcCoeff,
                          GrBlendCoeff dstCoeff,
                          const GrGpuGL* gpu,
                          Desc* outDesc);

    static GrGLProgram* Create(const GrGLContext& gl,
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
     * Gets the GL program ID for this program.
     */
    GrGLuint programID() const { return fProgramID; }

    /**
     * Some GL state that is relevant to programs is not stored per-program. In particular vertex
     * attributes are global state. This struct is read and updated by GrGLProgram::setData to
     * allow us to avoid setting this state redundantly.
     */
    struct SharedGLState {
        GrColor fConstAttribColor;
        GrColor fConstAttribCoverage;

        SharedGLState() { this->invalidate(); }
        void invalidate() {
            fConstAttribColor = GrColor_ILLEGAL;
            fConstAttribCoverage = GrColor_ILLEGAL;
        }
    };

    /**
     * The GrDrawState's view matrix along with the aspects of the render target determine the
     * matrix sent to GL. The size of the render target affects the GL matrix because we must
     * convert from Skia device coords to GL's normalized coords. Also the origin of the render
     * target may require us to perform a mirror-flip.
     */
    struct MatrixState {
        SkMatrix        fViewMatrix;
        SkISize         fRenderTargetSize;
        GrSurfaceOrigin fRenderTargetOrigin;

        MatrixState() { this->invalidate(); }
        void invalidate() {
            fViewMatrix = SkMatrix::InvalidMatrix();
            fRenderTargetSize.fWidth = -1;
            fRenderTargetSize.fHeight = -1;
            fRenderTargetOrigin = (GrSurfaceOrigin) -1;
        }
    };

    /**
     * This function uploads uniforms and calls each GrGLEffect's setData. It is called before a
     * draw occurs using the program after the program has already been bound. It also uses the
     * GrGpuGL object to bind the textures required by the GrGLEffects.
     *
     * The color and coverage params override the GrDrawState's getColor() and getCoverage() values.
     */
    void setData(GrGpuGL*, GrColor color, GrColor coverage, SharedGLState*);

    // Parameters that affect code generation
    // This structs should be kept compact; it is input to an expensive hash key generator.
    class Desc {
    public:
        Desc() {
            // since we use this as part of a key we can't have any uninitialized
            // padding
            memset(this, 0, sizeof(Desc));
        }

        // returns this as a uint32_t array to be used as a key in the program cache
        const uint32_t* asKey() const {
            return reinterpret_cast<const uint32_t*>(this);
        }

        // For unit testing.
        void setRandom(SkMWCRandom*,
                       const GrGpuGL* gpu,
                       const GrEffectStage stages[GrDrawState::kNumStages]);

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

        friend class GrGLProgram;
    };

    // Layout information for OpenGL vertex attributes
    struct AttribLayout {
        GrGLint     fCount;
        GrGLenum    fType;
        GrGLboolean fNormalized;
    };
    static const AttribLayout kAttribLayouts[kGrVertexAttribTypeCount];

private:
    GrGLProgram(const GrGLContext& gl,
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

    // Creates a GL program ID, binds shader attributes to GL vertex attrs, and links the program
    bool bindOutputsAttribsAndLinkProgram(const GrGLShaderBuilder& builder,
                                          bool bindColorOut,
                                          bool bindDualSrcOut);

    // Sets the texture units for samplers
    void initSamplerUniforms();

    bool compileShaders(const GrGLShaderBuilder& builder);

    const char* adjustInColor(const SkString& inColor) const;

    // Helper for setData(). Makes GL calls to specify the initial color when there is not
    // per-vertex colors.
    void setColor(const GrDrawState&, GrColor color, SharedGLState*);

    // Helper for setData(). Makes GL calls to specify the initial coverage when there is not
    // per-vertex coverages.
    void setCoverage(const GrDrawState&, GrColor coverage, SharedGLState*);

    // Helper for setData() that sets the view matrix and loads the render target height uniform
    void setMatrixAndRenderTargetHeight(const GrDrawState&);

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

    // these reflect the current values of uniforms (GL uniform values travel with program)
    MatrixState                 fMatrixState;
    GrColor                     fColor;
    GrColor                     fCoverage;
    GrColor                     fColorFilterColor;

    GrGLEffect*                 fEffects[GrDrawState::kNumStages];

    Desc                        fDesc;
    const GrGLContext&          fContext;

    GrGLUniformManager          fUniformManager;
    UniformHandles              fUniformHandles;

    typedef GrRefCnt INHERITED;
};

#endif
