/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLProgram_DEFINED
#define GrGLProgram_DEFINED

#include "builders/GrGLProgramBuilder.h"
#include "builders/GrGLNvprProgramBuilder.h"
#include "GrDrawState.h"
#include "GrGLContext.h"
#include "GrGLProgramDesc.h"
#include "GrGLSL.h"
#include "GrGLTexture.h"
#include "GrGLProgramDataManager.h"

#include "SkString.h"
#include "SkXfermode.h"

class GrGLProcessor;
class GrGLInstalledProcessors;
class GrGLProgramBuilder;

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

    typedef GrGLProgramBuilder::BuiltinUniformHandles BuiltinUniformHandles;

    virtual ~GrGLProgram();

    /**
     * Call to abandon GL objects owned by this program.
     */
    void abandon();

    const GrProgramDesc& getDesc() { return fDesc; }

    /**
     * Gets the GL program ID for this program.
     */
    GrGLuint programID() const { return fProgramID; }

    /*
     * The base class always has a vertex shader, only the NVPR variants may omit a vertex shader
     */
    virtual bool hasVertexShader() const { return true; }

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
     * This function uploads uniforms and calls each GrGLProcessor's setData. It is called before a
     * draw occurs using the program after the program has already been bound. It also uses the
     * GrGpuGL object to bind the textures required by the GrGLProcessors. The color and coverage
     * stages come from GrGLProgramDesc::Build().
     */
    void setData(const GrOptDrawState&,
                 GrGpu::DrawType,
                 const GrDeviceCoordTexture* dstCopy /* can be NULL*/);

protected:
    typedef GrGLProgramDataManager::UniformHandle UniformHandle;
    typedef GrGLProgramDataManager::UniformInfoArray UniformInfoArray;

    GrGLProgram(GrGpuGL*,
                const GrProgramDesc&,
                const BuiltinUniformHandles&,
                GrGLuint programID,
                const UniformInfoArray&,
                GrGLInstalledGeoProc* geometryProcessor,
                GrGLInstalledFragProcs* fragmentProcessors);

    // Sets the texture units for samplers.
    void initSamplerUniforms();
    void initSamplers(GrGLInstalledProc*, int* texUnitIdx);

    // Helper for setData(). Makes GL calls to specify the initial color when there is not
    // per-vertex colors.
    void setColor(const GrOptDrawState&, GrColor color);

    // Helper for setData(). Makes GL calls to specify the initial coverage when there is not
    // per-vertex coverages.
    void setCoverage(const GrOptDrawState&, uint8_t coverage);

    // A templated helper to loop over effects, set the transforms(via subclass) and bind textures
    void setFragmentData(const GrOptDrawState&);
    virtual void setTransformData(const GrFragmentStage& effectStage, GrGLInstalledFragProc* pe);
    void bindTextures(const GrGLInstalledProc*, const GrProcessor&);

    /*
     * Legacy NVPR needs a hook here to flush path tex gen settings.
     * TODO when legacy nvpr is removed, remove this call.
     */
    virtual void didSetData(GrGpu::DrawType);

    // Helper for setData() that sets the view matrix and loads the render target height uniform
    void setMatrixAndRenderTargetHeight(GrGpu::DrawType, const GrOptDrawState&);
    virtual void onSetMatrixAndRenderTargetHeight(GrGpu::DrawType, const GrOptDrawState&);

    // these reflect the current values of uniforms (GL uniform values travel with program)
    MatrixState fMatrixState;
    GrColor fColor;
    uint8_t fCoverage;
    int fDstCopyTexUnit;
    BuiltinUniformHandles fBuiltinUniformHandles;
    GrGLuint fProgramID;

    // the installed effects
    SkAutoTDelete<GrGLInstalledGeoProc> fGeometryProcessor;
    SkAutoTUnref<GrGLInstalledFragProcs> fFragmentProcessors;

    GrProgramDesc fDesc;
    GrGpuGL* fGpu;
    GrGLProgramDataManager fProgramDataManager;

    friend class GrGLProgramBuilder;

    typedef SkRefCnt INHERITED;
};

/*
 * Below are slight specializations of the program object for the different types of programs
 * The default GrGL programs consist of at the very least a vertex and fragment shader.
 * Legacy Nvpr only has a fragment shader, 1.3+ Nvpr ignores the vertex shader, but both require
 * specialized methods for setting transform data. Both types of NVPR also require setting the
 * projection matrix through a special function call
 */
class GrGLNvprProgramBase : public GrGLProgram {
protected:
    GrGLNvprProgramBase(GrGpuGL*,
                        const GrProgramDesc&,
                        const BuiltinUniformHandles&,
                        GrGLuint programID,
                        const UniformInfoArray&,
                        GrGLInstalledFragProcs* fragmentProcessors);
    virtual void onSetMatrixAndRenderTargetHeight(GrGpu::DrawType, const GrOptDrawState&);

    typedef GrGLProgram INHERITED;
};

class GrGLNvprProgram : public GrGLNvprProgramBase {
public:
      virtual bool hasVertexShader() const SK_OVERRIDE { return true; }

private:
    typedef GrGLNvprProgramBuilder::SeparableVaryingInfo SeparableVaryingInfo;
    typedef GrGLNvprProgramBuilder::SeparableVaryingInfoArray SeparableVaryingInfoArray;
    GrGLNvprProgram(GrGpuGL*,
                    const GrProgramDesc&,
                    const BuiltinUniformHandles&,
                    GrGLuint programID,
                    const UniformInfoArray&,
                    GrGLInstalledFragProcs* fragmentProcessors,
                    const SeparableVaryingInfoArray& separableVaryings);
    virtual void didSetData(GrGpu::DrawType) SK_OVERRIDE;
    virtual void setTransformData(const GrFragmentStage&, GrGLInstalledFragProc*) SK_OVERRIDE;

    struct Varying {
        GrGLint     fLocation;
        SkDEBUGCODE(
            GrSLType    fType;
        );
    };
    SkTArray<Varying, true> fVaryings;

    friend class GrGLNvprProgramBuilder;

    typedef GrGLNvprProgramBase INHERITED;
};

class GrGLLegacyNvprProgram : public GrGLNvprProgramBase {
public:
    virtual bool hasVertexShader() const SK_OVERRIDE { return false; }

private:
    GrGLLegacyNvprProgram(GrGpuGL* gpu,
                          const GrProgramDesc& desc,
                          const BuiltinUniformHandles&,
                          GrGLuint programID,
                          const UniformInfoArray&,
                          GrGLInstalledFragProcs* fragmentProcessors,
                          int texCoordSetCnt);
    virtual void didSetData(GrGpu::DrawType) SK_OVERRIDE;
    virtual void setTransformData(const GrFragmentStage&, GrGLInstalledFragProc*) SK_OVERRIDE;

    int fTexCoordSetCnt;

    friend class GrGLLegacyNvprProgramBuilder;

    typedef GrGLNvprProgramBase INHERITED;
};

#endif
