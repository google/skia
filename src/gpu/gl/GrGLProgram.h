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
//#include "GrGpu.h"

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
                               GrCustomStage** customStages);

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

    enum {
        kUnusedUniform = -1,
    };

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

        enum OutputConfig {
            // PM-color OR color with no alpha channel
            kPremultiplied_OutputConfig,
            // nonPM-color with alpha channel. Round components up after
            // dividing by alpha. Assumes output is 8 bits for r, g, and b
            kUnpremultiplied_RoundUp_OutputConfig,
            // nonPM-color with alpha channel. Round components down after
            // dividing by alpha. Assumes output is 8 bits for r, g, and b
            kUnpremultiplied_RoundDown_OutputConfig,

            kOutputConfigCnt
        };

        struct StageDesc {
            enum OptFlagBits {
                kNoPerspective_OptFlagBit       = 1 << 0,
                kIdentityMatrix_OptFlagBit      = 1 << 1,
                kIsEnabled_OptFlagBit           = 1 << 7
            };

            /**
              Flags set based on a src texture's pixel config. The operations
              described are performed after reading a texel.
             */
            enum InConfigFlags {
                kNone_InConfigFlag                      = 0x00,

                /**
                  Swap the R and B channels. This is incompatible with
                  kSmearAlpha. It is prefereable to perform the swizzle outside
                  the shader using GL_ARB_texture_swizzle if possible rather
                  than setting this flag.
                 */
                kSwapRAndB_InConfigFlag                 = 0x01,

                /**
                 Smear alpha across all four channels. This is incompatible with
                 kSwapRAndB, kMulRGBByAlpha* and kSmearRed. It is prefereable 
                 to perform the smear outside the shader using 
                 GL_ARB_texture_swizzle if possible rather than setting this 
                 flag.
                */
                kSmearAlpha_InConfigFlag                = 0x02,

                /**
                 Smear the red channel across all four channels. This flag is 
                 incompatible with kSwapRAndB, kMulRGBByAlpha*and kSmearAlpha. 
                 It is preferable to use GL_ARB_texture_swizzle instead of this 
                 flag.
                */
                kSmearRed_InConfigFlag                  = 0x04,

                /**
                 Multiply r,g,b by a after texture reads. This flag incompatible
                 with kSmearAlpha.

                 It is assumed the src texture has 8bit color components. After
                 reading the texture one version rounds up to the next multiple
                 of 1/255.0 and the other rounds down. At most one of these
                 flags may be set.
                 */
                kMulRGBByAlpha_RoundUp_InConfigFlag     =  0x08,
                kMulRGBByAlpha_RoundDown_InConfigFlag   =  0x10,

                kDummyInConfigFlag,
                kInConfigBitMask = (kDummyInConfigFlag-1) |
                                   (kDummyInConfigFlag-2)
            };

            uint8_t fOptFlags;
            uint8_t fInConfigFlags; // bitfield of InConfigFlags values

            /** Non-zero if user-supplied code will write the stage's
                contribution to the fragment shader. */
            uint16_t fCustomStageKey;

            GR_STATIC_ASSERT((InConfigFlags)(uint8_t)kInConfigBitMask ==
                             kInConfigBitMask);

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
        uint8_t fOutputConfig;      // casts to enum OutputConfig
        uint8_t fDualSrcOutput;     // casts to enum DualSrcOutput
        int8_t fFirstCoverageStage;
        SkBool8 fEmitsPointSize;
        SkBool8 fColorMatrixEnabled;

        uint8_t fColorFilterXfermode;  // casts to enum SkXfermode::Mode
        int8_t fPadding[1];
    };
    GR_STATIC_ASSERT(!(sizeof(Desc) % 4));

    // for code readability
    typedef Desc::StageDesc StageDesc;

private:
    GrGLProgram(const GrGLContextInfo& gl,
                const Desc& desc,
                GrCustomStage** customStages);

    bool succeeded() const { return 0 != fProgramID; }

    /**
     *  This is the heavy initilization routine for building a GLProgram.
     */
    bool genProgram(GrCustomStage** customStages);

    void genInputColor(GrGLShaderBuilder* builder, SkString* inColor);

    // Determines which uniforms will need to be bound.
    void genStageCode(int stageNum,
                      const char* fsInColor, // NULL means no incoming color
                      const char* fsOutColor,
                      const char* vsInCoord,
                      GrGLShaderBuilder* builder);

    void genGeometryShader(GrGLShaderBuilder* segments) const;

    void genUniformCoverage(GrGLShaderBuilder* segments, SkString* inOutCoverage);

    // generates code to compute coverage based on edge AA.
    void genEdgeCoverage(SkString* coverageVar, GrGLShaderBuilder* builder) const;

    // Creates a GL program ID, binds shader attributes to GL vertex attrs, and links the program
    bool bindOutputsAttribsAndLinkProgram(SkString texCoordAttrNames[GrDrawState::kMaxTexCoords],
                                          bool bindColorOut,
                                          bool bindDualSrcOut);

    // Binds uniforms; initializes cache to invalid values.
    void getUniformLocationsAndInitCache(const GrGLShaderBuilder& builder);

    bool compileShaders(const GrGLShaderBuilder& builder);

    const char* adjustInColor(const SkString& inColor) const;

    struct StageUniLocations {
        GrGLint fTextureMatrixUni;
        GrGLint fSamplerUni;
        GrGLint fTexDomUni;
        void reset() {
            fTextureMatrixUni = kUnusedUniform;
            fSamplerUni = kUnusedUniform;
            fTexDomUni = kUnusedUniform;
        }
    };

    struct UniLocations {
        GrGLint fViewMatrixUni;
        GrGLint fColorUni;
        GrGLint fCoverageUni;
        GrGLint fColorFilterUni;
        GrGLint fColorMatrixUni;
        GrGLint fColorMatrixVecUni;
        StageUniLocations fStages[GrDrawState::kNumStages];
        void reset() {
            fViewMatrixUni = kUnusedUniform;
            fColorUni = kUnusedUniform;
            fCoverageUni = kUnusedUniform;
            fColorFilterUni = kUnusedUniform;
            fColorMatrixUni = kUnusedUniform;
            fColorMatrixVecUni = kUnusedUniform;
            for (int s = 0; s < GrDrawState::kNumStages; ++s) {
                fStages[s].reset();
            }
        }
    };

    // IDs
    GrGLuint    fVShaderID;
    GrGLuint    fGShaderID;
    GrGLuint    fFShaderID;
    GrGLuint    fProgramID;
    // shader uniform locations (-1 if shader doesn't use them)
    UniLocations fUniLocations;

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

    friend class GrGpuGL; // TODO: remove this by adding getters and moving functionality.

    typedef GrRefCnt INHERITED;
};

#endif
