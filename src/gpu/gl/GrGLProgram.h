/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLProgram_DEFINED
#define GrGLProgram_DEFINED

#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrXferProcessor.h"
#include "src/gpu/gl/GrGLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

#include <vector>

class GrPipeline;
class GrGeometryProcessor;
class GrProgramInfo;
class GrRenderTarget;
class GrTextureProxy;

/**
 * This class manages a GPU program and records per-program information. It also records the vertex
 * and instance attribute layouts that are to be used with the program.
 */
class GrGLProgram : public SkRefCnt {
public:
    /**
     * This class has its own Attribute representation as it does not need the name and we don't
     * want to worry about copying the name string to memory with life time of GrGLProgram.
     * Additionally, these store the attribute location.
     */
    struct Attribute {
        GrVertexAttribType fCPUType;
        GrSLType fGPUType;
        size_t fOffset;
        GrGLint fLocation;
    };

    using UniformHandle = GrGLSLProgramDataManager::UniformHandle;
    using UniformInfoArray = GrGLProgramDataManager::UniformInfoArray;
    using VaryingInfoArray = GrGLProgramDataManager::VaryingInfoArray;

    /**
     * The attribute array consists of vertexAttributeCnt + instanceAttributeCnt elements with
     * the vertex attributes preceding the instance attributes.
     */
    static sk_sp<GrGLProgram> Make(
            GrGLGpu*,
            const GrGLSLBuiltinUniformHandles&,
            GrGLuint programID,
            const UniformInfoArray& uniforms,
            const UniformInfoArray& textureSamplers,
            std::unique_ptr<GrGeometryProcessor::ProgramImpl>,
            std::unique_ptr<GrXferProcessor::ProgramImpl>,
            std::vector<std::unique_ptr<GrFragmentProcessor::ProgramImpl>> fps,
            std::unique_ptr<Attribute[]>,
            int vertexAttributeCnt,
            int instanceAttributeCnt,
            int vertexStride,
            int instanceStride);

    ~GrGLProgram() override;

    /**
     * Call to abandon GL objects owned by this program.
     */
    void abandon();

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
    };

    /**
     * This function uploads uniforms and calls each GrGLSL*Processor's setData.
     *
     * It is the caller's responsibility to ensure the program is bound before calling.
     */
    void updateUniforms(const GrRenderTarget*, const GrProgramInfo&);

    /**
     * Binds all geometry processor and fragment processor textures.
     */
    void bindTextures(const GrGeometryProcessor&,
                      const GrSurfaceProxy* const geomProcTextures[],
                      const GrPipeline&);

    int vertexStride() const { return fVertexStride; }
    int instanceStride() const { return fInstanceStride; }

    int numVertexAttributes() const { return fVertexAttributeCnt; }
    const Attribute& vertexAttribute(int i) const {
        SkASSERT(i >= 0 && i < fVertexAttributeCnt);
        return fAttributes[i];
    }

    int numInstanceAttributes() const { return fInstanceAttributeCnt; }
    const Attribute& instanceAttribute(int i) const {
        SkASSERT(i >= 0 && i < fInstanceAttributeCnt);
        return fAttributes[i + fVertexAttributeCnt];
    }

private:
    GrGLProgram(GrGLGpu*,
                const GrGLSLBuiltinUniformHandles&,
                GrGLuint programID,
                const UniformInfoArray& uniforms,
                const UniformInfoArray& textureSamplers,
                std::unique_ptr<GrGeometryProcessor::ProgramImpl>,
                std::unique_ptr<GrXferProcessor::ProgramImpl>,
                std::vector<std::unique_ptr<GrFragmentProcessor::ProgramImpl>> fpImpls,
                std::unique_ptr<Attribute[]>,
                int vertexAttributeCnt,
                int instanceAttributeCnt,
                int vertexStride,
                int instanceStride);

    // Helper for setData() that sets the view matrix and loads the render target height uniform
    void setRenderTargetState(const GrRenderTarget*, GrSurfaceOrigin, const GrGeometryProcessor&);

    // these reflect the current values of uniforms (GL uniform values travel with program)
    RenderTargetState fRenderTargetState;
    GrGLSLBuiltinUniformHandles fBuiltinUniformHandles;
    GrGLuint fProgramID;

    // the installed effects
    std::unique_ptr<GrGeometryProcessor::ProgramImpl>              fGPImpl;
    std::unique_ptr<GrXferProcessor::ProgramImpl>                  fXPImpl;
    std::vector<std::unique_ptr<GrFragmentProcessor::ProgramImpl>> fFPImpls;

    std::unique_ptr<Attribute[]> fAttributes;
    int fVertexAttributeCnt;
    int fInstanceAttributeCnt;
    int fVertexStride;
    int fInstanceStride;

    GrGLGpu* fGpu;
    GrGLProgramDataManager fProgramDataManager;

    int fNumTextureSamplers;

    using INHERITED = SkRefCnt;
};

#endif
