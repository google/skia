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
     * This function uploads uniforms, calls each GrGLProcessor's setData, and retrieves the
     * textures that need to be bound on each unit. It is the caller's responsibility to ensure
     * the program is bound before calling, and to bind the outgoing textures to their respective
     * units upon return. (Each index in the array corresponds to its matching GL texture unit.)
     */
    void setData(const GrPrimitiveProcessor&, const GrPipeline&, const GrBatchTracker&,
                 SkTArray<const GrTextureAccess*>* textureBindings);

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
                GrGLInstalledFragProcs* fragmentProcessors,
                SkTArray<UniformHandle>* passSamplerUniforms);

    // A templated helper to loop over effects, set the transforms(via subclass) and bind textures
    void setFragmentData(const GrPrimitiveProcessor&, const GrPipeline&,
                         SkTArray<const GrTextureAccess*>* textureBindings);
    virtual void setTransformData(const GrPrimitiveProcessor&,
                                  const GrPendingFragmentStage&,
                                  int index,
                                  GrGLInstalledFragProc*);

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
    int fDstTextureUnit;
    BuiltinUniformHandles fBuiltinUniformHandles;
    GrGLuint fProgramID;

    // the installed effects
    SkAutoTDelete<GrGLInstalledGeoProc> fGeometryProcessor;
    SkAutoTDelete<GrGLInstalledXferProc> fXferProcessor;
    SkAutoTUnref<GrGLInstalledFragProcs> fFragmentProcessors;

    GrProgramDesc fDesc;
    GrGLGpu* fGpu;
    GrGLProgramDataManager fProgramDataManager;
    SkTArray<UniformHandle> fSamplerUniforms;

    friend class GrGLProgramBuilder;

    typedef SkRefCnt INHERITED;
};

#endif
