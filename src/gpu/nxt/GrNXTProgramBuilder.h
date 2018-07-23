/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNXTProgramBuilder_DEFINED
#define GrNXTProgramBuilder_DEFINED

#include <vector>

#include "SkLRUCache.h"
#include "SkSLCompiler.h"
#include "nxt/GrNXTProgramDataManager.h"
#include "nxt/GrNXTUniformHandler.h"
#include "nxt/GrNXTVaryingHandler.h"
#include "nxt/nxtcpp.h"
#include "glsl/GrGLSLProgramBuilder.h"

class GrPipeline;

struct GrRenderTargetState {
    SkISize         fRenderTargetSize;
    GrSurfaceOrigin fRenderTargetOrigin;

    GrRenderTargetState() { this->invalidate(); }
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

struct GrNXTProgram : public SkRefCnt {
    typedef GrGLSLBuiltinUniformHandles BuiltinUniformHandles;
    GrNXTProgram(const GrNXTUniformHandler::UniformInfoArray& uniforms,
                 uint32_t geometryUniformSize,
                 uint32_t fragmentUniformSize);
    std::unique_ptr<GrGLSLPrimitiveProcessor> fGeometryProcessor;
    std::unique_ptr<GrGLSLXferProcessor> fXferProcessor;
    std::vector<nxt::Sampler> fSamplers;
    std::unique_ptr<std::unique_ptr<GrGLSLFragmentProcessor>[]> fFragmentProcessors;
    int fFragmentProcessorCnt;
    nxt::BindGroupLayout fBindGroupLayout;
    nxt::RenderPipeline fRenderPipeline;
    GrNXTProgramDataManager fDataManager;
    GrRenderTargetState fRenderTargetState;
    BuiltinUniformHandles fBuiltinUniformHandles;
    size_t fVertexStride;
    struct BindGroupKey {
        static const int kPreAllocSize = 128;
        BindGroupKey() { }
        bool operator==(const BindGroupKey& other) const {
            SkASSERT(other.fData.count() == fData.count());
            return !memcmp(other.fData.begin(), fData.begin(), fData.count());
        }
        struct Hash {
            uint32_t operator()(const BindGroupKey& key) {
                return SkOpts::hash_fn(key.fData.begin(), key.fData.count(), 0);
            }
        };
        void append(size_t size, void* data) {
            fData.push_back_n(size, static_cast<uint8_t*>(data));
        }
        SkSTArray<kPreAllocSize, uint8_t, true> fData;
    };
    struct BindGroupValue {
        BindGroupValue() {}
        BindGroupValue(const BindGroupValue& other)
            : fBindGroup(other.fBindGroup.Clone())
            , fGeometryBuffer(other.fGeometryBuffer.Clone())
            , fFragmentBuffer(other.fFragmentBuffer.Clone()) {
        }
        nxt::BindGroup fBindGroup;
        nxt::Buffer    fGeometryBuffer;
        nxt::Buffer    fFragmentBuffer;
    };
    SkLRUCache<BindGroupKey, BindGroupValue, BindGroupKey::Hash> fBindGroupCache;

    void setRenderTargetState(const GrRenderTargetProxy*);
    void buildKey(BindGroupKey* key, const GrPrimitiveProcessor&, const GrPipeline&);
    nxt::BindGroup setData(GrNXTGpu* gpu, const GrPrimitiveProcessor&, const GrPipeline&);
};

class GrNXTProgramBuilder : public GrGLSLProgramBuilder {
public:
    static sk_sp<GrNXTProgram> Build(GrNXTGpu*,
                                     const GrPipeline&,
                                     const GrPrimitiveProcessor&,
                                     GrPrimitiveType primitiveType,
                                     nxt::TextureFormat colorFormat,
                                     bool hasDepthStencil,
                                     nxt::TextureFormat depthStencilFormat,
                                     GrProgramDesc* desc);
    const GrCaps* caps() const override;
    GrGLSLUniformHandler* uniformHandler() override { return &fUniformHandler; }
    const GrGLSLUniformHandler* uniformHandler() const override { return &fUniformHandler; }
    GrGLSLVaryingHandler* varyingHandler() override { return &fVaryingHandler; }

    GrNXTGpu* gpu() const { return fGpu; }

private:
    GrNXTProgramBuilder(GrNXTGpu*, const GrPrimitiveProcessor&, const GrPipeline&,
                        GrProgramDesc*);
    nxt::ShaderModule CreateShaderModule(nxt::Device, const GrGLSLShaderBuilder&, SkSL::Program::Kind, SkSL::Program::Inputs* inputs);
    GrNXTGpu*             fGpu;
    GrNXTVaryingHandler   fVaryingHandler;
    GrNXTUniformHandler   fUniformHandler;

    typedef GrGLSLProgramBuilder INHERITED;
};
#endif
