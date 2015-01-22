/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLProgram_DEFINED
#define GrGLProgram_DEFINED

#include "builders/GrGLProgramBuilder.h"
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
class GrPipeline;

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

    /**
     * We use the RT's size and origin to adjust from Skia device space to OpenGL normalized device
     * space and to make device space positions have the correct origin for processors that require
     * them.
     */
    struct RenderTargetState {
        SkISize         fRenderTargetSize;
        GrSurfaceOrigin fRenderTargetOrigin;

        RenderTargetState() { this->invalidate(); }
        void invalidate() {
            fRenderTargetSize.fWidth = -1;
            fRenderTargetSize.fHeight = -1;
            fRenderTargetOrigin = (GrSurfaceOrigin) -1;
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
     * GrGLGpu object to bind the textures required by the GrGLProcessors. The color and coverage
     * stages come from GrGLProgramDesc::Build().
     */
    void setData(const GrPrimitiveProcessor&, const GrPipeline&, const GrBatchTracker&);

protected:
    typedef GrGLProgramDataManager::UniformHandle UniformHandle;
    typedef GrGLProgramDataManager::UniformInfoArray UniformInfoArray;

    GrGLProgram(GrGLGpu*,
                const GrProgramDesc&,
                const BuiltinUniformHandles&,
                GrGLuint programID,
                const UniformInfoArray&,
                GrGLInstalledGeoProc* geometryProcessor,
                GrGLInstalledXferProc* xferProcessor,
                GrGLInstalledFragProcs* fragmentProcessors);

    // Sets the texture units for samplers.
    void initSamplerUniforms();
    template <class Proc>
    void initSamplers(Proc*, int* texUnitIdx);

    // A templated helper to loop over effects, set the transforms(via subclass) and bind textures
    void setFragmentData(const GrPrimitiveProcessor&, const GrPipeline&);
    virtual void setTransformData(const GrPrimitiveProcessor&,
                                  const GrPendingFragmentStage&,
                                  int index,
                                  GrGLInstalledFragProc*);
    template <class Proc>
    void bindTextures(const Proc*, const GrProcessor&);

    /*
     * Legacy NVPR needs a hook here to flush path tex gen settings.
     * TODO when legacy nvpr is removed, remove this call.
     */
    virtual void didSetData() {}

    // Helper for setData() that sets the view matrix and loads the render target height uniform
    void setRenderTargetState(const GrPrimitiveProcessor&, const GrPipeline&);
    virtual void onSetRenderTargetState(const GrPrimitiveProcessor&, const GrPipeline&);

    // these reflect the current values of uniforms (GL uniform values travel with program)
    RenderTargetState fRenderTargetState;
    GrColor fColor;
    uint8_t fCoverage;
    int fDstCopyTexUnit;
    BuiltinUniformHandles fBuiltinUniformHandles;
    GrGLuint fProgramID;

    // the installed effects
    SkAutoTDelete<GrGLInstalledGeoProc> fGeometryProcessor;
    SkAutoTDelete<GrGLInstalledXferProc> fXferProcessor;
    SkAutoTUnref<GrGLInstalledFragProcs> fFragmentProcessors;

    GrProgramDesc fDesc;
    GrGLGpu* fGpu;
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
class GrGLNvprProgram : public GrGLProgram {
protected:
    GrGLNvprProgram(GrGLGpu*,
                    const GrProgramDesc&,
                    const BuiltinUniformHandles&,
                    GrGLuint programID,
                    const UniformInfoArray&,
                    GrGLInstalledGeoProc*,
                    GrGLInstalledXferProc* xferProcessor,
                    GrGLInstalledFragProcs* fragmentProcessors);

private:
    void didSetData() SK_OVERRIDE;
    virtual void setTransformData(const GrPrimitiveProcessor&,
                                  const GrPendingFragmentStage&,
                                  int index,
                                  GrGLInstalledFragProc*) SK_OVERRIDE;
    virtual void onSetRenderTargetState(const GrPrimitiveProcessor&, const GrPipeline&);

    friend class GrGLNvprProgramBuilder;

    typedef GrGLProgram INHERITED;
};

#endif
