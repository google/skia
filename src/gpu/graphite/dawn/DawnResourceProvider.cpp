/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnResourceProvider.h"

#include "include/core/SkString.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/gpu/graphite/dawn/DawnGraphiteTypes.h"
#include "include/private/base/SingleOwner.h"
#include "include/private/base/SkAlign.h"
#include "src/gpu/graphite/ComputePipeline.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/dawn/DawnBuffer.h"
#include "src/gpu/graphite/dawn/DawnCommandBuffer.h"
#include "src/gpu/graphite/dawn/DawnComputePipeline.h"
#include "src/gpu/graphite/dawn/DawnErrorChecker.h"
#include "src/gpu/graphite/dawn/DawnGraphicsPipeline.h"
#include "src/gpu/graphite/dawn/DawnGraphiteUtils.h"
#include "src/gpu/graphite/dawn/DawnSampler.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"
#include "src/gpu/graphite/dawn/DawnTexture.h"
#include "src/sksl/SkSLCompiler.h"

namespace skgpu::graphite {

namespace {

constexpr uint32_t kBufferBindingSizeAlignment = 16;
constexpr int kMaxNumberOfCachedBufferBindGroups = 1024;
constexpr int kMaxNumberOfCachedTextureBindGroups = 4096;

wgpu::ShaderModule create_shader_module(const wgpu::Device& device, const char* source) {
#if defined(__EMSCRIPTEN__)
    wgpu::ShaderModuleWGSLDescriptor wgslDesc;
#else
    wgpu::ShaderSourceWGSL wgslDesc;
#endif
    wgslDesc.code = source;
    wgpu::ShaderModuleDescriptor descriptor;
    descriptor.nextInChain = &wgslDesc;
    return device.CreateShaderModule(&descriptor);
}

wgpu::RenderPipeline create_blit_render_pipeline(const DawnSharedContext* sharedContext,
                                                 const char* label,
                                                 wgpu::ShaderModule shaderModule,
                                                 const char* vsEntryPoint,
                                                 const char* fsEntryPoint,
                                                 wgpu::TextureFormat renderPassColorFormat,
                                                 wgpu::TextureFormat renderPassDepthStencilFormat,
                                                 SampleCount sampleCount) {
    wgpu::RenderPipelineDescriptor descriptor;
    descriptor.label = label;
    descriptor.layout = nullptr;

    wgpu::ColorTargetState colorTarget;
    colorTarget.format = renderPassColorFormat;
    colorTarget.blend = nullptr;
    colorTarget.writeMask = wgpu::ColorWriteMask::All;

    wgpu::DepthStencilState depthStencil;
    if (renderPassDepthStencilFormat != wgpu::TextureFormat::Undefined) {
        depthStencil.format = renderPassDepthStencilFormat;
        depthStencil.depthWriteEnabled = false;
        depthStencil.depthCompare = wgpu::CompareFunction::Always;

        descriptor.depthStencil = &depthStencil;
    }

    wgpu::FragmentState fragment;
    fragment.module = shaderModule;
    fragment.entryPoint = fsEntryPoint;
    fragment.constantCount = 0;
    fragment.constants = nullptr;
    fragment.targetCount = 1;
    fragment.targets = &colorTarget;
    descriptor.fragment = &fragment;

    descriptor.vertex.module = shaderModule;
    descriptor.vertex.entryPoint = vsEntryPoint;
    descriptor.vertex.constantCount = 0;
    descriptor.vertex.constants = nullptr;
    descriptor.vertex.bufferCount = 0;
    descriptor.vertex.buffers = nullptr;

    descriptor.primitive.frontFace = wgpu::FrontFace::CCW;
    descriptor.primitive.cullMode = wgpu::CullMode::None;
    descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleStrip;
    descriptor.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;

    descriptor.multisample.count = (uint8_t) sampleCount;
    descriptor.multisample.mask = 0xFFFFFFFF;
    descriptor.multisample.alphaToCoverageEnabled = false;

    std::optional<DawnErrorChecker> errorChecker;
    if (sharedContext->dawnCaps()->allowScopedErrorChecks()) {
        errorChecker.emplace(sharedContext);
    }
    auto pipeline = sharedContext->device().CreateRenderPipeline(&descriptor);
    if (errorChecker.has_value() && errorChecker->popErrorScopes() != DawnErrorType::kNoError) {
        return nullptr;
    }

    return pipeline;
}

template <size_t NumEntries>
using BindGroupKey = typename DawnResourceProvider::BindGroupKey<NumEntries>;
using UniformBindGroupKey = BindGroupKey<DawnResourceProvider::kNumUniformEntries>;

UniformBindGroupKey make_ubo_bind_group_key(
        const std::array<std::pair<const DawnBuffer*, uint32_t>,
                         DawnResourceProvider::kNumUniformEntries>& boundBuffersAndSizes) {
    UniformBindGroupKey uniqueKey;
    {
        // Each entry in the bind group needs 2 uint32_t in the key:
        //  - buffer's unique ID: 32 bits.
        //  - buffer's binding size: 32 bits.
        // We need total of 4 entries in the uniform buffer bind group.
        // Unused entries will be assigned zero values.
        UniformBindGroupKey::Builder builder(&uniqueKey);

        for (uint32_t i = 0; i < boundBuffersAndSizes.size(); ++i) {
            const DawnBuffer* boundBuffer = boundBuffersAndSizes[i].first;
            const uint32_t bindingSize = boundBuffersAndSizes[i].second;
            if (boundBuffer) {
                builder[2 * i] = boundBuffer->uniqueID().asUInt();
                builder[2 * i + 1] = bindingSize;
            } else {
                builder[2 * i] = 0;
                builder[2 * i + 1] = 0;
            }
        }

        builder.finish();
    }

    return uniqueKey;
}

BindGroupKey<1> make_texture_bind_group_key(const DawnSampler* sampler,
                                            const DawnTexture* texture) {
    BindGroupKey<1> uniqueKey;
    {
        BindGroupKey<1>::Builder builder(&uniqueKey);

        builder[0] = sampler->uniqueID().asUInt();
        builder[1] = texture->uniqueID().asUInt();

        builder.finish();
    }

    return uniqueKey;
}
}  // namespace


// Wraps a Dawn buffer, and tracks the intrinsic blocks residing in this buffer.
class DawnResourceProvider::IntrinsicBuffer final {
public:
    static constexpr int kNumSlots = 8;

    IntrinsicBuffer(sk_sp<DawnBuffer> dawnBuffer) : fDawnBuffer(std::move(dawnBuffer)) {}
    ~IntrinsicBuffer() = default;

    sk_sp<DawnBuffer> buffer() const { return fDawnBuffer; }

    // Track that 'intrinsicValues' is stored in the buffer at the 'offset'.
    void trackIntrinsic(UniformDataBlock intrinsicValues, uint32_t offset) {
        fCachedIntrinsicValues.set(UniformDataBlock::Make(intrinsicValues, &fUniformData), offset);
    }

    // Find the offset of 'intrinsicValues' in the buffer. If not found, return nullptr.
    uint32_t* findIntrinsic(UniformDataBlock intrinsicValues) const {
        return fCachedIntrinsicValues.find(intrinsicValues);
    }

    int slotsUsed() const { return fCachedIntrinsicValues.count(); }

    void updateAccessTime() {
        fLastAccess = skgpu::StdSteadyClock::now();
    }
    skgpu::StdSteadyClock::time_point lastAccessTime() const {
        return fLastAccess;
    }

private:
    skia_private::THashMap<UniformDataBlock, uint32_t, UniformDataBlock::Hash>
        fCachedIntrinsicValues;
    SkArenaAlloc fUniformData{0};

    sk_sp<DawnBuffer> fDawnBuffer;
    skgpu::StdSteadyClock::time_point fLastAccess;

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(IntrinsicBuffer);
};

// DawnResourceProvider::IntrinsicConstantsManager
// ----------------------------------------------------------------------------

/**
 * Since Dawn does not currently provide push constants, this helper class manages rotating through
 * buffers and writing each new occurrence of a set of intrinsic uniforms into the current buffer.
 */
class DawnResourceProvider::IntrinsicConstantsManager {
public:
    explicit IntrinsicConstantsManager(DawnResourceProvider* resourceProvider)
            : fResourceProvider(resourceProvider) {}

    ~IntrinsicConstantsManager() {
        auto alwaysTrue = [](IntrinsicBuffer* buffer) { return true; };
        this->purgeBuffersIf(alwaysTrue);

        SkASSERT(fIntrinsicBuffersLRU.isEmpty());
    }

    // Find or create a bind buffer info for the given intrinsic values used in the given command
    // buffer.
    BindBufferInfo add(DawnCommandBuffer* cb, UniformDataBlock intrinsicValues);

    void purgeResourcesNotUsedSince(StdSteadyClock::time_point purgeTime) {
        auto bufferNotUsedSince = [purgeTime, this](IntrinsicBuffer* buffer) {
            // We always keep the current buffer as it is likely to be used again soon.
            return buffer != fCurrentBuffer && buffer->lastAccessTime() < purgeTime;
        };
        this->purgeBuffersIf(bufferNotUsedSince);
    }

    void freeGpuResources() { this->purgeResourcesNotUsedSince(skgpu::StdSteadyClock::now()); }

private:
    // The max number of intrinsic buffers to keep around in the cache.
    static constexpr uint32_t kMaxNumBuffers = 16;

    // Traverse the intrinsic buffers and purge the ones that match the 'pred'.
    template<typename T> void purgeBuffersIf(T pred);

    DawnResourceProvider* const fResourceProvider;
    // The current buffer being filled up, as well as the how much of it has been written to.
    IntrinsicBuffer* fCurrentBuffer = nullptr;

    // All cached intrinsic buffers, in LRU order.
    SkTInternalLList<IntrinsicBuffer> fIntrinsicBuffersLRU;
    // The number of intrinsic buffers currently in the cache.
    uint32_t fNumBuffers = 0;
};

// Find or create a bind buffer info for the given intrinsic values used in the given command
// buffer.
BindBufferInfo DawnResourceProvider::IntrinsicConstantsManager::add(
        DawnCommandBuffer* cb, UniformDataBlock intrinsicValues) {
    using Iter = SkTInternalLList<IntrinsicBuffer>::Iter;
    Iter iter;
    auto* curr = iter.init(fIntrinsicBuffersLRU, Iter::kHead_IterStart);
    uint32_t* offset = nullptr;
    // Find the buffer that contains the given intrinsic values.
    while (curr != nullptr) {
        offset = curr->findIntrinsic(intrinsicValues);
        if (offset != nullptr) {
            break;
        }
        curr = iter.next();
    }
    // If we found the buffer, we can return the bind buffer info directly.
    if (curr != nullptr && offset != nullptr) {
        // Move the buffer to the head of the LRU list.
        fIntrinsicBuffersLRU.remove(curr);
        fIntrinsicBuffersLRU.addToHead(curr);
        // Track the dawn buffer's usage by the command buffer.
        cb->trackResource(curr->buffer());
        curr->updateAccessTime();
        return {curr->buffer().get(), *offset, SkTo<uint32_t>(intrinsicValues.size())};
    }

    // TODO: https://b.corp.google.com/issues/259267703
    // Make updating intrinsic constants faster. Metal has setVertexBytes method to quickly send
    // intrinsic constants to vertex shader without any buffer. But Dawn doesn't have similar
    // capability. So we have to use WriteBuffer(), and this method is not allowed to be called when
    // there is an active render pass.
    SkASSERT(!cb->hasActivePassEncoder());

    const Caps* caps = fResourceProvider->dawnSharedContext()->caps();
    const uint32_t stride =
            SkAlignTo(intrinsicValues.size(), caps->requiredUniformBufferAlignment());
    // In any one of the following cases, we need to create a new buffer:
    //     (1) There is no current buffer.
    //     (2) The current buffer is full.
    if (!fCurrentBuffer || fCurrentBuffer->slotsUsed() == IntrinsicBuffer::kNumSlots) {
        // We can just replace the current buffer; any prior buffer was already tracked in the LRU
        // list and the intrinsic constants were written directly to the Dawn queue.
        DawnResourceProvider* resourceProvider = fResourceProvider;
        auto dawnBuffer =
                resourceProvider->findOrCreateDawnBuffer(stride * IntrinsicBuffer::kNumSlots,
                                                         BufferType::kUniform,
                                                         AccessPattern::kGpuOnly,
                                                         "IntrinsicConstantBuffer");
        if (!dawnBuffer) {
            // If we failed to create a GPU buffer to hold the intrinsic uniforms, we will fail the
            // Recording being inserted, so return an empty bind info.
            return {};
        }

        fCurrentBuffer = new IntrinsicBuffer(dawnBuffer);
        fIntrinsicBuffersLRU.addToHead(fCurrentBuffer);
        fNumBuffers++;
        // If we have too many buffers, remove the least used one.
        if (fNumBuffers > kMaxNumBuffers) {
            auto* tail = fIntrinsicBuffersLRU.tail();
            fIntrinsicBuffersLRU.remove(tail);
            delete tail;
            fNumBuffers--;
        }
    }

    SkASSERT(fCurrentBuffer && fCurrentBuffer->slotsUsed() < IntrinsicBuffer::kNumSlots);
    uint32_t newOffset = (fCurrentBuffer->slotsUsed()) * stride;
    fResourceProvider->dawnSharedContext()->queue().WriteBuffer(
            fCurrentBuffer->buffer()->dawnBuffer(),
            newOffset,
            intrinsicValues.data(),
            intrinsicValues.size());

    // Track the intrinsic values in the buffer.
    fCurrentBuffer->trackIntrinsic(intrinsicValues, newOffset);

    cb->trackResource(fCurrentBuffer->buffer());
    fCurrentBuffer->updateAccessTime();

    return {fCurrentBuffer->buffer().get(), newOffset, SkTo<uint32_t>(intrinsicValues.size())};
}

template <typename T> void DawnResourceProvider::IntrinsicConstantsManager::purgeBuffersIf(T pred) {
    using Iter = SkTInternalLList<IntrinsicBuffer>::Iter;
    Iter iter;
    auto* curr = iter.init(fIntrinsicBuffersLRU, Iter::kHead_IterStart);
    while (curr != nullptr) {
        auto* next = iter.next();
        if (pred(curr)) {
            fIntrinsicBuffersLRU.remove(curr);
            fNumBuffers--;
            delete curr;
        }
        curr = next;
    }
}

// DawnResourceProvider::IntrinsicConstantsManager
// ----------------------------------------------------------------------------

// DawnResourceProvider::BlitWithDrawEncoder
DawnResourceProvider::BlitWithDrawEncoder::BlitWithDrawEncoder(wgpu::RenderPipeline pipeline,
                                                             bool srcIsMSAA)
        : fPipeline(std::move(pipeline)), fSrcIsMSAA(srcIsMSAA) {}

void DawnResourceProvider::BlitWithDrawEncoder::EncodeBlit(
        const wgpu::Device& device,
        const wgpu::RenderPassEncoder& renderEncoder,
        const wgpu::TextureView& srcTextureView,
        const SkIPoint& srcOffset,
        const SkIRect& dstBounds) {
    SkASSERT(fPipeline);
    renderEncoder.SetPipeline(fPipeline);

    // TODO(b/260368758): cache single texture's bind group creation.
    wgpu::BindGroupEntry entry;
    entry.binding = fSrcIsMSAA ? 1 : 0;
    entry.textureView = srcTextureView;

    wgpu::BindGroupDescriptor desc;
    desc.layout = fPipeline.GetBindGroupLayout(0);
    desc.entryCount = 1;
    desc.entries = &entry;

    auto bindGroup = device.CreateBindGroup(&desc);

    renderEncoder.SetBindGroup(0, bindGroup);

    renderEncoder.SetScissorRect(
            dstBounds.left(), dstBounds.top(), dstBounds.width(), dstBounds.height());
    renderEncoder.SetViewport(
            dstBounds.left(), dstBounds.top(), dstBounds.width(), dstBounds.height(), 0, 1);

    // In fragment shader, the sampling coords are calculated as:
    // - x = fragPosition.x - dstX + srcX = fragPosition.x - (dxtX - srcX)
    // - y = fragPosition.y - dstY + srcX = fragPosition.y - (dxtY - srcY)
    int32_t deltaX = dstBounds.left() - srcOffset.x();
    int32_t deltaY = dstBounds.top() - srcOffset.y();
    // Since texture's sizes are never larger than 16 bits, we can encode the offsets's (x, y)
    // in one single 32 bits instance index value. In future, once push constants are implemented in
    // Dawn, we should use them instead.
    SkASSERT(std::abs(deltaX) < std::numeric_limits<int16_t>::max());
    SkASSERT(std::abs(deltaY) < std::numeric_limits<int16_t>::max());
    int32_t baseInstance = (deltaX & 0xffff) | (deltaY << 16);

    renderEncoder.Draw(/*vertexCount=*/3,
                       /*instanceCount=*/ 1,
                       /*firstVertex=*/0,
                       /*firstInstance=*/baseInstance);
}

// ----------------------------------------------------------------------------
DawnResourceProvider::DawnResourceProvider(SharedContext* sharedContext,
                                           SingleOwner* singleOwner,
                                           uint32_t recorderID,
                                           size_t resourceBudget)
        : ResourceProvider(sharedContext, singleOwner, recorderID, resourceBudget)
        , fUniformBufferBindGroupCache(kMaxNumberOfCachedBufferBindGroups)
        , fSingleTextureSamplerBindGroups(kMaxNumberOfCachedTextureBindGroups)
        , fSingleOwner(singleOwner) {
    fIntrinsicConstantsManager = std::make_unique<IntrinsicConstantsManager>(this);

    // Only used for debug asserts so this avoids compile errors.
    (void)fSingleOwner;
}

DawnResourceProvider::~DawnResourceProvider() = default;

DawnResourceProvider::BlitWithDrawEncoder DawnResourceProvider::findOrCreateBlitWithDrawEncoder(
        const RenderPassDesc& renderPassDesc, SampleCount srcSampleCount) {
    // Currently Dawn only supports k1 and k4. So we can optimize the pipeline key by specifying
    // whether the source has MSAA or not.
    SkASSERT(srcSampleCount == SampleCount::k1 || srcSampleCount == SampleCount::k4);
    const bool srcIsMSAA = srcSampleCount > SampleCount::k1;
    const uint32_t pipelineKey = this->dawnSharedContext()->dawnCaps()->getRenderPassDescKeyForPipeline(
            renderPassDesc, srcIsMSAA);
    wgpu::RenderPipeline pipeline = fBlitWithDrawPipelines[pipelineKey];
    if (!pipeline) {
        // Since texture's sizes are never larger than 16 bits, we can encode the offsets's (x, y)
        // in one single 32 bits instance index value.
        static constexpr char kShaderSrc[] =
            "struct VertexOutput {"
                "@builtin(position) position: vec4f,"
                "@location(1) @interpolate(flat, either) srcOffset: vec2i,"
            "};"
            "var<private> fullscreenTriPositions : array<vec2<f32>, 3> = array<vec2<f32>, 3>("
                "vec2(-1.0, -1.0), vec2(-1.0, 3.0), vec2(3.0, -1.0));"

            "@vertex "
            "fn VS(@builtin(vertex_index) vertexIndex : u32,"
                  "@builtin(instance_index) instanceIndex : u32)"
                  "-> VertexOutput {"
                "var out: VertexOutput;"
                "out.position = vec4(fullscreenTriPositions[vertexIndex], 1.0, 1.0);"
                "var srcOffset = vec2u("
                    "bitcast<u32>(instanceIndex & 0xffff),"
                    "bitcast<u32>(instanceIndex >> 16)"
                ");"

                // Sign extending from 16 bits to 32 bits
                "let hasSignBit = (srcOffset & vec2u(0x8000)) != vec2u(0u);"
                "srcOffset = select(srcOffset, srcOffset | vec2u(0xffff0000), hasSignBit);"

                "out.srcOffset = bitcast<vec2i>(srcOffset);"

                "return out;"
            "}"

            "fn getSamplingCoords(input: VertexOutput) -> vec2i {"
                "var coords : vec2<i32> = vec2<i32>(i32(input.position.x), i32(input.position.y));"
                "return coords - input.srcOffset;"
            "}"

            "@group(0) @binding(0) var colorMap: texture_2d<f32>;"
            "@fragment "
            "fn SampleFS(input: VertexOutput) -> @location(0) vec4<f32> {"
                "let coords = getSamplingCoords(input);"
                "return textureLoad(colorMap, coords, 0);"
            "}"

            "@group(0) @binding(1) var msColorMap: texture_multisampled_2d<f32>;"
            "@fragment\n"
            "fn SampleMSAAFS(input: VertexOutput) -> @location(0) vec4<f32> {"
                "let coords = getSamplingCoords(input);"
                "const sampleCount = %u;"
                "var sum = vec4f(0.0);"
                "for (var i: u32 = 0; i < sampleCount; i = i + 1) {"
                    "sum += textureLoad(msColorMap, coords, i);"
                "}"
                "return sum * (1.0 / f32(sampleCount));"
            "}";

        auto shaderModule = create_shader_module(
                dawnSharedContext()->device(),
                SkStringPrintf(kShaderSrc, (unsigned) srcSampleCount).c_str());

        pipeline = create_blit_render_pipeline(
                dawnSharedContext(),
                /*label=*/"BlitWithDraw",
                std::move(shaderModule),
                "VS",
                srcIsMSAA ? "SampleMSAAFS": "SampleFS",
                /*renderPassColorFormat=*/
                TextureFormatToDawnFormat(renderPassDesc.fColorAttachment.fFormat),
                /*renderPassDepthStencilFormat=*/
                TextureFormatToDawnFormat(renderPassDesc.fDepthStencilAttachment.fFormat),
                renderPassDesc.fColorAttachment.fSampleCount);

        if (pipeline) {
            fBlitWithDrawPipelines.set(pipelineKey, pipeline);
        }
    }

    return BlitWithDrawEncoder(std::move(pipeline), srcIsMSAA);
}

sk_sp<Texture> DawnResourceProvider::onCreateWrappedTexture(const BackendTexture& texture) {
    // Convert to smart pointers. wgpu::Texture* constructor will increment the ref count.
    wgpu::Texture dawnTexture         = BackendTextures::GetDawnTexturePtr(texture);
    wgpu::TextureView dawnTextureView = BackendTextures::GetDawnTextureViewPtr(texture);
    SkASSERT(!dawnTexture || !dawnTextureView);

    if (!dawnTexture && !dawnTextureView) {
        return {};
    }

    if (dawnTexture) {
        return DawnTexture::MakeWrapped(this->dawnSharedContext(),
                                        texture.dimensions(),
                                        texture.info(),
                                        std::move(dawnTexture));
    } else {
        return DawnTexture::MakeWrapped(this->dawnSharedContext(),
                                        texture.dimensions(),
                                        texture.info(),
                                        std::move(dawnTextureView));
    }
}

sk_sp<DawnTexture> DawnResourceProvider::findOrCreateDiscardableMSAALoadTexture(
        SkISize dimensions, const TextureInfo& msaaInfo) {
    SkASSERT(msaaInfo.isValid());

    // Derive the load texture's info from MSAA texture's info.
    DawnTextureInfo dawnMsaaLoadTextureInfo = TextureInfoPriv::Get<DawnTextureInfo>(msaaInfo);
    dawnMsaaLoadTextureInfo.fSampleCount = SampleCount::k1;
    dawnMsaaLoadTextureInfo.fUsage |= wgpu::TextureUsage::TextureBinding;

#if !defined(__EMSCRIPTEN__)
    // MSAA texture can be transient attachment (memoryless) but the load texture cannot be.
    // This is because the load texture will need to have its content retained between two passes
    // loading:
    // - first pass: the resolve texture is blitted to the load texture.
    // - 2nd pass: the actual render pass is started and the load texture is blitted to the MSAA
    // texture.
    dawnMsaaLoadTextureInfo.fUsage &= (~wgpu::TextureUsage::TransientAttachment);
#endif

    auto texture = this->findOrCreateShareableTexture(
            dimensions,
            TextureInfos::MakeDawn(dawnMsaaLoadTextureInfo),
            "DiscardableLoadMSAATexture");

    return sk_sp<DawnTexture>(static_cast<DawnTexture*>(texture.release()));
}

sk_sp<ComputePipeline> DawnResourceProvider::createComputePipeline(
        const ComputePipelineDesc& desc) {
    return DawnComputePipeline::Make(this->dawnSharedContext(), desc);
}

sk_sp<Texture> DawnResourceProvider::createTexture(SkISize dimensions, const TextureInfo& info) {
    return DawnTexture::Make(this->dawnSharedContext(), dimensions, info);
}

sk_sp<Buffer> DawnResourceProvider::createBuffer(size_t size,
                                                 BufferType type,
                                                 AccessPattern accessPattern) {
    return DawnBuffer::Make(this->dawnSharedContext(), size, type, accessPattern);
}

sk_sp<Sampler> DawnResourceProvider::createSampler(const SamplerDesc& samplerDesc) {
    return DawnSampler::Make(this->dawnSharedContext(), samplerDesc);
}

BackendTexture DawnResourceProvider::onCreateBackendTexture(SkISize dimensions,
                                                            const TextureInfo& info) {
    wgpu::Texture texture = DawnTexture::MakeDawnTexture(this->dawnSharedContext(),
                                                         dimensions,
                                                         info);
    if (!texture) {
        return {};
    }

    return BackendTextures::MakeDawn(texture.MoveToCHandle());
}

void DawnResourceProvider::onDeleteBackendTexture(const BackendTexture& texture) {
    SkASSERT(texture.isValid());
    SkASSERT(texture.backend() == BackendApi::kDawn);

    // Automatically release the pointers in wgpu::TextureView & wgpu::Texture's dtor.
    // Acquire() won't increment the ref count.
    wgpu::TextureView::Acquire(BackendTextures::GetDawnTextureViewPtr(texture));
    // We need to explicitly call Destroy() here since since that is the recommended way to delete
    // a Dawn texture predictably versus just dropping a ref and relying on garbage collection.
    //
    // Additionally this helps to work around an issue where Skia may have cached a BindGroup that
    // references the underlying texture. Skia currently doesn't destroy BindGroups when its use of
    // the texture goes away, thus a ref to the texture remains on the BindGroup and memory is never
    // cleared up unless we call Destroy() here.
    wgpu::Texture::Acquire(BackendTextures::GetDawnTexturePtr(texture)).Destroy();
}

DawnSharedContext* DawnResourceProvider::dawnSharedContext() const {
    return static_cast<DawnSharedContext*>(fSharedContext);
}

sk_sp<DawnBuffer> DawnResourceProvider::findOrCreateDawnBuffer(size_t size,
                                                               BufferType type,
                                                               AccessPattern accessPattern,
                                                               std::string_view label) {
    sk_sp<Buffer> buffer = this->findOrCreateNonShareableBuffer(size, type, accessPattern, label);
    DawnBuffer* ptr = static_cast<DawnBuffer*>(buffer.release());
    return sk_sp<DawnBuffer>(ptr);
}

const wgpu::Buffer& DawnResourceProvider::getOrCreateNullBuffer() {
    if (!fNullBuffer) {
        wgpu::BufferDescriptor desc;
        if (fSharedContext->caps()->setBackendLabels()) {
            desc.label = "UnusedBufferSlot";
        }
        desc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform |
                     wgpu::BufferUsage::Storage;
        desc.size = kBufferBindingSizeAlignment;
        desc.mappedAtCreation = false;

        fNullBuffer = this->dawnSharedContext()->device().CreateBuffer(&desc);
        SkASSERT(fNullBuffer);
    }

    return fNullBuffer;
}

const wgpu::BindGroup& DawnResourceProvider::findOrCreateUniformBuffersBindGroup(
        const std::array<std::pair<const DawnBuffer*, uint32_t>, kNumUniformEntries>&
                boundBuffersAndSizes) {
    SKGPU_ASSERT_SINGLE_OWNER(fSingleOwner)

    auto key = make_ubo_bind_group_key(boundBuffersAndSizes);
    auto* existingBindGroup = fUniformBufferBindGroupCache.find(key);
    if (existingBindGroup) {
        // cache hit.
        return *existingBindGroup;
    }

    // Translate to wgpu::BindGroupDescriptor
    std::array<wgpu::BindGroupEntry, kNumUniformEntries> entries;

    constexpr uint32_t kBindingIndices[] = {
        DawnGraphicsPipeline::kIntrinsicUniformBufferIndex,
        DawnGraphicsPipeline::kCombinedUniformIndex,
        DawnGraphicsPipeline::kGradientBufferIndex,
    };

    for (uint32_t i = 0; i < boundBuffersAndSizes.size(); ++i) {
        const DawnBuffer* boundBuffer = boundBuffersAndSizes[i].first;
        const uint32_t bindingSize = boundBuffersAndSizes[i].second;

        entries[i].binding = kBindingIndices[i];
        entries[i].offset = 0;
        if (boundBuffer) {
            entries[i].buffer = boundBuffer->dawnBuffer();
            entries[i].size = SkAlignTo(bindingSize, kBufferBindingSizeAlignment);
        } else {
            entries[i].buffer = this->getOrCreateNullBuffer();
            entries[i].size = wgpu::kWholeSize;
        }
    }

    wgpu::BindGroupDescriptor desc;
    desc.layout = this->dawnSharedContext()->getUniformBuffersBindGroupLayout();
    desc.entryCount = entries.size();
    desc.entries = entries.data();

    const auto& device = this->dawnSharedContext()->device();
    auto bindGroup = device.CreateBindGroup(&desc);

    return *fUniformBufferBindGroupCache.insert(key, bindGroup);
}

const wgpu::BindGroup& DawnResourceProvider::findOrCreateSingleTextureSamplerBindGroup(
        const DawnSampler* sampler, const DawnTexture* texture) {
    SKGPU_ASSERT_SINGLE_OWNER(fSingleOwner)

    auto key = make_texture_bind_group_key(sampler, texture);
    auto* existingBindGroup = fSingleTextureSamplerBindGroups.find(key);
    if (existingBindGroup) {
        // cache hit.
        return *existingBindGroup;
    }

    std::array<wgpu::BindGroupEntry, 2> entries;

    entries[0].binding = 0;
    entries[0].sampler = sampler->dawnSampler();
    entries[1].binding = 1;
    entries[1].textureView = texture->sampleTextureView();

    wgpu::BindGroupDescriptor desc;
    desc.layout = this->dawnSharedContext()->getSingleTextureSamplerBindGroupLayout();
    desc.entryCount = entries.size();
    desc.entries = entries.data();

    const auto& device = this->dawnSharedContext()->device();
    auto bindGroup = device.CreateBindGroup(&desc);

    return *fSingleTextureSamplerBindGroups.insert(key, bindGroup);
}

void DawnResourceProvider::onFreeGpuResources() {
    SKGPU_ASSERT_SINGLE_OWNER(fSingleOwner)

    fIntrinsicConstantsManager->freeGpuResources();
    // The wgpu::Textures and wgpu::Buffers held by the BindGroups should be explicitly destroyed
    // when the DawnTexture and DawnBuffer is destroyed, but removing the bind groups themselves
    // helps reduce CPU memory periodically.
    fSingleTextureSamplerBindGroups.reset();
    fUniformBufferBindGroupCache.reset();
}

void DawnResourceProvider::onPurgeResourcesNotUsedSince(StdSteadyClock::time_point purgeTime) {
    fIntrinsicConstantsManager->purgeResourcesNotUsedSince(purgeTime);
}

BindBufferInfo DawnResourceProvider::findOrCreateIntrinsicBindBufferInfo(
        DawnCommandBuffer* cb, UniformDataBlock intrinsicValues) {
    return fIntrinsicConstantsManager->add(cb, intrinsicValues);
}

DawnThreadSafeResourceProvider::DawnThreadSafeResourceProvider(
        std::unique_ptr<ResourceProvider> resourceProvider)
    : ThreadSafeResourceProvider(std::move(resourceProvider)) {}

} // namespace skgpu::graphite
