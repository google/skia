/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_BufferManager_DEFINED
#define skgpu_graphite_BufferManager_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/UploadBufferManager.h"

#include <array>
#include <tuple>
#include <vector>

namespace skgpu::graphite {

class Buffer;
class Caps;
class Context;
class GlobalCache;
class QueueManager;
class Recording;
class ResourceProvider;

/**
 * DrawBufferManager controls writing to buffer data ranges within larger, cacheable Buffers and
 * automatically handles either mapping or copying via transfer buffer depending on what the GPU
 * hardware supports for the requested buffer type and use case. It is intended for repeatedly
 * uploading dynamic data to the GPU.
*/
class DrawBufferManager {
public:
    DrawBufferManager(ResourceProvider*, const Caps*, UploadBufferManager*);
    ~DrawBufferManager();

    std::tuple<VertexWriter, BindBufferInfo> getVertexWriter(size_t requiredBytes);
    std::tuple<IndexWriter, BindBufferInfo> getIndexWriter(size_t requiredBytes);
    std::tuple<UniformWriter, BindBufferInfo> getUniformWriter(size_t requiredBytes);
    std::tuple<UniformWriter, BindBufferInfo> getSsboWriter(size_t requiredBytes);

    // Return a pointer to a mapped storage buffer suballocation without a specific data writer.
    std::tuple<void*, BindBufferInfo> getUniformPointer(size_t requiredBytes);
    std::tuple<void*, BindBufferInfo> getStoragePointer(size_t requiredBytes);

    // Utilities that return an unmapped buffer suballocation for a particular usage. These buffers
    // are intended to be only accessed by the GPU and are not intended for CPU data uploads.
    BindBufferInfo getStorage(size_t requiredBytes, ClearBuffer cleared = ClearBuffer::kNo);
    BindBufferInfo getVertexStorage(size_t requiredBytes);
    BindBufferInfo getIndexStorage(size_t requiredBytes);
    BindBufferInfo getIndirectStorage(size_t requiredBytes, ClearBuffer cleared = ClearBuffer::kNo);

    // Returns the last 'unusedBytes' from the last call to getVertexWriter(). Assumes that
    // 'unusedBytes' is less than the 'requiredBytes' to the original allocation.
    void returnVertexBytes(size_t unusedBytes);

    size_t alignUniformBlockSize(size_t dataSize) {
        return SkAlignTo(dataSize, fCurrentBuffers[kUniformBufferIndex].fStartAlignment);
    }

    // Finalizes all buffers and transfers ownership of them to a Recording.
    void transferToRecording(Recording*);

private:
    struct BufferInfo {
        BufferInfo(BufferType type, size_t blockSize, const Caps* caps);

        const BufferType fType;
        const size_t fStartAlignment;
        const size_t fBlockSize;
        sk_sp<Buffer> fBuffer;
        // The fTransferBuffer can be null, if draw buffer cannot be mapped,
        // see Caps::drawBufferCanBeMapped() for detail.
        BindBufferInfo fTransferBuffer{};
        void* fTransferMapPtr = nullptr;
        size_t fOffset = 0;
    };
    std::pair<void*, BindBufferInfo> prepareMappedBindBuffer(BufferInfo* info,
                                                             size_t requiredBytes);
    BindBufferInfo prepareBindBuffer(BufferInfo* info,
                                     size_t requiredBytes,
                                     bool supportCpuUpload = false,
                                     ClearBuffer cleared = ClearBuffer::kNo);

    ResourceProvider* const fResourceProvider;
    const Caps* const fCaps;
    UploadBufferManager* fUploadManager;

    static constexpr size_t kVertexBufferIndex          = 0;
    static constexpr size_t kIndexBufferIndex           = 1;
    static constexpr size_t kUniformBufferIndex         = 2;
    static constexpr size_t kStorageBufferIndex         = 3;
    static constexpr size_t kGpuOnlyStorageBufferIndex  = 4;
    static constexpr size_t kVertexStorageBufferIndex   = 5;
    static constexpr size_t kIndexStorageBufferIndex    = 6;
    static constexpr size_t kIndirectStorageBufferIndex = 7;
    std::array<BufferInfo, 8> fCurrentBuffers;

    // Vector of buffer and transfer buffer pairs.
    std::vector<std::pair<sk_sp<Buffer>, BindBufferInfo>> fUsedBuffers;

    // List of buffer regions that were requested to be cleared at the time of allocation.
    skia_private::TArray<ClearBufferInfo> fClearList;
};

/**
 * The StaticBufferManager is the one-time-only analog to DrawBufferManager and provides "static"
 * Buffers to RenderSteps and other Context-lifetime-tied objects, where the Buffers' contents will
 * not change and can benefit from prioritizing GPU reads. The assumed use case is that they remain
 * read-only on the GPU as well, so a single static buffer can be shared by all Recorders.
 *
 * Unlike DrawBufferManager's getXWriter() functions that return both a Writer and a BindBufferInfo,
 * StaticBufferManager returns only a Writer and accepts a BindBufferInfo* as an argument. This will
 * be re-written with the final binding info for the GPU-private data once that can be determined
 * after *all* static buffers have been requested.
 */
class StaticBufferManager {
public:
    StaticBufferManager(ResourceProvider*, const Caps*);
    ~StaticBufferManager();

    // The passed in BindBufferInfos are updated when finalize() is later called, to point to the
    // packed, GPU-private buffer at the appropriate offset. The data written to the returned Writer
    // is copied to the private buffer at that offset. 'binding' must live until finalize() returns.
    VertexWriter getVertexWriter(size_t size, BindBufferInfo* binding);
    // TODO: Update the tessellation index buffer generation functions to use an IndexWriter so this
    // can return an IndexWriter vs. a VertexWriter that happens to just write uint16s...
    VertexWriter getIndexWriter(size_t size, BindBufferInfo* binding);

    enum class FinishResult : int {
        kFailure, // Unable to create or copy static buffers
        kSuccess, // Successfully created static buffers and added GPU tasks to the queue
        kNoWork   // No static buffers required, no GPU tasks add to the queue
    };

    // Finalizes all buffers and records a copy task to compact and privatize static data. The
    // final static buffers will become owned by the Context's GlobalCache.
    FinishResult finalize(Context*, QueueManager*, GlobalCache*);

private:
    struct CopyRange {
        BindBufferInfo  fSource; // The CPU-to-GPU buffer and offset for the source of the copy
        BindBufferInfo* fTarget; // The late-assigned destination of the copy
        size_t          fSize;   // The number of bytes to copy
    };
    struct BufferInfo {
        BufferInfo(BufferType type, const Caps* caps);

        bool createAndUpdateBindings(ResourceProvider*,
                                     Context*,
                                     QueueManager*,
                                     GlobalCache*) const;
        void reset() {
            fData.clear();
            fTotalRequiredBytes = 0;
        }

        const BufferType fBufferType;
        const size_t     fAlignment;

        std::vector<CopyRange> fData;
        size_t fTotalRequiredBytes;
    };

    void* prepareStaticData(BufferInfo* info, size_t requiredBytes, BindBufferInfo* target);

    ResourceProvider* const fResourceProvider;
    UploadBufferManager fUploadManager;

    // The source data that's copied into a final GPU-private buffer
    BufferInfo fVertexBufferInfo;
    BufferInfo fIndexBufferInfo;

    const size_t fRequiredTransferAlignment;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_BufferManager_DEFINED
