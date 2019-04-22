/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLProgram_DEFINED
#define GrGLProgram_DEFINED

#include "src/gpu/gl/GrGLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

class GrGLSLFragmentProcessor;
class GrGLSLPrimitiveProcessor;
class GrGLSLXferProcessor;
class GrPipeline;
class GrPrimitiveProcessor;
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
    GrGLProgram(GrGLGpu*,
                const GrGLSLBuiltinUniformHandles&,
                GrGLuint programID,
                const UniformInfoArray& uniforms,
                const UniformInfoArray& textureSamplers,
                const VaryingInfoArray&, // used for NVPR only currently
                std::unique_ptr<GrGLSLPrimitiveProcessor> geometryProcessor,
                std::unique_ptr<GrGLSLXferProcessor> xferProcessor,
                std::unique_ptr<std::unique_ptr<GrGLSLFragmentProcessor>[]> fragmentProcessors,
                int fragmentProcessorCnt,
                std::unique_ptr<Attribute[]>,
                int vertexAttributeCnt,
                int instanceAttributeCnt,
                int vertexStride,
                int instanceStride);

    ~GrGLProgram();

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

        /**
         * Gets a float4 that adjusts the position from Skia device coords to GL's normalized device
         * coords. Assuming the transformed position, pos, is a homogeneous float3, the vec, v, is
         * applied as such:
         * pos.x = dot(v.xy, pos.xz)
         * pos.y = dot(v.zw, pos.yz)
         */
        void getRTAdjustmentVec(float* destVec) {
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
     * This function uploads uniforms, calls each GrGLSL*Processor's setData. It binds all fragment
     * processor textures. Primitive process textures can be bound using this function or by
     * calling updatePrimitiveProcessorTextureBindings.
     *
     * It is the caller's responsibility to ensure the program is bound before calling.
     */
    void updateUniformsAndTextureBindings(const GrRenderTarget*, GrSurfaceOrigin,
                                          const GrPrimitiveProcessor&, const GrPipeline&,
                                          const GrTextureProxy* const primitiveProcessorTextures[]);

    void updatePrimitiveProcessorTextureBindings(const GrPrimitiveProcessor&,
                                                 const GrTextureProxy* const[]);

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
    // A helper to loop over effects, set the transforms (via subclass) and bind textures
    void setFragmentData(const GrPipeline&, int* nextTexSamplerIdx);

    // Helper for setData() that sets the view matrix and loads the render target height uniform
    void setRenderTargetState(const GrRenderTarget*, GrSurfaceOrigin, const GrPrimitiveProcessor&);

    // these reflect the current values of uniforms (GL uniform values travel with program)
    RenderTargetState fRenderTargetState;
    GrGLSLBuiltinUniformHandles fBuiltinUniformHandles;
    GrGLuint fProgramID;

    // the installed effects
    std::unique_ptr<GrGLSLPrimitiveProcessor> fPrimitiveProcessor;
    std::unique_ptr<GrGLSLXferProcessor> fXferProcessor;
    std::unique_ptr<std::unique_ptr<GrGLSLFragmentProcessor>[]> fFragmentProcessors;
    int fFragmentProcessorCnt;

    std::unique_ptr<Attribute[]> fAttributes;
    int fVertexAttributeCnt;
    int fInstanceAttributeCnt;
    int fVertexStride;
    int fInstanceStride;

    GrGLGpu* fGpu;
    GrGLProgramDataManager fProgramDataManager;

    int fNumTextureSamplers;

    typedef SkRefCnt INHERITED;
};

#endif
