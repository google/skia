/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 #include "src/gpu/graphite/BufferManager.h"

#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/Recording.h"
#include "include/private/base/SkAlign.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkMath.h"
#include "include/private/base/SkTo.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/QueueManager.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/UploadBufferManager.h"
#include "src/gpu/graphite/task/ClearBuffersTask.h"
#include "src/gpu/graphite/task/CopyTask.h"
#include "src/gpu/graphite/task/Task.h"
#include "src/gpu/graphite/task/TaskList.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <limits>
#include <numeric>
#include <tuple>

namespace skgpu::graphite {

namespace {

// The limit for all data created by the StaticBufferManager. This data remains alive for
// the entire SharedContext so we want to keep it small and give a concrete upper bound to
// clients for our steady-state memory usage.
// FIXME The current usage is 4732 bytes across static vertex and index buffers, but that includes
// multiple copies of tessellation data, and an unoptimized AnalyticRRect mesh. Once those issues
// are addressed, we can tighten this and decide on the transfer buffer sizing as well.
[[maybe_unused]] static constexpr uint32_t kMaxStaticDataSize = 6 << 10;

uint32_t validate_count_and_stride(size_t count, size_t stride, uint32_t alignment) {
    // size_t may just be uint32_t, so this ensures we have enough bits to
    // compute the required byte product.
    uint64_t count64 = SkTo<uint64_t>(count);
    uint64_t stride64 = SkTo<uint64_t>(stride);
    uint64_t bytes64 = count64*stride64;
    if (count64 > std::numeric_limits<uint32_t>::max() ||
        stride64 > std::numeric_limits<uint32_t>::max() ||
        bytes64 > std::numeric_limits<uint32_t>::max() - (alignment + 1)) {
        // Return 0 to skip further allocation attempts.
        return 0;
    }
    // Since count64 and stride64 fit into 32-bits, their product won't overflow a 64-bit multiply,
    // and we've confirmed product fits into 32-bits with head room to be aligned w/o overflow.
    return SkTo<uint32_t>(bytes64);
}

// Calculates the LCM of `alignMaybePow2` and `alignProbNonPow2`. Neither value needs to be a
// power of 2, but this is optimized to check for whether or not `alignMaybePow2` is a power of 2.
// It assumes the probability of the 2nd alignment value being a power of 2 is low enough to not
// be worth checking.
uint32_t lcm_alignment(uint32_t alignMaybePow2, uint32_t alignProbNonPow2) {
    SkASSERT(alignMaybePow2 != 0 && alignProbNonPow2 != 0);
    if (alignMaybePow2 == 1 ||
        alignMaybePow2 == alignProbNonPow2 ||
        (SkIsPow2(alignMaybePow2) &&
                alignProbNonPow2 > alignMaybePow2 &&
                (alignProbNonPow2 & (alignMaybePow2 - 1)) == 0)) {
        // Trivial LCM since alignProbNonPow2 is the same or a larger multiple of alignMaybePow2
        return alignProbNonPow2;
    } else {
        return std::lcm(alignMaybePow2, alignProbNonPow2);
    }
}

// Helpers for creating a BufferState based on type, options, and caps

AccessPattern get_gpu_access_pattern(bool isGpuOnlyAccess, const DrawBufferManager::Options& opts) {
    if (isGpuOnlyAccess) {
#if defined(GPU_TEST_UTILS)
        if (opts.fAllowCopyingGpuOnly) {
            return AccessPattern::kGpuOnlyCopySrc;
        }
#endif
        return AccessPattern::kGpuOnly;
    } else {
        return AccessPattern::kHostVisible;
    }
}

// This returns the minimum required alignment depending on the type of buffer. This is guaranteed
// to be a power of two.
uint32_t minimum_alignment(BufferType type, bool useTransferBuffers, const Caps* caps) {
    uint32_t alignment = 4;
    if (type == BufferType::kUniform) {
        alignment = SkTo<uint32_t>(caps->requiredUniformBufferAlignment());
    } else if (type == BufferType::kStorage || type == BufferType::kVertexStorage ||
               type == BufferType::kIndexStorage || type == BufferType::kIndirect) {
        alignment = SkTo<uint32_t>(caps->requiredStorageBufferAlignment());
    }
    if (useTransferBuffers) {
        // Both alignment and the requiredTransferBufferAlignment must be powers of two, so max
        // provides the correct alignment semantics
        alignment = std::max(alignment, SkTo<uint32_t>(caps->requiredTransferBufferAlignment()));
    }
    return alignment;
}

uint32_t min_block_size(BufferType type,
                        uint32_t minAlignment,
                        const DrawBufferManager::Options& opts) {
    uint32_t size;
    if (type == BufferType::kIndex || type == BufferType::kIndexStorage) {
        size = opts.fIndexBufferSize;
    } else if (type == BufferType::kVertex || type == BufferType::kVertexStorage) {
        size = opts.fVertexBufferMinSize;
    } else {
        size = opts.fStorageBufferMinSize;
    }
#if defined(GPU_TEST_UTILS)
    if (opts.fUseExactBuffSizes) {
        return size; // No extra alignment
    }
#endif

    return SkAlignTo(size, minAlignment);
}

uint32_t max_block_size(BufferType type,
                        uint32_t minAlignment,
                        const DrawBufferManager::Options& opts) {
#if defined(GPU_TEST_UTILS)
    if (opts.fUseExactBuffSizes) {
        // Clamp to the minimum size
        return min_block_size(type, minAlignment, opts);
    }
#endif

    uint32_t size;
    if (type == BufferType::kIndex || type == BufferType::kIndexStorage) {
        size = opts.fIndexBufferSize;
    } else if (type == BufferType::kVertex || type == BufferType::kVertexStorage) {
        size = opts.fVertexBufferMaxSize;
    } else {
        size = opts.fStorageBufferMaxSize;
    }

    return SkAlignTo(size, minAlignment);
}

} // anonymous namespace

// ------------------------------------------------------------------------------------------------
// BufferSubAllocator

BufferSubAllocator::BufferSubAllocator(DrawBufferManager* owner,
                                       int stateIndex,
                                       sk_sp<Buffer> buffer,
                                       BindBufferInfo transferBuffer,
                                       void* mappedPtr,
                                       size_t stride)
        : fOwner(owner)
        , fStateIndex(stateIndex)
        , fBuffer(std::move(buffer))
        , fTransferBuffer(transferBuffer)
        , fMappedPtr(mappedPtr)
        , fOffset(0)
        , fStride(SkTo<uint32_t>(stride)) {
    // A starting offset of 0 means we don't need to explicitly lookup the minimum binding alignment
    // since the first sub-allocation is automatically aligned.
    if (fBuffer && fStride > 0 && fStride <= fBuffer->size()) {
        // This puts the BufferSubAllocator in the same state as prepForStride(stride, *)
        fRemaining = SkTo<uint32_t>(fBuffer->size()) / fStride;
    } else {
        fRemaining = 0;
    }

    SkASSERT(fStride != 0);
}

BufferSubAllocator& BufferSubAllocator::operator=(BufferSubAllocator&& other) {
    if (this == &other) {
        return *this; // no-op moving into itself
    }

    // Reset the destination allocator first since other's contents will overwrite whatever came
    // beforehand and that must go back to the manager.
    this->reset();

    // Copy fields
    fOwner = other.fOwner;
    fStateIndex = other.fStateIndex;
    fTransferBuffer = other.fTransferBuffer;
    fMappedPtr = other.fMappedPtr;
    fOffset = other.fOffset;
    fStride = other.fStride;
    fRemaining = other.fRemaining;

    // Move buffer (leaving other in an invalid state)
    fBuffer = std::move(other.fBuffer);
    SkASSERT(!other);
    return *this;
}

void BufferSubAllocator::prepForStride(size_t stride, size_t align, size_t minCount) {
    SkASSERT(stride > 0 && align > 0); // Expect valid inputs
    if (fBuffer) {
        if (fStride == stride && (align == 1 || align == stride)) {
            // Shortcut if we're already aligned with the last call to prepForStride().
            // Leave fRemaining alone, it's either enough for minCount or not, but reserve() will
            // do the right thing regardless.
            SkASSERT(fOffset % align == 0);
            SkASSERT(fOffset % stride == 0);
            return;
        }

        // On re-aligning to a new stride, the offset needs to be aligned to the LCM of `align` and
        // `stride` so that repeated suballocations of `stride` can be performed by simply adding to
        // fOffset without additional instructions. If `fStride == 0`, it's a signal that the first
        // offset also needs to be aligned to the minimum binding requirement.
        uint32_t align32 = lcm_alignment(SkTo<uint32_t>(align), SkTo<uint32_t>(stride));
        if (fStride == 0) {
            const uint32_t minAlignment = fOwner->fCurrentBuffers[fStateIndex].fMinAlignment;
            align32 = lcm_alignment(minAlignment, align32);
        }

        // Ensures we won't overflow fOffset past buffer size once we align it
        if (this->remainingBytes() >= align32 - 1) {
            const uint32_t offset = SkAlignNonPow2(fOffset, align32);
            SkASSERT(offset <= fBuffer->size());
            fStride = SkTo<uint32_t>(stride);
            fRemaining = (SkTo<uint32_t>(fBuffer->size()) - offset) / fStride;
            if (fRemaining > 0 && fRemaining >= minCount) {
                // Successful prep, so preserve the aligned offset
                fOffset = offset;
                return;
            }
        }
    }

    // If we've reached here, there wasn't a buffer or enough room to align, or enough room to
    // satisfy minCount, so set fRemaining=0 to fail subsequent reservations.
    fRemaining = 0;
}

void BufferSubAllocator::reset() {
    if (fBuffer) {
        SkASSERT(fOwner);

        DrawBufferManager::BufferState& state = fOwner->fCurrentBuffers[fStateIndex];
        if (fBuffer->shareable() == Shareable::kScratch) {
            // TODO: Merge this reuse of scratch resources with the ScratchResourceManager, but
            // currently this is resolved outside of Task::prepareResources().

            // The scratch buffer's availability for reuse (scoped to the owning DrawBufferManager)
            // was tied to this BufferSubAllocator, so when that is reset, we just remove the buffer
            // from the set of unavailable buffers.
            SkASSERT((fOwner->fMappingFailed && state.fUnavailableScratchBuffers.empty()) ||
                     state.fUnavailableScratchBuffers.contains(fBuffer.get()));
            if (!fOwner->fMappingFailed) {
                state.fUnavailableScratchBuffers.remove(fBuffer.get());
            }

            SkASSERT(!fTransferBuffer); // Scratch buffers shouldn't be using transfer buffers
            fOwner->fUsedBuffers.emplace_back(std::move(fBuffer), BindBufferInfo{});
        } else if (state.fAvailableBuffer.fBuffer.get() == fBuffer.get() || // can't stash itself
                   this->remainingBytes() < state.fAvailableBuffer.remainingBytes() || // too small
                   this->remainingBytes() < state.fMinAlignment) { // basically empty
            // Transfer ownership of the buffer (and any transfer buffer) back to the manager, using
            // the current offset as a more restricted limit for copying.
            if (fTransferBuffer) {
                // This alignment ensures we are copying a subset that still respects xfer alignment
                fTransferBuffer.fSize = SkAlignTo(fOffset, state.fMinAlignment);
            }
            fOwner->fUsedBuffers.emplace_back(std::move(fBuffer), fTransferBuffer);
        } else {
            // Save this buffer for later, which leaves this instance empty and resets the prior
            // value of fAvailableBuffer (which then goes through the true branch of this if).
            state.fAvailableBuffer = std::move(*this);
        }

        fRemaining = 0;
        SkASSERT(!fBuffer);
    } // else nothing to reset
}

// ------------------------------------------------------------------------------------------------
// DrawBufferManager::BufferState

DrawBufferManager::BufferState::BufferState(BufferType type,
                                            const char* label,
                                            bool isGpuOnly,
                                            const Options& opts,
                                            const Caps* caps)
        : fType(type)
        // The buffer can be GPU-only if
        //     a) the caller does not intend to ever upload CPU data to the buffer; or
        //     b) CPU data will get uploaded to fBuffer only via a transfer buffer
        , fAccessPattern(get_gpu_access_pattern(isGpuOnly || !caps->drawBufferCanBeMapped(), opts))
        , fUseTransferBuffer(!isGpuOnly && !caps->drawBufferCanBeMapped())
        , fLabel(label)
        , fMinAlignment(minimum_alignment(type, fUseTransferBuffer, caps))
        , fMinBlockSize(min_block_size(type, fMinAlignment, opts))
        , fMaxBlockSize(max_block_size(type, fMinAlignment, opts)) {
    SkASSERT(SkIsPow2(fMinAlignment));
    SkASSERT(fMinBlockSize <= fMaxBlockSize);
}

sk_sp<Buffer> DrawBufferManager::BufferState::findOrCreateBuffer(ResourceProvider* provider,
                                                                 Shareable shareable,
                                                                 uint32_t byteCount) {
    if (shareable == Shareable::kScratch) {
        sk_sp<Buffer> scratchBuffer = provider->findOrCreateScratchBuffer(
                byteCount, fType, fAccessPattern, fLabel, fUnavailableScratchBuffers);
        if (scratchBuffer) {
            fUnavailableScratchBuffers.add(scratchBuffer.get());
        }
        return scratchBuffer;
    } else {
        return provider->findOrCreateNonShareableBuffer(byteCount, fType, fAccessPattern, fLabel);
    }
}

// ------------------------------------------------------------------------------------------------
// DrawBufferManager

DrawBufferManager::DrawBufferManager(ResourceProvider* resourceProvider,
                                     const Caps* caps,
                                     UploadBufferManager* uploadManager,
                                     Options dbmOpts)
        : fResourceProvider(resourceProvider)
        , fCaps(caps)
        , fUploadManager(uploadManager)
        , fCurrentBuffers{{
            // Mappable buffers
            {BufferType::kVertex, "VertexBuffer", /*isGpuOnly=*/false, dbmOpts, caps},
            {BufferType::kIndex, "IndexBuffer", /*isGpuOnly=*/false, dbmOpts, caps},
            {BufferType::kUniform, "UniformBuffer", /*isGpuOnly=*/false, dbmOpts, caps},
            {BufferType::kStorage, "StorageBuffer", /*isGpuOnly=*/false, dbmOpts, caps},
            // GPU-only buffers
            {BufferType::kStorage, "GPUOnlyStorageBuffer", /*isGpuOnly=*/true, dbmOpts, caps},
            {BufferType::kVertexStorage, "VertexStorageBuffer", /*isGpuOnly=*/true, dbmOpts, caps},
            {BufferType::kIndexStorage, "IndexStorageBuffer", /*isGpuOnly=*/true, dbmOpts, caps},
            {BufferType::kIndirect, "IndirectStorageBuffer", /*isGpuOnly=*/true, dbmOpts, caps}}} {}

DrawBufferManager::~DrawBufferManager() {
    // Must reset these *before* we are deleted
    for (auto& b : fCurrentBuffers) {
        b.fAvailableBuffer.reset();
    }
}

void DrawBufferManager::onFailedBuffer() {
    fMappingFailed = true;

    // Clean up and unmap everything now
    fClearList.clear();
    for (auto& state : fCurrentBuffers) {
        state.fAvailableBuffer.reset();
         // We aren't allocating anything anymore so don't maintain this list. Their outstanding
         // BufferSubAllocators will have a no-op when they get reset.
        state.fUnavailableScratchBuffers.reset();
        state.fLastBufferSize = 0;
    }

    for (auto& [buffer, _] : fUsedBuffers) {
        if (buffer->isMapped()) {
            buffer->unmap();
        }
    }
    fUsedBuffers.clear();
}

bool DrawBufferManager::transferToRecording(Recording* recording) {
    if (fMappingFailed) {
        // All state should have been reset by onFailedBuffer() except for this error flag.
        SkASSERT(fUsedBuffers.empty() && fClearList.empty());
#if defined(SK_DEBUG)
        for (const auto& state : fCurrentBuffers) {
            SkASSERT(!SkToBool(state.fAvailableBuffer));
            SkASSERT(state.fUnavailableScratchBuffers.empty());
        }
#endif

        fMappingFailed = false;
        return false;
    }

    for (auto& state : fCurrentBuffers) {
        // Reset all available buffer sub allocators since they won't be allocatable anymore.
        // This pushes the underlying resource and transfer range to fUsedBuffers
        state.fAvailableBuffer.reset();
        // BufferSubAllocators should have gone out of scope well before Recorder::snap() is called.
        SkASSERT(state.fUnavailableScratchBuffers.empty());

        // We reset the last buffer size back to 0 to keep the buffer growth behavior the same
        // across calls to snap(). If we knew every snap() would be approximately the same workload,
        // we could choose to keep the last alloc size as-is so that subsequent frames create
        // fewer buffers. We choose *not* to do this because:
        //  - Chrome often snaps Recordings with disparate workloads within a frame (e.g. tile vs
        //    canvas2d) and we don't want to overallocate on a small recording.
        //  - It obfuscates the performance cost of the first frame if we reach a steady state that
        //    requires no additional buffer allocations.
        // We could choose to reduce fLastBufferSize (e.g. halve it) to get a head start and reduce
        // the potential for over-allocation, but in performance measurements on buffer-heavy scenes
        // this did not lead to measurable improvements. Thus, we reset so every frame is the same.
        state.fLastBufferSize = 0;
    }

    if (!fClearList.empty()) {
        recording->priv().taskList()->add(ClearBuffersTask::Make(std::move(fClearList)));
    }

    for (auto& [buffer, transferBuffer] : fUsedBuffers) {
        if (transferBuffer) {
            SkASSERT(buffer);
            SkASSERT(!fCaps->drawBufferCanBeMapped());
            // Since the transfer buffer is managed by the UploadManager, we don't manually unmap
            // it here or need to pass a ref into CopyBufferToBufferTask.
            size_t copySize = buffer->size();
            recording->priv().taskList()->add(
                    CopyBufferToBufferTask::Make(transferBuffer.fBuffer,
                                                 transferBuffer.fOffset,
                                                 std::move(buffer),
                                                 /*dstOffset=*/0,
                                                 copySize));
        } else {
            if (buffer->isMapped()) {
                buffer->unmap();
            }
            recording->priv().addResourceRef(std::move(buffer));
        }
    }

    fUsedBuffers.clear();

    return true;
}

BufferSubAllocator DrawBufferManager::getBuffer(
        int stateIndex,
        size_t count,
        size_t stride,
        size_t xtraAlignment,
        ClearBuffer cleared,
        Shareable shareable) {
    BufferState& state = fCurrentBuffers[stateIndex];
    // The size for a buffer is aligned to the minimum block size for better resource reuse, which
    // is more conservative than fMinAlignment.
    uint32_t requiredBytes32 = validate_count_and_stride(count, stride, state.fMinBlockSize);
    if (fMappingFailed || !requiredBytes32) {
        return {};
    }

    const bool supportCpuUpload = state.fAccessPattern == AccessPattern::kHostVisible ||
                                  state.fUseTransferBuffer;
    // Shareable buffers must be GPU-only to actually share effectively.
    SkASSERT(shareable == Shareable::kNo || !supportCpuUpload);

    // For non-shareable buffers, we keep the largest relinquished non-shareable buffer in case it
    // has room leftover to be used by future allocations. Scratch buffer ownership is entirely
    // managed by the caller, so always create a new BufferSubAllocator.
    if (shareable == Shareable::kNo) {
        state.fAvailableBuffer.resetForNewBinding(); // ensure we include min binding alignment
        state.fAvailableBuffer.prepForStride(stride, xtraAlignment, count);
        if (state.fAvailableBuffer.availableWithStride() >= count) {
            SkASSERT(state.fAvailableBuffer.fBuffer);
            SkASSERT(state.fAvailableBuffer.fBuffer->shareable() == shareable);
            SkASSERT(SkToBool(state.fAvailableBuffer.fMappedPtr) == supportCpuUpload);
            return std::move(state.fAvailableBuffer);
        }

        // Not enough room in the available buffer so release it and create a new buffer.
        state.fAvailableBuffer.reset();
    }

    // Create the next buffer by doubling the size of the previous buffer and clamping to be within
    // the min and max block sizes if `requiredBytes` is less than the max. Otherwise, create a
    // buffer large enough to satisfy `requiredBytes` but align it to minBlockSize.
    uint32_t bufferSize = SkAlignTo(requiredBytes32, state.fMinBlockSize);
    if (bufferSize < state.fMaxBlockSize) {
        // fMaxBlockSize should be sufficiently small that there's no risk of overflowing here.
        SkASSERT(std::numeric_limits<uint32_t>::max() /2 > state.fLastBufferSize);
        bufferSize = std::max(bufferSize, std::min(state.fLastBufferSize * 2, state.fMaxBlockSize));
        state.fLastBufferSize = bufferSize;
        SkASSERT(bufferSize <= state.fMaxBlockSize);
    } else {
        // Jump to the max block size for subsequent amortized allocations if we get a really big
        // buffer request.
        state.fLastBufferSize = state.fMaxBlockSize;
    }
    SkASSERT(bufferSize >= requiredBytes32 && bufferSize >= state.fMinBlockSize);

    sk_sp<Buffer> buffer = state.findOrCreateBuffer(fResourceProvider, shareable, bufferSize);
    if (!buffer) {
        this->onFailedBuffer();
        return {};
    }

    BindBufferInfo transferBuffer;
    void* mappedPtr = nullptr;
    if (supportCpuUpload) {
        if (state.fUseTransferBuffer) {
            std::tie(mappedPtr, transferBuffer) = fUploadManager->makeBindInfo(buffer->size(),
                    fCaps->requiredTransferBufferAlignment(), "TransferForDataBuffer");
        } else {
            mappedPtr = buffer->map();
        }

        if (!mappedPtr) {
            this->onFailedBuffer(); // Either transfer buffer failed or direct mapping failed
            return {};
        }
    }

    if (cleared == ClearBuffer::kYes) {
        fClearList.push_back(BindBufferInfo{buffer.get(), 0, bufferSize});
    }

    // The returned buffer is not set to fAvailableBuffer because it is going to be passed up to
    // the caller for their use first. Since a new BufferSubAllocator starts at offset 0, there's no
    // need to account for `xtraAlignment`.
    return BufferSubAllocator(this, stateIndex, std::move(buffer),
                              transferBuffer, mappedPtr, stride);
}

// ------------------------------------------------------------------------------------------------
// StaticBufferManager

StaticBufferManager::StaticBufferManager(ResourceProvider* resourceProvider,
                                         const Caps* caps)
        : fResourceProvider(resourceProvider)
        , fUploadManager(resourceProvider, caps)
        , fRequiredTransferAlignment(SkTo<uint32_t>(caps->requiredTransferBufferAlignment()))
        , fVertexBufferState(BufferType::kVertex, caps)
        , fIndexBufferState(BufferType::kIndex, caps) {}
StaticBufferManager::~StaticBufferManager() = default;

StaticBufferManager::BufferState::BufferState(BufferType type, const Caps* caps)
        : fBufferType(type)
        , fMinimumAlignment(minimum_alignment(type, /*useTransferBuffers=*/true, caps))
        , fTotalRequiredBytes(0) {}

// ARM hardware b/399631317 also means that static vertex data must be padded and zeroed out. So we
// always request a count 4 aligned offset, count 4 aligned amount of space, and zero it.
VertexWriter StaticBufferManager::getVertexWriter(size_t count,
                                                  size_t stride,
                                                  BindBufferInfo* binding) {
    const size_t size = count * stride;
    const size_t alignedCount = SkAlign4(count);
    void* data = this->prepareStaticData(&fVertexBufferState, size, stride * 4, binding);
    if (alignedCount > count) {
        const uint32_t byteDiff = (alignedCount - count) * stride;
        void* zPtr = SkTAddOffset<void>(data, count * stride);
        memset(zPtr, 0, byteDiff);
    }
    return VertexWriter{data, size};
}

VertexWriter StaticBufferManager::getIndexWriter(size_t size, BindBufferInfo* binding) {
    // The index writer does not have the same alignment requirements as a vertex, so we simply pass
    // in the minimum alignment as the required alignment
    void* data = this->prepareStaticData(&fIndexBufferState,
                                         size,
                                         fIndexBufferState.fMinimumAlignment,
                                         binding);
    return VertexWriter{data, size};
}

void* StaticBufferManager::prepareStaticData(BufferState* state,
                                             size_t requiredBytes,
                                             size_t requiredAlignment,
                                             BindBufferInfo* target) {
    // Zero-out the target binding in the event of any failure in actually transfering data later.
    // Unlike in BufferSubAllocator::reserve(), we do use SkTo<uint32_t> to check
    // `requiredAlignment`. This is not dynamic data and is fully controlled by Graphite, so if it
    // asserts, then there is a bug in the static data for a Renderer that must be fixed.
    const uint32_t align32 = lcm_alignment(state->fMinimumAlignment,
                                           SkTo<uint32_t>(requiredAlignment));

    SkASSERT(target);
    *target = {nullptr, 0};
    uint32_t size32 = validate_count_and_stride(requiredBytes, /*stride=*/1, align32);
    if (!size32 || fMappingFailed) {
        return nullptr;
    }

    // Copy data must be aligned to the transfer alignment, so align the reserved size to the LCM
    // of the minimum alignment (already net buffer and transfer alignment) and the required
    // alignment stride.
    size32 = SkAlignNonPow2(size32, align32);
    auto [transferMapPtr, transferBindInfo] =
            fUploadManager.makeBindInfo(size32,
                                        fRequiredTransferAlignment,
                                        "TransferForStaticBuffer");
    if (!transferMapPtr) {
        SKGPU_LOG_E("Failed to create or map transfer buffer that initializes static GPU data.");
        fMappingFailed = true;
        return nullptr;
    }

    state->fData.push_back(
            {transferBindInfo,
             target,
             SkTo<uint32_t>(requiredAlignment),
#if defined(GPU_TEST_UTILS)
             SkTo<uint32_t>(requiredBytes)
#endif
            });

    state->fTotalRequiredBytes = SkAlignNonPow2(state->fTotalRequiredBytes, align32) + size32;

    return transferMapPtr;
}

bool StaticBufferManager::BufferState::createAndUpdateBindings(
        ResourceProvider* resourceProvider,
        Context* context,
        QueueManager* queueManager,
        GlobalCache* globalCache,
        std::string_view label) const {
    if (!fTotalRequiredBytes) {
        return true; // No buffer needed
    }

    // The static buffer is always copyable when testing.
    constexpr AccessPattern gpuAccessPattern =
#if defined(GPU_TEST_UTILS)
        AccessPattern::kGpuOnlyCopySrc;
#else
        AccessPattern::kGpuOnly;
#endif

    sk_sp<Buffer> staticBuffer = resourceProvider->findOrCreateNonShareableBuffer(
            fTotalRequiredBytes,
            fBufferType,
            gpuAccessPattern,
            std::move(label));
    if (!staticBuffer) {
        SKGPU_LOG_E("Failed to create static buffer for type %d of size %u bytes.\n",
                    (int) fBufferType, fTotalRequiredBytes);
        return false;
    }

    uint32_t offset = 0;
    for (const CopyRange& data : fData) {
        // Each copy range's size should be aligned to the lcm of the required alignment and minimum
        // alignment so we can increment the offset in the static buffer.
        const uint32_t alignment = lcm_alignment(fMinimumAlignment, data.fRequiredAlignment);
        offset = SkAlignNonPow2(offset, alignment);
        SkASSERT(!(offset % fMinimumAlignment) && !(offset % data.fRequiredAlignment));
        uint32_t size = data.fSource.fSize;
        data.fTarget->fBuffer = staticBuffer.get();
        data.fTarget->fOffset = offset;
        data.fTarget->fSize = size;

        auto copyTask = CopyBufferToBufferTask::Make(
                data.fSource.fBuffer, data.fSource.fOffset,
                sk_ref_sp(data.fTarget->fBuffer), data.fTarget->fOffset,
                size);
        // For static buffers, we want them all to be optimized as GPU only buffers. If we are in
        // a protected context, this means the buffers must be non-protected since they will be
        // read in the vertex shader which doesn't allow protected memory access. Thus all the
        // uploads to these buffers must be done as non-protected commands.
        if (!queueManager->addTask(copyTask.get(), context, Protected::kNo)) {
            SKGPU_LOG_E("Failed to copy data to static buffer.\n");
            return false;
        }

        offset += size;
    }

    SkASSERT(offset == fTotalRequiredBytes);
    globalCache->addStaticResource(std::move(staticBuffer));
    return true;
}

StaticBufferManager::FinishResult StaticBufferManager::finalize(Context* context,
                                                                QueueManager* queueManager,
                                                                GlobalCache* globalCache) {
    if (fMappingFailed) {
        return FinishResult::kFailure;
    }

    const size_t totalRequiredBytes = fVertexBufferState.fTotalRequiredBytes +
                                      fIndexBufferState.fTotalRequiredBytes;
    SkASSERT(totalRequiredBytes <= kMaxStaticDataSize);
    if (!totalRequiredBytes) {
        return FinishResult::kNoWork;
    }

    if (!fVertexBufferState.createAndUpdateBindings(fResourceProvider,
                                                   context,
                                                   queueManager,
                                                   globalCache,
                                                   "StaticVertexBuffer")) {
        return FinishResult::kFailure;
    }

#if defined(GPU_TEST_UTILS)
    skia_private::TArray<GlobalCache::StaticVertexCopyRanges> statVertCopy;
    for (const CopyRange& data : fVertexBufferState.fData) {
        statVertCopy.push_back({data.fTarget->fOffset,
                                data.fUnalignedSize,
                                data.fTarget->fSize,
                                data.fRequiredAlignment});
    }
    globalCache->testingOnly_SetStaticVertexInfo(
            statVertCopy,
            fVertexBufferState.fData[0].fTarget->fBuffer);
#endif

    if (!fIndexBufferState.createAndUpdateBindings(fResourceProvider,
                                                   context,
                                                   queueManager,
                                                   globalCache,
                                                   "StaticIndexBuffer")) {
        return FinishResult::kFailure;
    }
    queueManager->addUploadBufferManagerRefs(&fUploadManager);

    // Reset the static buffer manager since the Recording's copy tasks now manage ownership of
    // the transfer buffers and the GlobalCache owns the final static buffers.
    fVertexBufferState.reset();
    fIndexBufferState.reset();

    return FinishResult::kSuccess;
}

} // namespace skgpu::graphite
