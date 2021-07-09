/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DCommandList_DEFINED
#define GrD3DCommandList_DEFINED

#include "include/gpu/GrTypes.h"
#include "include/gpu/d3d/GrD3DTypes.h"
#include "src/gpu/GrManagedResource.h"
#include "src/gpu/GrRingBuffer.h"
#include "src/gpu/d3d/GrD3DRootSignature.h"

#include <memory>

class GrD3DGpu;
class GrD3DBuffer;
class GrD3DCommandSignature;
class GrD3DConstantRingBuffer;
class GrD3DPipeline;
class GrD3DRenderTarget;
class GrD3DRootSignature;
class GrD3DAttachment;
class GrD3DTexture;
class GrD3DTextureResource;

class GrScissorState;

class GrD3DCommandList {
public:
    virtual ~GrD3DCommandList() {
        this->releaseResources();
    }

    enum class SubmitResult {
        kNoWork,
        kSuccess,
        kFailure,
    };
    SubmitResult submit(ID3D12CommandQueue* queue);

    bool close();
    void reset();

    ////////////////////////////////////////////////////////////////////////////
    // GraphicsCommandList commands
    ////////////////////////////////////////////////////////////////////////////

    // All barriers should reference subresources of managedResource
    void resourceBarrier(sk_sp<GrManagedResource> managedResource,
                         int numBarriers,
                         const D3D12_RESOURCE_TRANSITION_BARRIER* barriers);

    void uavBarrier(sk_sp<GrManagedResource> managedResource,
                    ID3D12Resource* uavResource);

    void aliasingBarrier(sk_sp<GrManagedResource> beforeManagedResource,
                         ID3D12Resource* beforeResource,
                         sk_sp<GrManagedResource> afterManagedResource,
                         ID3D12Resource* afterResource);

    // Helper method that calls copyTextureRegion multiple times, once for each subresource
    // The srcBuffer comes from a staging buffer so we don't need to take any refs to it. Instead,
    // we ref the whole buffer during sumbit.
    void copyBufferToTexture(ID3D12Resource* srcBuffer,
                             const GrD3DTextureResource* dstTexture,
                             uint32_t subresourceCount,
                             D3D12_PLACED_SUBRESOURCE_FOOTPRINT* bufferFootprints,
                             int left, int top);

    void copyTextureRegionToTexture(sk_sp<GrManagedResource> dst,
                                    const D3D12_TEXTURE_COPY_LOCATION* dstLocation,
                                    UINT dstX, UINT dstY,
                                    sk_sp<GrManagedResource> src,
                                    const D3D12_TEXTURE_COPY_LOCATION* srcLocation,
                                    const D3D12_BOX* srcBox);

     void copyTextureRegionToBuffer(sk_sp<const GrBuffer> dst,
                                    const D3D12_TEXTURE_COPY_LOCATION* dstLocation,
                                    UINT dstX,
                                    UINT dstY,
                                    sk_sp<GrManagedResource> src,
                                    const D3D12_TEXTURE_COPY_LOCATION* srcLocation,
                                    const D3D12_BOX* srcBox);

     void copyTextureToTexture(const GrD3DTexture* dst,
                               const GrD3DTexture* src,
                               UINT subresourceIndex = -1);

    // We don't take a ref to the src buffer because we assume the src buffer is coming from a
    // staging buffer which will get ref'd during submit.
    void copyBufferToBuffer(sk_sp<GrD3DBuffer> dstBuffer, uint64_t dstOffset,
                            ID3D12Resource* srcBuffer, uint64_t srcOffset,
                            uint64_t numBytes);

    void addGrBuffer(sk_sp<const GrBuffer> buffer) {
        fTrackedGpuBuffers.push_back(std::move(buffer));
    }

    void releaseResources();

    bool hasWork() const { return fHasWork; }

    void addFinishedCallback(sk_sp<GrRefCntedCallback> callback);

private:
    static const int kInitialTrackedResourcesCount = 32;

protected:
    GrD3DCommandList(gr_cp<ID3D12CommandAllocator> allocator,
                     gr_cp<ID3D12GraphicsCommandList> commandList);

    // Add ref-counted resource that will be tracked and released when this command buffer finishes
    // execution
    void addResource(sk_sp<GrManagedResource> resource) {
        SkASSERT(resource);
        fTrackedResources.push_back(std::move(resource));
    }

    // Add ref-counted resource that will be tracked and released when this command buffer finishes
    // execution. When it is released, it will signal that the resource can be recycled for reuse.
    void addRecycledResource(sk_sp<GrRecycledResource> resource) {
        fTrackedRecycledResources.push_back(std::move(resource));
    }

    void addingWork();
    virtual void onReset() {}

    void submitResourceBarriers();

    gr_cp<ID3D12GraphicsCommandList> fCommandList;

    SkSTArray<kInitialTrackedResourcesCount, sk_sp<GrManagedResource>> fTrackedResources;
    SkSTArray<kInitialTrackedResourcesCount, sk_sp<GrRecycledResource>> fTrackedRecycledResources;
    SkSTArray<kInitialTrackedResourcesCount, sk_sp<const GrBuffer>> fTrackedGpuBuffers;


    // When we create a command list it starts in an active recording state
    SkDEBUGCODE(bool fIsActive = true;)
    bool fHasWork = false;

private:
    void callFinishedCallbacks() { fFinishedCallbacks.reset(); }

    gr_cp<ID3D12CommandAllocator> fAllocator;

    SkSTArray<4, D3D12_RESOURCE_BARRIER> fResourceBarriers;

    SkTArray<sk_sp<GrRefCntedCallback>> fFinishedCallbacks;
};

class GrD3DDirectCommandList : public GrD3DCommandList {
public:
    static std::unique_ptr<GrD3DDirectCommandList> Make(ID3D12Device* device);

    ~GrD3DDirectCommandList() override = default;

    void setPipelineState(const sk_sp<GrD3DPipeline>& pipeline);

    void setStencilRef(unsigned int stencilRef);
    void setBlendFactor(const float blendFactor[4]);
    void setPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology);
    void setScissorRects(unsigned int numRects, const D3D12_RECT* rects);
    void setViewports(unsigned int numViewports, const D3D12_VIEWPORT* viewports);
    void setCenteredSamplePositions(unsigned int numSamples);
    void setDefaultSamplePositions();
    void setGraphicsRootSignature(const sk_sp<GrD3DRootSignature>& rootSignature);
    void setComputeRootSignature(const sk_sp<GrD3DRootSignature>& rootSignature);
    void setVertexBuffers(unsigned int startSlot,
                          sk_sp<const GrBuffer> vertexBuffer, size_t vertexStride,
                          sk_sp<const GrBuffer> instanceBuffer, size_t instanceStride);
    void setIndexBuffer(sk_sp<const GrBuffer> indexBuffer);
    void drawInstanced(unsigned int vertexCount, unsigned int instanceCount,
                       unsigned int startVertex, unsigned int startInstance);
    void drawIndexedInstanced(unsigned int indexCount, unsigned int instanceCount,
                              unsigned int startIndex, unsigned int baseVertex,
                              unsigned int startInstance);
    void executeIndirect(const sk_sp<GrD3DCommandSignature> commandSig, unsigned int maxCommandCnt,
                         const GrD3DBuffer* argumentBuffer, size_t argumentBufferOffset);
    void dispatch(unsigned int threadGroupCountX, unsigned int threadGroupCountY,
                  unsigned int threadGroupCountZ = 1);

    void clearRenderTargetView(const GrD3DRenderTarget* renderTarget,
                               std::array<float, 4> color,
                               const D3D12_RECT* rect);
    void clearDepthStencilView(const GrD3DAttachment*,
                               uint8_t stencilClearValue,
                               const D3D12_RECT* rect);
    void setRenderTarget(const GrD3DRenderTarget* renderTarget);
    void resolveSubresourceRegion(const GrD3DTextureResource* dstTexture,
                                  unsigned int dstX, unsigned int dstY,
                                  const GrD3DTextureResource* srcTexture,
                                  D3D12_RECT* srcRect);

    void setGraphicsRootConstantBufferView(unsigned int rootParameterIndex,
                                           D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);
    void setGraphicsRootDescriptorTable(unsigned int rootParameterIndex,
                                        D3D12_GPU_DESCRIPTOR_HANDLE bufferLocation);
    void setComputeRootConstantBufferView(unsigned int rootParameterIndex,
                                          D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);
    void setComputeRootDescriptorTable(unsigned int rootParameterIndex,
                                       D3D12_GPU_DESCRIPTOR_HANDLE bufferLocation);
    void setDescriptorHeaps(sk_sp<GrRecycledResource> srvCrvHeapResource,
                            ID3D12DescriptorHeap* srvDescriptorHeap,
                            sk_sp<GrRecycledResource> samplerHeapResource,
                            ID3D12DescriptorHeap* samplerDescriptorHeap);

    void addSampledTextureRef(GrD3DTexture*);

private:
    GrD3DDirectCommandList(gr_cp<ID3D12CommandAllocator> allocator,
                           gr_cp<ID3D12GraphicsCommandList> commandList);

    void onReset() override;

    const GrD3DPipeline* fCurrentPipeline = nullptr;
    const GrD3DRootSignature* fCurrentGraphicsRootSignature = nullptr;
    const GrD3DRootSignature* fCurrentComputeRootSignature = nullptr;
    const GrBuffer* fCurrentVertexBuffer = nullptr;
    size_t fCurrentVertexStride = 0;
    const GrBuffer* fCurrentInstanceBuffer = nullptr;
    size_t fCurrentInstanceStride = 0;
    const GrBuffer* fCurrentIndexBuffer = nullptr;
    bool fUsingCenteredSamples = false;

    D3D12_GPU_VIRTUAL_ADDRESS fCurrentGraphicsConstantBufferAddress = 0;
    D3D12_GPU_VIRTUAL_ADDRESS fCurrentComputeConstantBufferAddress = 0;
    D3D12_GPU_DESCRIPTOR_HANDLE fCurrentGraphicsRootDescTable[GrD3DRootSignature::kParamIndexCount];
    D3D12_GPU_DESCRIPTOR_HANDLE fCurrentComputeRootDescTable[GrD3DRootSignature::kParamIndexCount];
    const ID3D12DescriptorHeap* fCurrentSRVCRVDescriptorHeap = nullptr;
    const ID3D12DescriptorHeap* fCurrentSamplerDescriptorHeap = nullptr;
};

class GrD3DCopyCommandList : public GrD3DCommandList {
public:
    static std::unique_ptr<GrD3DCopyCommandList> Make(ID3D12Device* device);

private:
    GrD3DCopyCommandList(gr_cp<ID3D12CommandAllocator> allocator,
                         gr_cp<ID3D12GraphicsCommandList> commandList);
};
#endif
