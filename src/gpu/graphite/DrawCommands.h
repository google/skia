/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DrawPassCommands_DEFINED
#define skgpu_graphite_DrawPassCommands_DEFINED

#include "include/core/SkRect.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkTBlockList.h"
#include "src/gpu/graphite/CommandTypes.h"
#include "src/gpu/graphite/DrawTypes.h"

namespace skgpu::graphite {

namespace DrawPassCommands {

// A list of all the commands types used by a DrawPass.
// Each of these is reified into a struct below.
//
// The design of this systems is based on SkRecords.

// (We're using the macro-of-macro trick here to do several different things with the same list.)
//
// We leave this SKGPU_DRAW_COMMAND_TYPES macro defined for use by code that wants to operate on
// DrawPassCommands types polymorphically.
#define SKGPU_DRAW_PASS_COMMAND_TYPES(M) \
    M(BindGraphicsPipeline)              \
    M(SetBlendConstants)                 \
    M(BindUniformBuffer)                 \
    M(BindStaticDataBuffer)              \
    M(BindAppendDataBuffer)              \
    M(BindIndirectBuffer)                \
    M(BindIndexBuffer)                   \
    M(BindTexturesAndSamplers)           \
    M(SetScissor)                        \
    M(Draw)                              \
    M(DrawIndexed)                       \
    M(DrawInstanced)                     \
    M(DrawIndexedInstanced)              \
    M(DrawIndirect)                      \
    M(DrawIndexedIndirect)               \
    M(AddBarrier)

// Defines DrawPassCommands::Type, an enum of all draw command types.
#define ENUM(T) k##T,
enum class Type { SKGPU_DRAW_PASS_COMMAND_TYPES(ENUM) };
#undef ENUM

#define ACT_AS_PTR(ptr)                 \
    operator T*() const { return ptr; } \
    T* operator->() const { return ptr; }

// PODArray doesn't own the pointer's memory, and we assume the data is POD.
template <typename T>
class PODArray {
public:
    PODArray() {}
    PODArray(T* ptr) : fPtr(ptr) {}
    // Default copy and assign.

    ACT_AS_PTR(fPtr)
private:
    T* fPtr;
};

#undef ACT_AS_PTR

// A macro to make it a little easier to define a struct that can be stored in DrawPass.
#define COMMAND(T, ...)                    \
struct T {                                 \
static constexpr Type kType = Type::k##T;  \
    __VA_ARGS__;                           \
};

COMMAND(BindGraphicsPipeline,
            uint32_t fPipelineIndex);
COMMAND(SetBlendConstants,
            PODArray<float> fBlendConstants);
COMMAND(BindUniformBuffer,
            BindBufferInfo fInfo;
            UniformSlot fSlot);
COMMAND(BindStaticDataBuffer,
            BindBufferInfo fStaticData);
COMMAND(BindAppendDataBuffer,
            BindBufferInfo fAppendData)
COMMAND(BindIndexBuffer,
            BindBufferInfo fIndices);
COMMAND(BindIndirectBuffer,
            BindBufferInfo fIndirect);
COMMAND(BindTexturesAndSamplers,
            int fNumTexSamplers;
            PODArray<int> fTextureIndices;
            PODArray<int> fSamplerIndices);
COMMAND(SetScissor,
            Scissor fScissor);
COMMAND(Draw,
            PrimitiveType fType;
            uint32_t fBaseVertex;
            uint32_t fVertexCount);
COMMAND(DrawIndexed,
            PrimitiveType fType;
            uint32_t fBaseIndex;
            uint32_t fIndexCount;
            uint32_t fBaseVertex);
COMMAND(DrawInstanced,
            PrimitiveType fType;
            uint32_t fBaseVertex;
            uint32_t fVertexCount;
            uint32_t fBaseInstance;
            uint32_t fInstanceCount);
COMMAND(DrawIndexedInstanced,
            PrimitiveType fType;
            uint32_t fBaseIndex;
            uint32_t fIndexCount;
            uint32_t fBaseVertex;
            uint32_t fBaseInstance;
            uint32_t fInstanceCount);
COMMAND(DrawIndirect,
            PrimitiveType fType);
COMMAND(DrawIndexedIndirect,
            PrimitiveType fType);
COMMAND(AddBarrier,
            BarrierType fType);

#undef COMMAND

#define ASSERT_TRIV_DES(T) static_assert(std::is_trivially_destructible<T>::value);
SKGPU_DRAW_PASS_COMMAND_TYPES(ASSERT_TRIV_DES)
#undef ASSERT_TRIV_DES
#define ASSERT_TRIV_CPY(T) static_assert(std::is_trivially_copyable<T>::value);
SKGPU_DRAW_PASS_COMMAND_TYPES(ASSERT_TRIV_CPY)
#undef ASSERT_TRIV_CPY

class List {
public:
    List() = default;
    ~List() = default;

    int count() const { return fCommands.count(); }

    void bindGraphicsPipeline(uint32_t pipelineIndex) {
        this->add<BindGraphicsPipeline>(pipelineIndex);
    }

    void setBlendConstants(std::array<float, 4>  blendConstants) {
        this->add<SetBlendConstants>(this->copy(blendConstants.data(), 4));
    }

    void bindUniformBuffer(BindBufferInfo info, UniformSlot slot) {
        this->add<BindUniformBuffer>(info, slot);
    }

    // Caller must write 'numTexSamplers' texture and sampler indices into the two returned arrays.
    std::pair<int*, int*>
    bindDeferredTexturesAndSamplers(int numTexSamplers) {
        int* textureIndices = fAlloc.makeArrayDefault<int>(numTexSamplers);
        int* samplerIndices = fAlloc.makeArrayDefault<int>(numTexSamplers);
        this->add<BindTexturesAndSamplers>(numTexSamplers, textureIndices, samplerIndices);
        return {textureIndices, samplerIndices};
    }

    void setScissor(SkIRect scissor) {
        this->add<SetScissor>(Scissor(scissor));
    }

    void bindStaticDataBuffer(BindBufferInfo staticAttribs) {
        this->add<BindStaticDataBuffer>(staticAttribs);
    }

    void bindAppendDataBuffer(BindBufferInfo appendAttribs) {
        this->add<BindAppendDataBuffer>(appendAttribs);
    }

    void bindIndexBuffer(BindBufferInfo indices) {
        this->add<BindIndexBuffer>(indices);
    }

    void bindIndirectBuffer(BindBufferInfo indirect) {
        this->add<BindIndirectBuffer>(indirect);
    }

    void draw(PrimitiveType type, unsigned int baseVertex, unsigned int vertexCount) {
        this->add<Draw>(type, baseVertex, vertexCount);
    }

    void drawIndexed(PrimitiveType type, unsigned int baseIndex,
                     unsigned int indexCount, unsigned int baseVertex) {
        this->add<DrawIndexed>(type, baseIndex, indexCount, baseVertex);
    }

    void drawInstanced(PrimitiveType type,
                       unsigned int baseVertex, unsigned int vertexCount,
                       unsigned int baseInstance, unsigned int instanceCount) {
        this->add<DrawInstanced>(type, baseVertex, vertexCount, baseInstance, instanceCount);
    }

    void drawIndexedInstanced(PrimitiveType type,
                              unsigned int baseIndex, unsigned int indexCount,
                              unsigned int baseVertex, unsigned int baseInstance,
                              unsigned int instanceCount) {
        this->add<DrawIndexedInstanced>(type,
                                        baseIndex,
                                        indexCount,
                                        baseVertex,
                                        baseInstance,
                                        instanceCount);
    }

    void drawIndirect(PrimitiveType type) {
        this->add<DrawIndirect>(type);
    }

    void drawIndexedIndirect(PrimitiveType type) {
        this->add<DrawIndexedIndirect>(type);
    }

    void addBarrier(BarrierType type) {
        this->add<AddBarrier>(type);
    }

    using Command = std::pair<Type, void*>;
    using Iter = SkTBlockList<Command, 16>::CIter;
    Iter commands() const { return fCommands.items(); }

private:
    template <typename T, typename... Args>
    void add(Args&&... args) {
        T* cmd = fAlloc.make<T>(T{std::forward<Args>(args)...});
        fCommands.push_back(std::make_pair(T::kType, cmd));
    }

    // This copy() is for arrays.
    // It will work with POD only arrays.
    template <typename T>
    T* copy(const T src[], size_t count) {
        static_assert(std::is_trivially_copyable<T>::value);
        T* dst = fAlloc.makeArrayDefault<T>(count);
        memcpy(dst, src, count*sizeof(T));
        return dst;
    }

    SkTBlockList<Command, 16> fCommands{SkBlockAllocator::GrowthPolicy::kFibonacci};

    // fAlloc needs to be a data structure which can append variable length data in contiguous
    // chunks, returning a stable handle to that data for later retrieval.
    SkArenaAlloc fAlloc{256};
};

} // namespace DrawPassCommands

} // namespace skgpu::graphite

#endif // skgpu_graphite_DrawPassCommands_DEFINED
