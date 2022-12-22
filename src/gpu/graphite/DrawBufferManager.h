/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DrawBufferManager_DEFINED
#define skgpu_graphite_DrawBufferManager_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/ResourceTypes.h"

#include <array>
#include <unordered_map>
#include <vector>

namespace skgpu::graphite {

class Buffer;
class Caps;
class Recording;
class ResourceProvider;

class DrawBufferManager {
public:
    DrawBufferManager(ResourceProvider*, const Caps*);
    ~DrawBufferManager();

    std::tuple<VertexWriter, BindBufferInfo> getVertexWriter(size_t requiredBytes);
    std::tuple<IndexWriter, BindBufferInfo> getIndexWriter(size_t requiredBytes);
    std::tuple<UniformWriter, BindBufferInfo> getUniformWriter(size_t requiredBytes);
    std::tuple<UniformWriter, BindBufferInfo> getSsboWriter(size_t requiredBytes);

    // Returns the last 'unusedBytes' from the last call to getVertexWriter(). Assumes that
    // 'unusedBytes' is less than the 'requiredBytes' to the original allocation.
    void returnVertexBytes(size_t unusedBytes);

    size_t alignUniformBlockSize(size_t dataSize) {
        return SkAlignTo(dataSize, fCurrentBuffers[kUniformBufferIndex].fStartAlignment);
    }

    // Get the shared static buffer filled with contents computed by the InitializeBufferFn.
    // Both InitializeBufferFn and BufferSizeFn should be static functions since their addresses are
    // used in the static buffer's internal unique keys.
    // TODO(skbug.com/13059): getStaticBuffer is only temporary until RenderSteps can create buffers
    // during initialization of the Context. Only use with RenderSteps that will be updated
    // to the new initialization pattern in the near future.
    using BufferSizeFn = size_t(*)();
    using InitializeBufferFn = void(*)(skgpu::VertexWriter, size_t bufferSize);
    BindBufferInfo getStaticBuffer(BufferType type,
                                   InitializeBufferFn,
                                   BufferSizeFn);

    // Finalizes all buffers and transfers ownership of them to a Recording.
    void transferToRecording(Recording*);

private:
    struct BufferInfo {
        BufferInfo(BufferType type, size_t blockSize, const Caps* caps);

        const BufferType fType;
        const size_t fStartAlignment;
        const size_t fBlockSize;
        sk_sp<Buffer> fBuffer{};
        // The fTransferBuffer can be null, if draw buffer cannot be mapped,
        // see Caps::drawBufferCanBeMapped() for detail.
        sk_sp<Buffer> fTransferBuffer{};
        size_t fOffset = 0;

        Buffer* getMappableBuffer() {
            return fTransferBuffer ? fTransferBuffer.get() : fBuffer.get();
        }
    };
    std::pair<void*, BindBufferInfo> prepareBindBuffer(BufferInfo* info, size_t requiredBytes);

    ResourceProvider* const fResourceProvider;
    const Caps* const fCaps;

    static constexpr size_t kVertexBufferIndex  = 0;
    static constexpr size_t kIndexBufferIndex   = 1;
    static constexpr size_t kUniformBufferIndex = 2;
    static constexpr size_t kStorageBufferIndex = 3;
    std::array<BufferInfo, 4> fCurrentBuffers;

    // Vector of buffer and transfer buffer pairs.
    std::vector<std::pair<sk_sp<Buffer>, sk_sp<Buffer>>> fUsedBuffers;
    // TODO(skbug.com/13059): This is likely not the final location for static buffers, but makes it
    // convenient to maintain ownership and call trackResources() on the CommandBuffer.
    std::unordered_map<uintptr_t, sk_sp<Buffer>> fStaticBuffers;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DrawBufferManager_DEFINED
