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
#include "GrGLShaderBuilder.h"
#include "GrGLSL.h"
#include "GrGLTexture.h"
#include "GrGLUniformManager.h"

#include "SkString.h"
#include "SkXfermode.h"

class GrBinHashKeyBuilder;
class GrGLEffect;
class GrGLProgramEffects;
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
class GrGLProgram : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrGLProgram)

    static GrGLProgram* Create(GrGpuGL* gpu,
                               const GrGLProgramDesc& desc,
                               const GrEffectStage* colorStages[],
                               const GrEffectStage* coverageStages[]);

    virtual ~GrGLProgram();

    /**
     * Call to abandon GL objects owned by this program.
     */
    void abandon();

    /**
     * The shader may modify the blend coefficients. Params are in/out.
     */
    void overrideBlend(GrBlendCoeff* srcCoeff, GrBlendCoeff* dstCoeff) const;

    const GrGLProgramDesc& getDesc() { return fDesc; }

    /**
     * Gets the GL program ID for this program.
     */
    GrGLuint programID() const { return fBuilderOutput.fProgramID; }

    bool hasVertexShader() const { return fBuilderOutput.fHasVertexShader; }

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

        /**
         * Gets a matrix that goes from local coords to Skia's device coordinates.
         */
        template<int Size> void getGLMatrix(GrGLfloat* destMatrix) {
            GrGLGetMatrix<Size>(destMatrix, fViewMatrix);
        }

        /**
         * Gets a matrix that goes from local coordinates to GL normalized device coords.
         */
        template<int Size> void getRTAdjustedGLMatrix(GrGLfloat* destMatrix) {
            SkMatrix combined;
            if (kBottomLeft_GrSurfaceOrigin == fRenderTargetOrigin) {
                combined.setAll(SkIntToScalar(2) / fRenderTargetSize.fWidth, 0, -SK_Scalar1,
                                0, -SkIntToScalar(2) / fRenderTargetSize.fHeight, SK_Scalar1,
                                0, 0, 1);
            } else {
                combined.setAll(SkIntToScalar(2) / fRenderTargetSize.fWidth, 0, -SK_Scalar1,
                                0, SkIntToScalar(2) / fRenderTargetSize.fHeight, -SK_Scalar1,
                                0, 0, 1);
            }
            combined.preConcat(fViewMatrix);
            GrGLGetMatrix<Size>(destMatrix, combined);
        }

        /**
         * Gets a vec4 that adjusts the position from Skia device coords to GL's normalized device
         * coords. Assuming the transformed position, pos, is a homogeneous vec3, the vec, v, is
         * applied as such:
         * pos.x = dot(v.xy, pos.xz)
         * pos.y = dot(v.zq, pos.yz)
         */
        void getRTAdjustmentVec(GrGLfloat* destVec) {
            destVec[0] = 2.f / fRenderTargetSize.fWidth;
            destVec[1] = -1.f;
            if (kBottomLeft_GrSurfaceOrigin == fRenderTargetOrigin) {
                destVec[2] = -2.f / fRenderTargetSize.fHeight;
                destVec[3] = 1.f;
            } else {
                destVec[2] = 2.f / fRenderTargetSize.fHeight;
                destVec[3] = -1.f;
            }
        }
    };

    /**
     * This function uploads uniforms and calls each GrGLEffect's setData. It is called before a
     * draw occurs using the program after the program has already been bound. It also uses the
     * GrGpuGL object to bind the textures required by the GrGLEffects. The color and coverage
     * stages come from GrGLProgramDesc::Build().
     */
    void setData(GrDrawState::BlendOptFlags,
                 const GrEffectStage* colorStages[],
                 const GrEffectStage* coverageStages[],
                 const GrDeviceCoordTexture* dstCopy, // can be NULL
                 SharedGLState*);

private:
    typedef GrGLUniformManager::UniformHandle UniformHandle;

    GrGLProgram(GrGpuGL*,
                const GrGLProgramDesc&,
                GrGLUniformManager*,
                const GrGLShaderBuilder::GenProgramOutput&);

    // Sets the texture units for samplers.
    void initSamplerUniforms();

    // Helper for setData(). Makes GL calls to specify the initial color when there is not
    // per-vertex colors.
    void setColor(const GrDrawState&, GrColor color, SharedGLState*);

    // Helper for setData(). Makes GL calls to specify the initial coverage when there is not
    // per-vertex coverages.
    void setCoverage(const GrDrawState&, GrColor coverage, SharedGLState*);

    // Helper for setData() that sets the view matrix and loads the render target height uniform
    void setMatrixAndRenderTargetHeight(const GrDrawState&);

    // these reflect the current values of uniforms (GL uniform values travel with program)
    MatrixState                         fMatrixState;
    GrColor                             fColor;
    GrColor                             fCoverage;
    int                                 fDstCopyTexUnit;

    GrGLShaderBuilder::GenProgramOutput fBuilderOutput;

    GrGLProgramDesc                     fDesc;
    GrGpuGL*                            fGpu;

    SkAutoTUnref<GrGLUniformManager>    fUniformManager;

    typedef SkRefCnt INHERITED;
};

#endif
