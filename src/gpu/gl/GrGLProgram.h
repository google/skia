/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLProgram_DEFINED
#define GrGLProgram_DEFINED

#include "GrDrawState.h"
#include "GrGLContext.h"
#include "GrGLProgramDesc.h"
#include "GrGLSL.h"
#include "GrGLTexture.h"
#include "GrGLUniformManager.h"

#include "SkString.h"
#include "SkXfermode.h"

class GrBinHashKeyBuilder;
class GrGLEffect;
class GrGLShaderBuilder;

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

    static GrGLProgram* Create(const GrGLContext& gl,
                               const GrGLProgramDesc& desc,
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

    const GrGLProgramDesc& getDesc() { return fDesc; }

    /**
     * Gets the GL program ID for this program.
     */
    GrGLuint programID() const { return fProgramID; }

    /**
     * Some GL state that is relevant to programs is not stored per-program. In particular color
     * and coverage attributes can be global state. This struct is read and updated by
     * GrGLProgram::setColor and GrGLProgram::setCoverage to allow us to avoid setting this state
     * redundantly.
     */
    struct SharedGLState {
        GrColor fConstAttribColor;
        int     fConstAttribColorIndex;
        GrColor fConstAttribCoverage;
        int     fConstAttribCoverageIndex;

        SharedGLState() { this->invalidate(); }
        void invalidate() {
            fConstAttribColor = GrColor_ILLEGAL;
            fConstAttribColorIndex = -1;
            fConstAttribCoverage = GrColor_ILLEGAL;
            fConstAttribCoverageIndex = -1;
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
    void setData(GrGpuGL*,
                 GrColor color,
                 GrColor coverage,
                 const GrDeviceCoordTexture* dstCopy, // can be NULL
                 SharedGLState*);

private:
    GrGLProgram(const GrGLContext& gl,
                const GrGLProgramDesc& desc,
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

        // Uniforms for computing texture coords to do the dst-copy lookup
        UniformHandle       fDstCopyTopLeftUni;
        UniformHandle       fDstCopyScaleUni;
        UniformHandle       fDstCopySamplerUni;

        // An array of sampler uniform handles for each effect.
        SamplerUniSArray    fEffectSamplerUnis[GrDrawState::kNumStages];

        UniformHandles() {
            fViewMatrixUni = GrGLUniformManager::kInvalidUniformHandle;
            fColorUni = GrGLUniformManager::kInvalidUniformHandle;
            fCoverageUni = GrGLUniformManager::kInvalidUniformHandle;
            fColorFilterUni = GrGLUniformManager::kInvalidUniformHandle;
            fRTHeightUni = GrGLUniformManager::kInvalidUniformHandle;
            fDstCopyTopLeftUni = GrGLUniformManager::kInvalidUniformHandle;
            fDstCopyScaleUni = GrGLUniformManager::kInvalidUniformHandle;
            fDstCopySamplerUni = GrGLUniformManager::kInvalidUniformHandle;
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

    GrGLProgramDesc             fDesc;
    const GrGLContext&          fContext;

    GrGLUniformManager          fUniformManager;
    UniformHandles              fUniformHandles;

    typedef GrRefCnt INHERITED;
};

#endif
