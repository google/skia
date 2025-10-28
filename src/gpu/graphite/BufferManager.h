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
#include "src/core/SkTHash.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/UploadBufferManager.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <utility>

namespace skgpu::graphite {

class Caps;
class Context;
class DrawBufferManager;
class GlobalCache;
class QueueManager;
class Recording;
class ResourceProvider;

/**
 * BufferSubAllocator provides an entire GPU buffer to the caller so that the caller can sub
 * allocate intervals within the buffer. Each buffer type has a minimum required alignment for
 * binding. This alignment is automatically used for the *first* suballocation from an allocator
 * instance. Scoping the lifetime of an allocator to when the contents are bound allows these
 * binding requirements to automatically be met and use a tighter alignment for additional
 * suballocations that can be accessed without requiring a new binding.
 *
 * When a BufferSubAllocator goes out of scope, its underlying Buffer is returned to the manager. By
 * default, any remaining space can be returned by subsequent allocation requests but written bytes
 * will not be able to be overwritten by later BufferSubAllocators. The exception is with the
 * BufferSubAllocator instances returned by BufferManager::getScratchStorage(), whose Buffers will
 * be Shareable::kScratch resources, and can be fully reused by other Recorders or once the
 * BufferSubAllocator goes out of scope.
 *
 * Buffers created by the DrawBufferManager for an allocator are automatically transferred to the
 * Recording and CommandBuffers when snapped or inserted.
 */
class BufferSubAllocator final {
public:
    BufferSubAllocator() = default;

    // Disallow copy
    BufferSubAllocator(const BufferSubAllocator&) = delete;
    BufferSubAllocator& operator=(const BufferSubAllocator&) = delete;

    // Allow move
    BufferSubAllocator(BufferSubAllocator&& b) { *this = std::move(b); }
    BufferSubAllocator& operator=(BufferSubAllocator&&);

    ~BufferSubAllocator() { this->reset(); }

    // Returns false if the underlying buffer has been returned to the reuse pool or moved.
    bool isValid() const { return SkToBool(fBuffer); }
    explicit operator bool() const { return this->isValid(); }

    // Returns the underlying buffer object back to the pool and invalidates this allocator.
    // Depending on the GPU buffer's Shareable value, either:
    //  - kNo: The remaining space that hasn't been written to can be used by another allocator,
    //    but it will assume that use will involve a new buffer binding command.
    //  - kScratch: The entire buffer can be overwritten by another allocator.
    void reset();

    // Causes the next call to get[Mapped]Subrange() to also align with the minimum binding
    // requirement for the buffer's type. It resets any remaining strided blocks following the
    // last suballocation or appended range.
    void resetForNewBinding() {
        fRemaining = 0;
        fStride = 0; // signals prepForStride() to factor in minimum binding alignment next time.
    }

    // Returns the number of remaining bytes in the GPU buffer, assuming an alignment of 1.
    uint32_t remainingBytes() const {
        return fBuffer ? SkTo<uint32_t>(fBuffer->size()) - fOffset : 0;
    }

    /**
     * Suballocate `count*stride` bytes and a pointer (wrapped in a BufferWriter) to the mapped
     * range and the BindBufferInfo defining that range in a GPU-backed Buffer. The returned
     * subrange will be aligned according to the following rules:
     *  - The first suballocation, or the first after resetForNewBinding(), will be aligned to the
     *    lowest common multiple of `stride`, the binding's required alignment, and any extra base
     *    alignment passed in as `align`.
     *  - Subsequent suballocations will be aligned to just `stride` and any extra base alignment.
     *
     * It is assumed the caller will write all `count*stride` bytes to the returned address. If
     * `reservedCount` is greater than `count`, the suballocation will only succeed if the buffer
     * has room for an aligned `reservedCount*stride` bytes. The returned pointer can still only
     * write `count*stride` bytes, the remaining `reservedCount-count` is available for future
     * suballocations guaranteed to then fit within the same buffer (assuming the same or lower
     * alignment).
     *
     * It is acceptable to pass `count=0` to this function, which will still succeed if there is
     * space to align at least one more aligned `stride` block. In this case, the BufferWriter will
     * be null but the BindBufferInfo will have a valid Buffer and byte offset (just a zero size).
     * This then represents the start of the binding range for future calls to appendMappedStride().
     *
     * An invalid BufferWriter and empty BindBufferInfo are returned if the buffer does not have
     * enough room remaining to fulfill the suballocation in this buffer.
     */
     std::pair<BufferWriter, BindBufferInfo> getMappedSubrange(size_t count,
                                                               size_t stride,
                                                               size_t align=1) {
        SkASSERT(fMappedPtr || !fBuffer); // Writing should have checked validity of allocator first
        this->prepForStride(stride, align, count);
        return this->reserve(count, &BufferSubAllocator::getWriterAndBinding);
    }

    // Sub-allocate a slice within the scratch buffer object. This variation should be used when the
    // returned range will be written to by the GPU as part of executing a command buffer.
    //
    // Other than returning just a buffer slice to be written to later by a GPU task, the
    // suballocation behaves identically to getMappedSubrange().
    BindBufferInfo getSubrange(size_t count, size_t stride, size_t align=1) {
        SkASSERT(!fMappedPtr); // Should not be used when data is intended to be written by CPU
        this->prepForStride(stride, align, count);
        return this->reserve(count, &BufferSubAllocator::binding);
    }

    // Returns the number of `stride` blocks left in the buffer, where `stride` was the last
    // value passed into get[Mapped]Subrange().
    uint32_t availableWithStride() const { return fRemaining; }

    /**
     * Returns a buffer writer for `count*stride` bytes, where `stride` was the last value passed
     * to getMappedSubrange.
     *
     * If `count <= this->availableWithStride()`, then the appended range will be contiguous with
     * the BindBufferInfo returned from the last call to getMappedSubrange (and/or other calls to
     * appendMappedStride). The binding's size should be increased by count*stride, which is the
     * caller's responsibility.
     *
     * Otherwise, a null BufferWriter will be returned and no change is made to the allocator.
     */
    BufferWriter appendMappedWithStride(size_t count) {
        SkASSERT(count > 0 && (fMappedPtr || fRemaining == 0));
        return this->reserve(count, &BufferSubAllocator::getWriter);
    }

private:
    friend class DrawBufferManager;

    BufferSubAllocator(DrawBufferManager* owner,
                       int stateIndex,
                       sk_sp<Buffer> buffer,
                       BindBufferInfo transferBuffer, // optional (when direct mapping unavailable)
                       void* mappedPtr, // `buffer` or `transferBuffer`'s ptr, or null if GPU-only
                       size_t stride);  // initial stride to compute `fRemaining`

    // If minCount*stride bytes fit when aligned to the LCM of stride, align, and possibly the
    // binding alignment (when fStride == 0), then fOffset is updated and fRemaining is set to the
    // number of stride units that fit in the rest of the buffer after fOffset. If not, fRemaining
    // is set to 0 and fOffset is unmodified.
    void prepForStride(size_t stride, size_t align, size_t minCount);

    template <typename T>
    T reserve(size_t count, T (BufferSubAllocator::*create)(uint32_t offset, uint32_t size) const) {
        if (!fBuffer || count > fRemaining) {
            return T();
        }

        SkASSERT(fStride > 0); // fRemaining should be set to 0 if fStride is 0
        // fRemaining and fStride were already validated in prepForStride() so we can cast `count`
        const uint32_t requiredBytes32 = SkTo<uint32_t>(count) * fStride;

        uint32_t offset = fOffset;
        fOffset += requiredBytes32;
        fRemaining -= count;
        return (this->*create)(offset, requiredBytes32);
    }

    BindBufferInfo binding(uint32_t offset, uint32_t size) const {
        SkASSERT(fBuffer);
        SkASSERT(fOffset >= size && fOffset - size >= offset);
        return BindBufferInfo{fBuffer.get(), offset, size};
    }

    BufferWriter getWriter(uint32_t offset, uint32_t size) const {
        SkASSERT(fMappedPtr);
        SkASSERT(fBuffer);
        SkASSERT(fOffset >= size && fOffset - size >= offset);
        return BufferWriter(SkTAddOffset<void>(fMappedPtr, offset), size);
    }

    std::pair<BufferWriter, BindBufferInfo> getWriterAndBinding(
                uint32_t offset, uint32_t size) const {
        return {this->getWriter(offset, size), this->binding(offset, size)};
    }

    // Non-null when valid and not already returned to the pool
    DrawBufferManager* fOwner = nullptr;
    int fStateIndex = 0;

    sk_sp<Buffer> fBuffer;
    BindBufferInfo fTransferBuffer;

     // If mapped for writing, this is the CPU address of offset 0 of the buffer. When a mapped
     // buffer is returned to the DrawBufferManager, only the bytes after fOffset can be reused.
     // If there is no mapped buffer pointer, it's assumed the GPU buffer is reusable for another
     // BufferSubAllocator instance (this default reuse policy can be revisited if needed).
    void* fMappedPtr = nullptr;
    uint32_t fOffset = 0;    // Next suballocation can start at fOffset at the earliest
    uint32_t fStride = 1;    // The byte count of blocks for the optimized append() function
    uint32_t fRemaining = 0; // The number of `fStride` contiguous blocks fitting after fOffset
};

/**
 * DrawBufferManager controls writing to buffer data ranges within larger, cacheable Buffers and
 * automatically handles either mapping or copying via transfer buffer depending on what the GPU
 * hardware supports for the requested buffer type and use case. It is intended for repeatedly
 * uploading dynamic data to the GPU.
*/
class DrawBufferManager {
public:
    struct Options {
        Options() = default;

        uint32_t fVertexBufferMinSize  = 16 << 10; // 16 KB;
        uint32_t fVertexBufferMaxSize  = 1 << 20;  // 1  MB
        uint32_t fIndexBufferSize      = 2 << 10;  // 2  KB
        uint32_t fStorageBufferMinSize = 2 << 10;  // 2  KB;
        uint32_t fStorageBufferMaxSize = 1 << 20;  // 1  MB;

#if defined(GPU_TEST_UTILS)
        bool     fUseExactBuffSizes    = false; // Disables automatic buffer growth
        bool     fAllowCopyingGpuOnly  = false; // Adds kCopySrc to GPU-only buffer usage
#endif
    };

    DrawBufferManager(ResourceProvider* resourceProvider, const Caps* caps,
                      UploadBufferManager* uploadManager,
                      Options dbmOpts);
    ~DrawBufferManager();

    // Let possible users check if the manager is already in a bad mapping state and skip any extra
    // work that will be wasted because the next Recording snap will fail.
    bool hasMappingFailed() const { return fMappingFailed; }

    // Return a BufferWriter to write to the count*dataStride bytes of the GPU buffer subrange
    // represented by the returned BindBufferInfo. The returned BufferSubAllocator represents the
    // entire GPU buffer that the mapped subrange belongs to; it can be used to get additional
    // mapped suballocations, which when successful are guaranteed to be in the same buffer. This
    // allows callers to more easily manage when buffers must be bound.
    //
    // The returned {BufferWriter, BindBufferInfo} are effectively an automatic call to
    // BufferSubAllocator.getMappedSubrange(count, stride, reservedCount). The offset of this first
    // allocation will be aligned to the LCM of `stride` and the minimum required alignment for the
    // buffer type. For function variants that take an extra `alignment`, the initial suballocation
    // will also be aligned to that, equivalent to if resetForNewBinding(alignment) had been called
    // before. Subsequent suballocations from the returned allocator will only be aligned to their
    // requested stride unless resetForNewBinding() was called.
    //
    // When the returned BufferSubAllocator goes out of scope, any remaining bytes that were never
    // returned from either this function or later calls to getMappedSubrange() can be used to
    // satisfy a future call to getMapped[X]Buffer.
    using MappedAllocationInfo = std::tuple<BufferWriter, BindBufferInfo, BufferSubAllocator>;

    MappedAllocationInfo getMappedVertexBuffer(size_t count, size_t stride,
                                               size_t reservedCount=0, size_t alignment=1) {
        return this->getMappedBuffer(kVertexBufferIndex, count, stride, reservedCount, alignment);
    }
    MappedAllocationInfo getMappedIndexBuffer(size_t count) {
        return this->getMappedBuffer(kIndexBufferIndex, count, sizeof(uint16_t));
    }
    MappedAllocationInfo getMappedUniformBuffer(size_t count, size_t stride) {
        return this->getMappedBuffer(kUniformBufferIndex, count, stride);
    }
    MappedAllocationInfo getMappedStorageBuffer(size_t count, size_t stride) {
        return this->getMappedBuffer(kStorageBufferIndex, count, stride);
    }

    // The remaining writers and buffer allocator functions assume that byte counts are safely
    // calculated by the caller (e.g. Vello).

    // Utilities that return an unmapped buffer suballocation for a particular usage. These buffers
    // are intended to be only accessed by the GPU and are not intended for CPU data uploads.
    BindBufferInfo getStorage(size_t requiredBytes, ClearBuffer cleared = ClearBuffer::kNo) {
        return this->getBinding(kGpuOnlyStorageBufferIndex, requiredBytes, cleared);
    }
    BindBufferInfo getVertexStorage(size_t requiredBytes) {
        return this->getBinding(kVertexStorageBufferIndex, requiredBytes, ClearBuffer::kNo);
    }
    BindBufferInfo getIndexStorage(size_t requiredBytes) {
        return this->getBinding(kIndexStorageBufferIndex, requiredBytes, ClearBuffer::kNo);
    }
    BindBufferInfo getIndirectStorage(size_t requiredBytes, ClearBuffer cleared=ClearBuffer::kNo) {
        return this->getBinding(kIndirectStorageBufferIndex, requiredBytes, cleared);
    }

    // Returns an entire storage buffer object that is large enough to fit `requiredBytes`. The
    // returned BufferSubAllocator can be used to sub-allocate one or more storage buffer bindings
    // that reference the same buffer object.
    //
    // When the BufferSubAllocator goes out of scope, the buffer object gets added to an internal
    // pool and is available for immediate reuse. getScratchStorage() returns buffers from this pool
    // if possible. A BufferSubAllocator can be explicitly returned to the pool by calling
    // `returnToPool()`.
    //
    // Returning a BufferSubAllocator back to the buffer too early can result in validation failures
    // and/or data races. It is the callers responsibility to manage reuse within a Recording and
    // guarantee synchronized access to buffer bindings.
    //
    // This type of usage is currently limited to GPU-only storage buffers.
    BufferSubAllocator getScratchStorage(size_t requiredBytes) {
        return this->getBuffer(kGpuOnlyStorageBufferIndex, requiredBytes,
                               /*stride=*/1, /*xtraAlignment=*/1,
                               ClearBuffer::kNo, Shareable::kScratch);
    }

    // Finalizes all buffers and transfers ownership of them to a Recording. Returns true on success
    // and false if a mapping had previously failed.
    //
    // Regardless of success or failure, the DrawBufferManager is reset to a valid initial state
    // for recording buffer data for the next Recording.
    [[nodiscard]] bool transferToRecording(Recording*);

private:
    friend class BufferSubAllocator;

    struct BufferState {
        const BufferType    fType;
        const AccessPattern fAccessPattern;
        const bool          fUseTransferBuffer;
        const char*         fLabel;

        const uint32_t fMinAlignment; // guaranteed power of two, required for binding
        const uint32_t fMinBlockSize;
        const uint32_t fMaxBlockSize;

        BufferSubAllocator fAvailableBuffer;

        // Buffers held in this array are owned by still-alive BufferSubAllocators that were created
        // with Shareable::kScratch. This is compatible with ResourceCache::ScratchResourceSet.
        skia_private::THashSet<const Resource*> fUnavailableScratchBuffers;

        // The size of the last allocated Buffer, pinned to min/max block size, for amortizing the
        // number of buffer allocations for large Recordings.
        uint32_t fLastBufferSize = 0;

        BufferState(BufferType, const char* label, bool isGpuOnly,
                    const Options&, const Caps* caps);

        sk_sp<Buffer> findOrCreateBuffer(ResourceProvider*, Shareable, uint32_t byteCount);
    };

    // The returned sub allocator will have an offset that is aligned to `stride`, `xtraAlignment`
    // and the minimum alignment for `stateIndex`. Its `availableWithStride()` will be >= `count`.
    BufferSubAllocator getBuffer(int stateIndex,
                                 size_t count,
                                 size_t stride,
                                 size_t xtraAlignment,
                                 ClearBuffer cleared,
                                 Shareable shareable);

    MappedAllocationInfo getMappedBuffer(int stateIndex, size_t count, size_t stride,
                                         size_t reservedCount=0, size_t xtraAlignment=1) {
        BufferSubAllocator buffer = this->getBuffer(stateIndex,
                                                    std::max(count, reservedCount),
                                                    stride,
                                                    xtraAlignment,
                                                    ClearBuffer::kNo,
                                                    Shareable::kNo);
        auto [writer, binding] = buffer.getMappedSubrange(count, stride);
        return {std::move(writer), binding, std::move(buffer)};
    }

    // Helper method for the public GPU-only BufferBindInfo methods
    BindBufferInfo getBinding(int stateIndex, size_t requiredBytes, ClearBuffer cleared) {
        auto alloc = this->getBuffer(stateIndex, requiredBytes,
                                     /*stride=*/1, /*xtraAlignment=*/1,
                                     cleared, Shareable::kNo);
        // `alloc` goes out of scope when this returns, but that is okay because it is only used
        // for GPU-only, non-shareable buffers. The returned BindBufferInfo will be unique still.
        return alloc.getSubrange(requiredBytes, /*stride=*/1);
    }

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
    std::array<BufferState, 8> fCurrentBuffers;

    // Vector of buffer and transfer buffer pairs.
    skia_private::TArray<std::pair<sk_sp<Buffer>, BindBufferInfo>> fUsedBuffers;

    // List of buffer regions that were requested to be cleared at the time of allocation.
    skia_private::TArray<BindBufferInfo> fClearList;

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

    // For the vertex writer, the count and stride of the buffer is passed to allow alignment of
    // future vertices.
    VertexWriter getVertexWriter(size_t count, size_t stride, BindBufferInfo* binding);
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
        BindBufferInfo  fSource;            // The CPU-to-GPU buffer and offset for the source of the copy
        BindBufferInfo* fTarget;            // The late-assigned destination of the copy
        uint32_t        fRequiredAlignment; // The requested stride of the data.
#if defined(GPU_TEST_UTILS)
        uint32_t        fUnalignedSize;     // The requested size without count-4 alignment
#endif
    };
    struct BufferState {
        BufferState(BufferType type, const Caps* caps);

        bool createAndUpdateBindings(ResourceProvider*, Context*, QueueManager*, GlobalCache*,
                                     std::string_view label) const;
        void reset() {
            fData.clear();
            fTotalRequiredBytes = 0;
        }

        const BufferType fBufferType;
        // This is the lcm of the alignment requirement of the buffer type and the transfer buffer
        // alignment requirement.
        const uint32_t fMinimumAlignment;

        skia_private::TArray<CopyRange> fData;
        uint32_t fTotalRequiredBytes;
    };

    void* prepareStaticData(BufferState* info,
                            size_t requiredBytes,
                            size_t requiredAlignment,
                            BindBufferInfo* target);

    ResourceProvider* const fResourceProvider;
    UploadBufferManager fUploadManager;
    const uint32_t fRequiredTransferAlignment;

    // The source data that's copied into a final GPU-private buffer
    BufferState fVertexBufferState;
    BufferState fIndexBufferState;

    // If mapping failed on Buffers created/managed by this StaticBufferManager or by the mapped
    // transfer buffers from the UploadManager, remember so that finalize() will fail.
    bool fMappingFailed = false;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_BufferManager_DEFINED
