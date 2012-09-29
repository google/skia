/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLProgram_DEFINED
#define GrGLProgram_DEFINED

#include "GrDrawState.h"
#include "GrGLContextInfo.h"
#include "GrGLSL.h"
#include "GrGLTexture.h"
#include "GrGLUniformManager.h"

#include "SkString.h"
#include "SkXfermode.h"

class GrBinHashKeyBuilder;
class GrGLProgramStage;
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
                               const GrCustomStage** customStages);

    virtual ~GrGLProgram();

    /** Call to abandon GL objects owned by this program */
    void abandon();

    /**
     * The shader may modify the blend coeffecients. Params are in/out
     */
    void overrideBlend(GrBlendCoeff* srcCoeff, GrBlendCoeff* dstCoeff) const;

    const Desc& getDesc() { return fDesc; }

    /**
     * Attribute indices. These should not overlap. Matrices consume 3 slots.
     */
    static int PositionAttributeIdx() { return 0; }
    static int TexCoordAttributeIdx(int tcIdx) { return 1 + tcIdx; }
    static int ColorAttributeIdx() { return 1 + GrDrawState::kMaxTexCoords; }
    static int CoverageAttributeIdx() {
        return 2 + GrDrawState::kMaxTexCoords;
    }
    static int EdgeAttributeIdx() { return 3 + GrDrawState::kMaxTexCoords; }

    static int ViewMatrixAttributeIdx() {
        return 4 + GrDrawState::kMaxTexCoords;
    }
    static int TextureMatrixAttributeIdx(int stage) {
        return 7 + GrDrawState::kMaxTexCoords + 3 * stage;
    }

    // Parameters that affect code generation
    // These structs should be kept compact; they are the input to an
    // expensive hash key generator.
    struct Desc {
        Desc() {
            // since we use this as part of a key we can't have any unitialized
            // padding
            memset(this, 0, sizeof(Desc));
        }

        // returns this as a uint32_t array to be used as a key in the program cache
        const uint32_t* asKey() const {
            return reinterpret_cast<const uint32_t*>(this);
        }

        struct StageDesc {
            enum OptFlagBits {
                kNoPerspective_OptFlagBit       = 1 << 0,
                kIdentityMatrix_OptFlagBit      = 1 << 1,
                kIsEnabled_OptFlagBit           = 1 << 7
            };

            uint8_t fOptFlags;

            /** Non-zero if user-supplied code will write the stage's
                contribution to the fragment shader. */
            GrProgramStageFactory::StageKey fCustomStageKey;

            inline bool isEnabled() const {
                return SkToBool(fOptFlags & kIsEnabled_OptFlagBit);
            }
            inline void setEnabled(bool newValue) {
                if (newValue) {
                    fOptFlags |= kIsEnabled_OptFlagBit;
                } else {
                    fOptFlags &= ~kIsEnabled_OptFlagBit;
                }
            }
        };

        // Specifies where the intitial color comes from before the stages are
        // applied.
        enum ColorInput {
            kSolidWhite_ColorInput,
            kTransBlack_ColorInput,
            kAttribute_ColorInput,
            kUniform_ColorInput,

            kColorInputCnt
        };
        // Dual-src blending makes use of a secondary output color that can be
        // used as a per-pixel blend coeffecient. This controls whether a
        // secondary source is output and what value it holds.
        enum DualSrcOutput {
            kNone_DualSrcOutput,
            kCoverage_DualSrcOutput,
            kCoverageISA_DualSrcOutput,
            kCoverageISC_DualSrcOutput,

            kDualSrcOutputCnt
        };

        GrDrawState::VertexEdgeType fVertexEdgeType;

        // stripped of bits that don't affect prog generation
        GrVertexLayout fVertexLayout;

        StageDesc fStages[GrDrawState::kNumStages];

        // To enable experimental geometry shader code (not for use in
        // production)
#if GR_GL_EXPERIMENTAL_GS
        bool fExperimentalGS;
#endif

        uint8_t fColorInput;        // casts to enum ColorInput
        uint8_t fCoverageInput;     // casts to enum CoverageInput
        uint8_t fDualSrcOutput;     // casts to enum DualSrcOutput
        int8_t fFirstCoverageStage;
        SkBool8 fEmitsPointSize;
        SkBool8 fColorMatrixEnabled;

        uint8_t fColorFilterXfermode;  // casts to enum SkXfermode::Mode
    };
    GR_STATIC_ASSERT(!(sizeof(Desc) % 4));

    // for code readability
    typedef Desc::StageDesc StageDesc;

private:
    struct StageUniforms;

    GrGLProgram(const GrGLContextInfo& gl,
                const Desc& desc,
                const GrCustomStage** customStages);

    bool succeeded() const { return 0 != fProgramID; }

    /**
     *  This is the heavy initilization routine for building a GLProgram.
     */
    bool genProgram(const GrCustomStage** customStages);

    void genInputColor(GrGLShaderBuilder* builder, SkString* inColor);

    static GrGLProgramStage* GenStageCode(const GrCustomStage* stage,
                                          const StageDesc& desc, // TODO: Eliminate this
                                          StageUniforms* stageUniforms, // TODO: Eliminate this
                                          const char* fsInColor, // NULL means no incoming color
                                          const char* fsOutColor,
                                          const char* vsInCoord,
                                          GrGLShaderBuilder* builder);

    void genGeometryShader(GrGLShaderBuilder* segments) const;

    typedef GrGLUniformManager::UniformHandle UniformHandle;

    void genUniformCoverage(GrGLShaderBuilder* segments, SkString* inOutCoverage);

    // generates code to compute coverage based on edge AA. Returns true if edge coverage was
    // inserted in which case coverageVar will be updated to refer to a scalar. Otherwise,
    // coverageVar is set to an empty string.
    bool genEdgeCoverage(SkString* coverageVar, GrGLShaderBuilder* builder) const;

    // Creates a GL program ID, binds shader attributes to GL vertex attrs, and links the program
    bool bindOutputsAttribsAndLinkProgram(SkString texCoordAttrNames[GrDrawState::kMaxTexCoords],
                                          bool bindColorOut,
                                          bool bindDualSrcOut);

    // Sets the texture units for samplers
    void initSamplerUniforms();

    bool compileShaders(const GrGLShaderBuilder& builder);

    const char* adjustInColor(const SkString& inColor) const;

    struct StageUniforms {
        UniformHandle fTextureMatrixUni;
        SkTArray<UniformHandle, true> fSamplerUniforms;
        StageUniforms() {
            fTextureMatrixUni = GrGLUniformManager::kInvalidUniformHandle;
        }
    };

    struct Uniforms {
        UniformHandle fViewMatrixUni;
        UniformHandle fColorUni;
        UniformHandle fCoverageUni;
        UniformHandle fColorFilterUni;
        UniformHandle fColorMatrixUni;
        UniformHandle fColorMatrixVecUni;
        StageUniforms fStages[GrDrawState::kNumStages];
        Uniforms() {
            fViewMatrixUni = GrGLUniformManager::kInvalidUniformHandle;
            fColorUni = GrGLUniformManager::kInvalidUniformHandle;
            fCoverageUni = GrGLUniformManager::kInvalidUniformHandle;
            fColorFilterUni = GrGLUniformManager::kInvalidUniformHandle;
            fColorMatrixUni = GrGLUniformManager::kInvalidUniformHandle;
            fColorMatrixVecUni = GrGLUniformManager::kInvalidUniformHandle;
        }
    };

    // IDs
    GrGLuint    fVShaderID;
    GrGLuint    fGShaderID;
    GrGLuint    fFShaderID;
    GrGLuint    fProgramID;

    // The matrix sent to GL is determined by both the client's matrix and
    // the size of the viewport.
    GrMatrix  fViewMatrix;
    SkISize   fViewportSize;

    // these reflect the current values of uniforms
    // (GL uniform values travel with program)
    GrColor                     fColor;
    GrColor                     fCoverage;
    GrColor                     fColorFilterColor;
    /// When it is sent to GL, the texture matrix will be flipped if the texture orientation
    /// (below) requires.
    GrMatrix                    fTextureMatrices[GrDrawState::kNumStages];
    GrGLTexture::Orientation    fTextureOrientation[GrDrawState::kNumStages];

    GrGLProgramStage*           fProgramStage[GrDrawState::kNumStages];

    Desc fDesc;
    const GrGLContextInfo&      fContextInfo;

    GrGLUniformManager          fUniformManager;
    Uniforms                    fUniforms;

    friend class GrGpuGL; // TODO: remove this by adding getters and moving functionality.

    typedef GrRefCnt INHERITED;
};

#endif
