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
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/UploadBufferManager.h"

#include <array>
#include <tuple>

namespace skgpu::graphite {

class Caps;
class Context;
class DrawBufferManager;
class GlobalCache;
class QueueManager;
class Recording;
class ResourceProvider;

/**
 * ScratchBuffer represents a GPU buffer object that is allowed to be reused across strictly
 * sequential tasks within a Recording. It can be used to sub-allocate multiple bindings.
 * When a ScratchBuffer gets deallocated, the underlying GPU buffer gets returned to the
 * originating DrawBufferManager for reuse.
 */
class ScratchBuffer final {
public:
    // The default constructor creates an invalid ScratchBuffer that cannot be used for
    // suballocations.
    ScratchBuffer() = default;

    // The destructor returns the underlying buffer back to the reuse pool, if the ScratchBuffer is
    // valid.
    ~ScratchBuffer();

    // Disallow copy
    ScratchBuffer(const ScratchBuffer&) = delete;
    ScratchBuffer& operator=(const ScratchBuffer&) = delete;

    // Allow move
    ScratchBuffer(ScratchBuffer&&) = default;
    ScratchBuffer& operator=(ScratchBuffer&&) = default;

    // Returns false if the underlying buffer has been returned to the reuse pool.
    bool isValid() const { return static_cast<bool>(fBuffer); }

    // Convenience wrapper for checking the validity of a buffer.
    explicit operator bool() { return this->isValid(); }

    // Logical size of the initially requested allocation.
    //
    // NOTE: This number may be different from the size of the underlying GPU buffer but it is
    // guaranteed to be less than or equal to it.
    size_t size() const { return fSize; }

    // Sub-allocate a slice within the scratch buffer object. Fails and returns a NULL pointer if
    // the buffer doesn't have enough space remaining for `requiredBytes`.
    // TODO(b/330743233): Currently the suballocations use the alignment for the BufferInfo that was
    // assigned by the DrawBufferManager based on the ScratchBuffer's buffer type. One way to
    // generalize this across different buffer usages/types is to have this function accept an
    // additional alignment parameter. That should happen after we loosen the coupling between
    // DrawBufferManager's BufferInfos and ScratchBuffer reuse pools.
    BindBufferInfo suballocate(size_t requiredBytes);

    // Returns the underlying buffer object back to the pool and invalidates this ScratchBuffer.
    void returnToPool();

private:
    friend class DrawBufferManager;

    ScratchBuffer(size_t size, size_t alignment, sk_sp<Buffer>, DrawBufferManager*);

    size_t fSize;
    size_t fAlignment;
    sk_sp<Buffer> fBuffer;
    size_t fOffset = 0;

    DrawBufferManager* fOwner = nullptr;
};

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

    // Let possible users check if the manager is already in a bad mapping state and skip any extra
    // work that will be wasted because the next Recording snap will fail.
    bool hasMappingFailed() const { return fMappingFailed; }

    std::pair<VertexWriter, BindBufferInfo> getVertexWriter(size_t requiredBytes);
    std::pair<IndexWriter, BindBufferInfo> getIndexWriter(size_t requiredBytes);
    std::pair<UniformWriter, BindBufferInfo> getUniformWriter(size_t requiredBytes);
    std::pair<UniformWriter, BindBufferInfo> getSsboWriter(size_t requiredBytes);

    // Return a pointer to a mapped storage buffer suballocation without a specific data writer.
    std::pair<void* /* mappedPtr */, BindBufferInfo> getUniformPointer(size_t requiredBytes);
    std::pair<void* /* mappedPtr */, BindBufferInfo> getStoragePointer(size_t requiredBytes);

    // Utilities that return an unmapped buffer suballocation for a particular usage. These buffers
    // are intended to be only accessed by the GPU and are not intended for CPU data uploads.
    BindBufferInfo getStorage(size_t requiredBytes, ClearBuffer cleared = ClearBuffer::kNo);
    BindBufferInfo getVertexStorage(size_t requiredBytes);
    BindBufferInfo getIndexStorage(size_t requiredBytes);
    BindBufferInfo getIndirectStorage(size_t requiredBytes, ClearBuffer cleared = ClearBuffer::kNo);

    // Returns an entire storage buffer object that is large enough to fit `requiredBytes`. The
    // returned ScratchBuffer can be used to sub-allocate one or more storage buffer bindings that
    // reference the same buffer object.
    //
    // When the ScratchBuffer goes out of scope, the buffer object gets added to an internal pool
    // and is available for immediate reuse. getScratchStorage() returns buffers from this pool if
    // possible. A ScratchBuffer can be explicitly returned to the pool by calling `returnToPool()`.
    //
    // Returning a ScratchBuffer back to the buffer too early can result in validation failures
    // and/or data races. It is the callers responsibility to manage reuse within a Recording and
    // guarantee synchronized access to buffer bindings.
    //
    // This type of usage is currently limited to GPU-only storage buffers.
    //
    // TODO(b/330743233): Generalize the underlying pool to other buffer types, including mapped
    //                    ones.
    ScratchBuffer getScratchStorage(size_t requiredBytes);

    // Returns the last 'unusedBytes' from the last call to getVertexWriter(). Assumes that
    // 'unusedBytes' is less than the 'requiredBytes' to the original allocation.
    void returnVertexBytes(size_t unusedBytes);

    size_t alignUniformBlockSize(size_t dataSize) {
        return SkAlignTo(dataSize, fCurrentBuffers[kUniformBufferIndex].fStartAlignment);
    }

    // Finalizes all buffers and transfers ownership of them to a Recording. Should not call if
    // hasMappingFailed() returns true.
    void transferToRecording(Recording*);

private:
    friend class ScratchBuffer;

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
    std::pair<void* /*mappedPtr*/, BindBufferInfo> prepareMappedBindBuffer(BufferInfo* info,
                                                                           size_t requiredBytes,
                                                                           std::string_view label);
    BindBufferInfo prepareBindBuffer(BufferInfo* info,
                                     size_t requiredBytes,
                                     std::string_view label,
                                     bool supportCpuUpload = false,
                                     ClearBuffer cleared = ClearBuffer::kNo);

    sk_sp<Buffer> findReusableSbo(size_t bufferSize);

    // Marks manager in a failed state, unmaps any previously collected buffers.
    void onFailedBuffer();

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
    skia_private::TArray<std::pair<sk_sp<Buffer>, BindBufferInfo>> fUsedBuffers;

    // List of buffer regions that were requested to be cleared at the time of allocation.
    skia_private::TArray<ClearBufferInfo> fClearList;

    // TODO(b/330744081): These should probably be maintained in a sorted data structure that
    // supports fast insertion and lookup doesn't waste buffers (e.g. by vending out large buffers
    // for small buffer sizes).
    // TODO(b/330743233): We may want this pool to contain buffers with mixed usages (such as
    // VERTEX|INDEX|UNIFORM|STORAGE) to reduce buffer usage on platforms like Dawn where
    // host-written data always go through a copy via transfer buffer.
    skia_private::TArray<sk_sp<Buffer>> fReusableScratchStorageBuffers;

    // If mapping failed on Buffers created/managed by this DrawBufferManager or by the mapped
    // transfer buffers from the UploadManager, remember so that the next Recording will fail.
    bool fMappingFailed = false;
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
                                     GlobalCache*,
                                     std::string_view label) const;
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
    const size_t fRequiredTransferAlignment;

    // The source data that's copied into a final GPU-private buffer
    BufferInfo fVertexBufferInfo;
    BufferInfo fIndexBufferInfo;

    // If mapping failed on Buffers created/managed by this StaticBufferManager or by the mapped
    // transfer buffers from the UploadManager, remember so that finalize() will fail.
    bool fMappingFailed = false;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_BufferManager_DEFINED
