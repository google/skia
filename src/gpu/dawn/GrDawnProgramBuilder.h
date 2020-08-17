/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnProgramBuilder_DEFINED
#define GrDawnProgramBuilder_DEFINED

#include "src/core/SkLRUCache.h"
#include "src/gpu/GrSPIRVUniformHandler.h"
#include "src/gpu/GrSPIRVVaryingHandler.h"
#include "src/gpu/dawn/GrDawnProgramDataManager.h"
#include "src/sksl/SkSLCompiler.h"
#include "dawn/webgpu_cpp.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"

class GrPipeline;

struct GrDawnProgram : public SkRefCnt {
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
            if (kTopLeft_GrSurfaceOrigin == fRenderTargetOrigin) {
                destVec[2] = -2.f / fRenderTargetSize.fHeight;
                destVec[3] = 1.f;
            } else {
                destVec[2] = 2.f / fRenderTargetSize.fHeight;
                destVec[3] = -1.f;
            }
        }
    };
    static const int kMaxBindGroupCacheEntries = 4096;
    typedef GrGLSLBuiltinUniformHandles BuiltinUniformHandles;
    GrDawnProgram(const GrSPIRVUniformHandler::UniformInfoArray& uniforms,
                  uint32_t uniformBufferSize)
      : fDataManager(uniforms, uniformBufferSize)
      , fUniformBindGroupCache(kMaxBindGroupCacheEntries) {
    }
    std::unique_ptr<GrGLSLPrimitiveProcessor> fGeometryProcessor;
    std::unique_ptr<GrGLSLXferProcessor> fXferProcessor;
    std::unique_ptr<std::unique_ptr<GrGLSLFragmentProcessor>[]> fFragmentProcessors;
    std::vector<wgpu::BindGroupLayout> fBindGroupLayouts;
    wgpu::RenderPipeline fRenderPipeline;
    GrDawnProgramDataManager fDataManager;
    RenderTargetState fRenderTargetState;
    BuiltinUniformHandles fBuiltinUniformHandles;
    struct UniformBindGroupKey {
        static const int kPreAllocSize = 128;
        UniformBindGroupKey() {}
        bool operator==(const UniformBindGroupKey& other) const {
            SkASSERT(other.fData.count() == fData.count());
            return !memcmp(other.fData.begin(), fData.begin(), fData.count());
        }
        struct Hash {
            uint32_t operator()(const UniformBindGroupKey& key) {
                return SkOpts::hash_fn(key.fData.begin(), key.fData.count(), 0);
            }
          };
        void append(size_t size, const void* data) {
            fData.push_back_n(size, static_cast<const uint8_t*>(data));
        }
        SkSTArray<kPreAllocSize, uint8_t, true> fData;
    };
    struct UniformBindGroupValue {
        UniformBindGroupValue() {}
        UniformBindGroupValue(const UniformBindGroupValue& other)
            : fBindGroup(other.fBindGroup) {
        }
        wgpu::BindGroup  fBindGroup;
        int              fOffset;
    };
    SkLRUCache<UniformBindGroupKey, UniformBindGroupValue, UniformBindGroupKey::Hash> fUniformBindGroupCache;
    void setRenderTargetState(const GrRenderTarget*, GrSurfaceOrigin);
    wgpu::BindGroup setUniformData(GrDawnGpu*, const GrRenderTarget*, const GrProgramInfo&);
    wgpu::BindGroup setTextures(GrDawnGpu* gpu,
                                const GrPrimitiveProcessor& primProc,
                                const GrPipeline& pipeline,
                                const GrSurfaceProxy* const primProcTextures[]);

    void buildUniformKey(UniformBindGroupKey* key, const GrPrimitiveProcessor&, const GrPipeline&,
                         const GrTextureProxy* const primProcTextures[]);

};

class GrDawnProgramBuilder : public GrGLSLProgramBuilder {
public:
    static sk_sp<GrDawnProgram> Build(GrDawnGpu*,
                                      GrRenderTarget*,
                                      const GrProgramInfo&,
                                      wgpu::TextureFormat colorFormat,
                                      bool hasDepthStencil,
                                      wgpu::TextureFormat depthStencilFormat,
                                      GrProgramDesc*);
    const GrCaps* caps() const override;
    GrGLSLUniformHandler* uniformHandler() override { return &fUniformHandler; }
    const GrGLSLUniformHandler* uniformHandler() const override { return &fUniformHandler; }
    GrGLSLVaryingHandler* varyingHandler() override { return &fVaryingHandler; }

    GrDawnGpu* gpu() const { return fGpu; }

private:
    GrDawnProgramBuilder(GrDawnGpu*,
                         GrRenderTarget*,
                         const GrProgramInfo&,
                         GrProgramDesc*);
    wgpu::ShaderModule createShaderModule(const GrGLSLShaderBuilder&, SkSL::Program::Kind,
                                          bool flipY, SkSL::Program::Inputs* inputs);
    GrDawnGpu*             fGpu;
    GrSPIRVVaryingHandler   fVaryingHandler;
    GrSPIRVUniformHandler   fUniformHandler;

    typedef GrGLSLProgramBuilder INHERITED;
};
#endif
