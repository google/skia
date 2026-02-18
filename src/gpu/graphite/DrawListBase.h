/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_DrawListBase_DEFINED
#define skgpu_graphite_DrawListBase_DEFINED

#include "include/gpu/graphite/GraphiteTypes.h"

#include "include/private/base/SkDebug.h"
#include "src/base/SkBlockAllocator.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkTBlockList.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawCommands.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/Transform.h"

#include <array>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>

namespace skgpu::graphite {

class DrawPass;
class Geometry;
class Renderer;
class Recorder;

struct Layer;

/**
 * The base interface for recording draw commands. DrawList implements the existing Graphite
 * drawing behavior. In the next CL, DrawListLayer.h will implement the experimental layer based
 * method.
 */
class DrawListBase {
public:
    // TODO (thomsmit): Remove this limit when DrawListLayer is used
    static constexpr int kMaxRenderSteps = 4096;
    static_assert(kMaxRenderSteps <= std::numeric_limits<uint16_t>::max());

    DrawListBase() {}
    virtual ~DrawListBase() = default;

    virtual std::pair<DrawParams*, Layer*> recordDraw(
            const Renderer* renderer,
            const Transform& localToDevice,
            const Geometry& geometry,
            const Clip& clip,
            DrawOrder ordering,
            UniquePaintParamsID paintID,
            SkEnumBitMask<DstUsage> dstUsage,
            BarrierType barrierBeforeDraws,
            PipelineDataGatherer* gatherer,
            const StrokeStyle* stroke,
            const Layer* latestDepthLayer) = 0;

    virtual std::unique_ptr<DrawPass> snapDrawPass(Recorder* recorder,
                                                   sk_sp<TextureProxy> target,
                                                   const SkImageInfo& targetInfo,
                                                   const DstReadStrategy dstReadStrategy) = 0;

    int renderStepCount() const { return fRenderStepCount; }

    bool modifiesTarget() const {
        return this->renderStepCount() > 0 || fLoadOp == LoadOp::kClear;
    }

    bool samplesTexture(const TextureProxy* texture) const {
        return fTextureDataCache.hasTexture(texture);
    }

    const Rect& dstReadBounds() const { return fDstReadBounds; }
    const Rect& passBounds() const { return fPassBounds; }
    bool drawsReadDst() const { return !fDstReadBounds.isEmptyNegativeOrNaN(); }
    bool drawsRequireMSAA() const { return fRequiresMSAA; }
    SkEnumBitMask<DepthStencilFlags> depthStencilFlags() const { return fDepthStencilFlags; }

    SkDEBUGCODE(bool hasCoverageMaskDraws() const { return fCoverageMaskShapeDrawCount > 0; })

    virtual void reset(LoadOp op, SkColor4f clearColor = {0.f, 0.f, 0.f, 0.f}) {
        fLoadOp = op;
        fClearColor = clearColor.premul().array();
        fRenderStepCount = 0;
        fDstReadBounds = Rect::InfiniteInverted();
        fPassBounds = Rect::InfiniteInverted();
        fRequiresMSAA = false;
        fDepthStencilFlags = DepthStencilFlags::kNone;

        fTransforms.reset();
        fUniformDataCache.reset();
        fTextureDataCache.reset();
        fPipelineCache.reset();

        SkDEBUGCODE(fCoverageMaskShapeDrawCount = 0;)
    }

protected:
    const Transform& deduplicateTransform(const Transform& localToDevice) {
        if (fTransforms.empty() || fTransforms.back() != localToDevice) {
            fTransforms.push_back(localToDevice);
        }
        return fTransforms.back();
    }

    // TODO (thomsmit): Once sort-based draw ordering is removed, try putting the transforms onto
    // the DrawListLayer's arena allocator.
    SkTBlockList<Transform, 4> fTransforms{SkBlockAllocator::GrowthPolicy::kFibonacci};

    UniformDataCache fUniformDataCache;
    TextureDataCache fTextureDataCache;
    GraphicsPipelineCache fPipelineCache;

    int fRenderStepCount = 0;

    Rect fDstReadBounds = Rect::InfiniteInverted();
    Rect fPassBounds = Rect::InfiniteInverted();
    bool fRequiresMSAA = false;
    SkEnumBitMask<DepthStencilFlags> fDepthStencilFlags = DepthStencilFlags::kNone;

    LoadOp fLoadOp = LoadOp::kLoad;
    std::array<float, 4> fClearColor = {0.f, 0.f, 0.f, 0.f};

#if defined(SK_DEBUG)
    int fCoverageMaskShapeDrawCount = 0;
#endif

    // Writes uniform data either to uniform buffers or to shared storage buffers, and tracks when
    // bindings need to change between draws.
    class UniformTracker {
    public:
        UniformTracker(bool useStorageBuffers) : fUseStorageBuffers(useStorageBuffers) {}

        bool writeUniforms(UniformDataCache& uniformCache,
                           DrawBufferManager* bufferMgr,
                           UniformDataCache::Index index) {
            if (index >= UniformDataCache::kInvalidIndex) {
                return false;
            }

            if (index == fLastIndex) {
                return false;
            }
            fLastIndex = index;

            UniformDataCache::Entry& uniformData = uniformCache.lookup(index);
            const size_t uniformDataSize = uniformData.fCpuData.size();

            // Upload the uniform data if we haven't already.
            // Alternatively, re-upload the uniform data to avoid a rebind if we're using storage
            // buffers. This will result in more data uploaded, but the tradeoff seems worthwhile.
            if (!uniformData.fBufferBinding.fBuffer ||
                (fUseStorageBuffers &&
                 uniformData.fBufferBinding.fBuffer != fLastBinding.fBuffer)) {
                BufferWriter writer;
                std::tie(writer, uniformData.fBufferBinding) =
                        fCurrentBuffer.getMappedSubrange(1, uniformDataSize);
                if (!writer) {
                    // Allocate a new buffer
                    std::tie(writer, uniformData.fBufferBinding, fCurrentBuffer) =
                            fUseStorageBuffers
                                    ? bufferMgr->getMappedStorageBuffer(1, uniformDataSize)
                                    : bufferMgr->getMappedUniformBuffer(1, uniformDataSize);
                    if (!writer) {
                        return {};  // Allocation failed so early out
                    }
                }

                writer.write(uniformData.fCpuData.data(), uniformDataSize);

                if (fUseStorageBuffers) {
                    // When using storage buffers, store the SSBO index in the binding's offset
                    // field and always use the entire buffer's size in the size field.
                    SkASSERT(uniformData.fBufferBinding.fOffset % uniformDataSize == 0);
                    uniformData.fBufferBinding.fOffset /= uniformDataSize;
                    uniformData.fBufferBinding.fSize = uniformData.fBufferBinding.fBuffer->size();
                } else {
                    // Every new set of uniform data has to be bound, this ensures its aligned
                    // correctly
                    fCurrentBuffer.resetForNewBinding();
                }
            }

            const bool needsRebind = uniformData.fBufferBinding.fBuffer != fLastBinding.fBuffer ||
                                     (!fUseStorageBuffers &&
                                      uniformData.fBufferBinding.fOffset != fLastBinding.fOffset);

            fLastBinding = uniformData.fBufferBinding;

            return needsRebind;
        }

        void bindUniforms(UniformSlot slot, DrawPassCommands::List* commandList) {
            BindBufferInfo binding = fLastBinding;
            if (fUseStorageBuffers) {
                // Track the SSBO index in fLastBinding, but set offset = 0 in the actual used
                // binding.
                binding.fOffset = 0;
            }
            commandList->bindUniformBuffer(binding, slot);
        }

        uint32_t ssboIndex() const {
            // The SSBO index for the last-bound storage buffer is stored in the binding's offset
            // field.
            return fLastBinding.fOffset;
        }

    private:
        // The GPU buffer data is being written into; for SSBOs, Graphite will only record a bind
        // command when this changes. Sub-allocations will be aligned such that they can be randomly
        // accessed even if the data is heterogenous. UBOs will always have to issue binding
        // commands when a draw needs to use a different set of uniform values.
        BufferSubAllocator fCurrentBuffer;

        // Internally track the last binding returned, so that we know whether new uploads or
        // rebindings are necessary. If we're using SSBOs, this is treated specially -- the fOffset
        // field holds the index in the storage buffer of the last-written uniforms, and the offsets
        // used for actual bindings are always zero.
        BindBufferInfo fLastBinding;

        // This keeps track of the last index used for writing uniforms from a provided uniform
        // cache. If a provided index matches the last index, the uniforms are assumed to already be
        // written and no additional uploading is performed. This assumes a UniformTracker will
        // always be provided with the same uniform cache.
        UniformDataCache::Index fLastIndex = UniformDataCache::kInvalidIndex;

        const bool fUseStorageBuffers;
    };

    // Tracks when to issue BindTexturesAndSamplers commands to a command list and converts
    // TextureDataBlocks to that representation as needed.
    class TextureTracker {
    public:
        TextureTracker(TextureDataCache* textureCache) : fTextureCache(textureCache) {}

        bool setCurrentTextureBindings(TextureDataCache::Index bindingIndex) {
            if (bindingIndex < TextureDataCache::kInvalidIndex && fLastIndex != bindingIndex) {
                fLastIndex = bindingIndex;
                return true;
            }
            // No binding change
            return false;
        }

        void bindTextures(DrawPassCommands::List* commandList) {
            SkASSERT(fLastIndex < TextureDataCache::kInvalidIndex);
            TextureDataBlock binding = fTextureCache->lookup(fLastIndex);

            auto [textures, samplers] =
                    commandList->bindDeferredTexturesAndSamplers(binding.numTextures());

            for (int i = 0; i < binding.numTextures(); ++i) {
                auto [t, s] = binding.texture(i);
                textures[i] = t.get();
                samplers[i] = s;
            }
        }

    private:
        TextureDataCache::Index fLastIndex = TextureDataCache::kInvalidIndex;

        TextureDataCache* const fTextureCache;
    };
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_DrawListBase_DEFINED
