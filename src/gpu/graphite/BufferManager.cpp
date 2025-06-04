/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/BufferManager.h"

#include "include/gpu/graphite/Recording.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/QueueManager.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/SharedContext.h"
#include "src/gpu/graphite/UploadBufferManager.h"
#include "src/gpu/graphite/task/ClearBuffersTask.h"
#include "src/gpu/graphite/task/CopyTask.h"
#include "src/gpu/graphite/task/TaskList.h"

#include <limits>
#include <numeric>
#include <optional>

namespace skgpu::graphite {

namespace {

// The limit for all data created by the StaticBufferManager. This data remains alive for
// the entire SharedContext so we want to keep it small and give a concrete upper bound to
// clients for our steady-state memory usage.
// FIXME The current usage is 4732 bytes across static vertex and index buffers, but that includes
// multiple copies of tessellation data, and an unoptimized AnalyticRRect mesh. Once those issues
// are addressed, we can tighten this and decide on the transfer buffer sizing as well.
[[maybe_unused]] static constexpr uint32_t kMaxStaticDataSize = 6 << 10;

uint32_t validate_count_and_stride(size_t count, size_t stride) {
    // size_t may just be uint32_t, so this ensures we have enough bits to do
    // compute the required byte product.
    uint64_t count64 = SkTo<uint64_t>(count);
    uint64_t stride64 = SkTo<uint64_t>(stride);
    uint64_t bytes64 = count64*stride64;
    if (count64 > std::numeric_limits<uint32_t>::max() ||
        stride64 > std::numeric_limits<uint32_t>::max() ||
        bytes64 > std::numeric_limits<uint32_t>::max()) {
        // Return 0 to skip further allocation attempts.
        return 0;
    }
    // Since count64 and stride64 fit into 32-bits, their product did not overflow, and the product
    // fits into 32-bits so this cast is safe.
    return SkTo<uint32_t>(bytes64);
}

uint32_t validate_size(size_t requiredBytes) {
    return validate_count_and_stride(1, requiredBytes);
}

uint32_t sufficient_block_size(uint32_t requiredBytes, uint32_t blockSize) {
    // Always request a buffer at least 'requiredBytes', but keep them in multiples of
    // 'blockSize' for improved reuse.
    static constexpr uint32_t kMaxSize   = std::numeric_limits<uint32_t>::max();
    uint32_t maxBlocks = kMaxSize / blockSize;
    uint32_t blocks = (requiredBytes / blockSize) + 1;
    uint32_t bufferSize = blocks > maxBlocks ? kMaxSize : (blocks * blockSize);
    SkASSERT(requiredBytes < bufferSize);
    return bufferSize;
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

// Buffers can explicitly require a certain alignment. To ensure correctness, we thus need to find
// the lcm of the required alignment and the minimum alignment, which itself is the lcm of the
// buffer type's alignment and the transferBuffer alignment.
// Since we guarantee that none of our alignments will be zero, lcm is commutative and associative:
// lcm(a, b) = lcm(b, a) and lcm(a, lcm(b, c)) = lcm(lcm(a, b), c)
uint32_t align_to_req_min_lcm(uint32_t bytes, uint32_t req, uint32_t min) {
    // This should never be called with a 0 required alignment, DBM guards already, SBM does not
    // append 0 stride vert data
    SkASSERT(req);
    SkASSERT(SkIsPow2(min));
    // The minimum alignment is guaranteed to be a power of two, so we can easily check if the
    // requiredAlignment is a multiple of it.
    if (req & (min - 1)) {
        // If it's not divisible, we need to find the lcm between the two
        bytes = SkTo<uint32_t>(SkAlignNonPow2(bytes, std::lcm(req, min)));
    } else {
        // Since it is divisible, we can align without calling lcm
        // If req != min, then not guaranteed power of two
        if (SkIsPow2(req)) {
            bytes = SkTo<uint32_t>(SkAlignTo(bytes, req));
        } else {
            bytes = SkTo<uint32_t>(SkAlignNonPow2(bytes, req));
        }
    }
    return bytes;
}

std::optional<uint32_t> can_offset_fit(uint32_t reqSize,
                                       uint32_t allocatedSize,
                                       uint32_t currentOffset,
                                       uint32_t minAlignment,
                                       uint32_t reqAlignment) {
    uint32_t startOffset = reqAlignment ?
                           align_to_req_min_lcm(currentOffset, reqAlignment, minAlignment) :
                           SkAlignTo(currentOffset, minAlignment);
    return (allocatedSize > startOffset && reqSize <= allocatedSize - startOffset) ?
           std::optional<uint32_t>(startOffset) : std::nullopt;
}

} // anonymous namespace

// ------------------------------------------------------------------------------------------------
// ScratchBuffer

ScratchBuffer::ScratchBuffer(uint32_t size, uint32_t alignment,
                             sk_sp<Buffer> buffer, DrawBufferManager* owner)
        : fSize(size)
        , fAlignment(alignment)
        , fBuffer(std::move(buffer))
        , fOwner(owner) {
    SkASSERT(fSize > 0);
    SkASSERT(fBuffer);
    SkASSERT(fOwner);
    SkASSERT(fSize <= fBuffer->size());
}

ScratchBuffer::~ScratchBuffer() { this->returnToPool(); }

BindBufferInfo ScratchBuffer::suballocate(size_t requiredBytes) {
    const uint32_t requiredBytes32 = validate_size(requiredBytes);
    if (!this->isValid() || !requiredBytes32) {
        return {};
    }
    std::optional<uint32_t> offset = can_offset_fit(requiredBytes32, fSize, fOffset, fAlignment, 0);
    if (!offset.has_value()) {
        return {};
    }
    fOffset = offset.value() + requiredBytes32;
    return {fBuffer.get(), offset.value(), requiredBytes32};
}

void ScratchBuffer::returnToPool() {
    if (fOwner && fBuffer) {
        // TODO: Generalize the pool to other buffer types.
        fOwner->fReusableScratchStorageBuffers.push_back(std::move(fBuffer));
        SkASSERT(!fBuffer);
    }
}

// ------------------------------------------------------------------------------------------------
// DrawBufferManager

DrawBufferManager::DrawBufferManager(ResourceProvider* resourceProvider,
                                     const Caps* caps,
                                     UploadBufferManager* uploadManager,
                                     DrawBufferManagerOptions dbmOpts)
        : fResourceProvider(resourceProvider)
        , fCaps(caps)
        , fUploadManager(uploadManager)
        , fCurrentBuffers{{{BufferType::kVertex,
                            dbmOpts.fVertexBufferMinSize, dbmOpts.fVertexBufferMaxSize, caps},
                           {BufferType::kIndex,
                            dbmOpts.fIndexBufferSize, dbmOpts.fIndexBufferSize, caps},
                           {BufferType::kUniform,
                            dbmOpts.fUniformBufferSize, dbmOpts.fUniformBufferSize, caps},
                           // mapped storage
                           {BufferType::kStorage,
                            dbmOpts.fStorageBufferMinSize, dbmOpts.fStorageBufferMaxSize, caps},
                           // GPU-only storage
                           {BufferType::kStorage,
                            dbmOpts.fStorageBufferMinSize, dbmOpts.fStorageBufferMinSize, caps},
                           {BufferType::kVertexStorage,
                            dbmOpts.fVertexBufferMinSize, dbmOpts.fVertexBufferMinSize, caps},
                           {BufferType::kIndexStorage,
                            dbmOpts.fIndexBufferSize, dbmOpts.fIndexBufferSize, caps},
                           {BufferType::kIndirect,
                            dbmOpts.fStorageBufferMinSize, dbmOpts.fStorageBufferMinSize, caps}}}
#if defined(GPU_TEST_UTILS)
        , fUseExactBuffSizes(dbmOpts.fUseExactBuffSizes)
        , fAllowCopyingGpuOnly(dbmOpts.fAllowCopyingGpuOnly)
#endif
{
    // Make sure the buffer size constants are all powers of two, so we can align to them
    // efficiently when dynamically sizing buffers.
    SkASSERT(SkIsPow2(dbmOpts.fVertexBufferMinSize));
    SkASSERT(SkIsPow2(dbmOpts.fVertexBufferMaxSize));
    SkASSERT(SkIsPow2(dbmOpts.fIndexBufferSize));
    SkASSERT(SkIsPow2(dbmOpts.fUniformBufferSize));
    SkASSERT(SkIsPow2(dbmOpts.fStorageBufferMinSize));
    SkASSERT(SkIsPow2(dbmOpts.fStorageBufferMaxSize));
}

DrawBufferManager::~DrawBufferManager() {}

// For simplicity, if transfer buffers are being used, we align the data to the max alignment of
// either the final buffer type or cpu->gpu transfer alignment so that the buffers are laid out
// the same in memory.
DrawBufferManager::BufferInfo::BufferInfo(BufferType type,
                                          uint32_t minBlockSize,
                                          uint32_t maxBlockSize,
                                          const Caps* caps)
        : fType(type)
        , fMinimumAlignment(minimum_alignment(type, !caps->drawBufferCanBeMapped(), caps))
        , fMinBlockSize(minBlockSize)
        , fMaxBlockSize(maxBlockSize)
        , fCurBlockSize(SkAlignTo(minBlockSize, fMinimumAlignment)) {}

bool DrawBufferManager::willVertexOverflow(size_t count, size_t dataStride,
                                           size_t alignStride) const {
    uint32_t requiredBytes = validate_count_and_stride(count, dataStride);
    const BufferInfo& vertBuff = fCurrentBuffers[kVertexBufferIndex];
    if (!requiredBytes || !vertBuff.fBuffer) {
        return false;
    }
    return !can_offset_fit(requiredBytes,
                          SkTo<uint32_t>(vertBuff.fBuffer->size()),
                          vertBuff.fOffset,
                          vertBuff.fMinimumAlignment,
                          alignStride).has_value();
}

// For the vertexWriter, we explicitly pass in the required stride to align the mapped
// bindBuffer to try to keep the buffer contiguous with future vertex data.
std::pair<VertexWriter, BindBufferInfo> DrawBufferManager::getVertexWriter(size_t count,
                                                                           size_t dataStride,
                                                                           size_t alignStride) {
    uint32_t requiredBytes = validate_count_and_stride(count, dataStride);
    if (!requiredBytes) {
        return {};
    }

    auto& info = fCurrentBuffers[kVertexBufferIndex];
    auto [ptr, bindInfo] =
        this->prepareMappedBindBuffer(&info, "VertexBuffer", requiredBytes, alignStride);
    return {VertexWriter(ptr, requiredBytes), bindInfo};
}

void DrawBufferManager::returnVertexBytes(size_t unusedBytes) {
    if (fMappingFailed) {
        // The caller can be unaware that the written data went to no-where and will still call
        // this function.
        return;
    }
    SkASSERT(fCurrentBuffers[kVertexBufferIndex].fOffset >= unusedBytes);
    fCurrentBuffers[kVertexBufferIndex].fOffset -= unusedBytes;
}

std::pair<IndexWriter, BindBufferInfo> DrawBufferManager::getIndexWriter(size_t count,
                                                                         size_t stride) {
    uint32_t requiredBytes = validate_count_and_stride(count, stride);
    if (!requiredBytes) {
        return {};
    }

    auto& info = fCurrentBuffers[kIndexBufferIndex];
    auto [ptr, bindInfo] = this->prepareMappedBindBuffer(&info, "IndexBuffer", requiredBytes);
    return {IndexWriter(ptr, requiredBytes), bindInfo};
}

std::pair<UniformWriter, BindBufferInfo> DrawBufferManager::getUniformWriter(size_t count,
                                                                             size_t stride) {
    uint32_t requiredBytes = validate_count_and_stride(count, stride);
    if (!requiredBytes) {
        return {};
    }

    auto& info = fCurrentBuffers[kUniformBufferIndex];
    auto [ptr, bindInfo] = this->prepareMappedBindBuffer(&info, "UniformBuffer", requiredBytes);
    return {UniformWriter(ptr, requiredBytes), bindInfo};
}

std::pair<UniformWriter, BindBufferInfo> DrawBufferManager::getSsboWriter(size_t count,
                                                                          size_t stride,
                                                                          size_t alignment) {
    uint32_t requiredBytes = validate_count_and_stride(count, stride);
    if (!requiredBytes) {
        return {};
    }

    auto& info = fCurrentBuffers[kStorageBufferIndex];
    auto [ptr, bindInfo] =
            this->prepareMappedBindBuffer(&info, "StorageBuffer", requiredBytes, alignment);
    return {UniformWriter(ptr, requiredBytes), bindInfo};
}

std::pair<UniformWriter, BindBufferInfo> DrawBufferManager::getSsboWriter(size_t count,
                                                                          size_t stride) {
    // By setting alignment=0, use the default buffer alignment requirement for storage buffers.
    return this->getSsboWriter(count, stride, /*alignment=*/0);
}

std::pair<UniformWriter, BindBufferInfo> DrawBufferManager::getAlignedSsboWriter(size_t count,
                                                                                 size_t stride) {
    // Align to the provided element stride.
    return this->getSsboWriter(count, stride, stride);
}

std::pair<void* /*mappedPtr*/, BindBufferInfo> DrawBufferManager::getUniformPointer(
            size_t requiredBytes) {
    uint32_t requiredBytes32 = validate_size(requiredBytes);
    if (!requiredBytes32) {
        return {};
    }

    auto& info = fCurrentBuffers[kUniformBufferIndex];
    return this->prepareMappedBindBuffer(&info, "UniformBuffer", requiredBytes32);
}

std::pair<void* /*mappedPtr*/, BindBufferInfo> DrawBufferManager::getStoragePointer(
        size_t requiredBytes) {
    uint32_t requiredBytes32 = validate_size(requiredBytes);
    if (!requiredBytes32) {
        return {};
    }

    auto& info = fCurrentBuffers[kStorageBufferIndex];
    return this->prepareMappedBindBuffer(&info, "StorageBuffer", requiredBytes32);
}

BindBufferInfo DrawBufferManager::getStorage(size_t requiredBytes, ClearBuffer cleared) {
    uint32_t requiredBytes32 = validate_size(requiredBytes);
    if (!requiredBytes32) {
        return {};
    }

    auto& info = fCurrentBuffers[kGpuOnlyStorageBufferIndex];
    return this->prepareBindBuffer(&info,
                                   "StorageBuffer",
                                   requiredBytes32,
                                   /*requiredAlignment=*/0,
                                   /*supportCpuUpload=*/false,
                                   cleared);
}

BindBufferInfo DrawBufferManager::getVertexStorage(size_t requiredBytes) {
    uint32_t requiredBytes32 = validate_size(requiredBytes);
    if (!requiredBytes32) {
        return {};
    }

    auto& info = fCurrentBuffers[kVertexStorageBufferIndex];
    return this->prepareBindBuffer(&info, "VertexStorageBuffer", requiredBytes32);
}

BindBufferInfo DrawBufferManager::getIndexStorage(size_t requiredBytes) {
    uint32_t requiredBytes32 = validate_size(requiredBytes);
    if (!requiredBytes32) {
        return {};
    }

    auto& info = fCurrentBuffers[kIndexStorageBufferIndex];
    return this->prepareBindBuffer(&info, "IndexStorageBuffer", requiredBytes32);
}

BindBufferInfo DrawBufferManager::getIndirectStorage(size_t requiredBytes, ClearBuffer cleared) {
    uint32_t requiredBytes32 = validate_size(requiredBytes);
    if (!requiredBytes32) {
        return {};
    }

    auto& info = fCurrentBuffers[kIndirectStorageBufferIndex];
    return this->prepareBindBuffer(&info,
                                   "IndirectStorageBuffer",
                                   requiredBytes32,
                                   /*requiredAlignment=*/0,
                                   /*supportCpuUpload=*/false,
                                   cleared);
}

ScratchBuffer DrawBufferManager::getScratchStorage(size_t requiredBytes) {
    uint32_t requiredBytes32 = validate_size(requiredBytes);
    if (!requiredBytes32 || fMappingFailed) {
        return {};
    }

    // TODO: Generalize the pool to other buffer types.
    auto& info = fCurrentBuffers[kStorageBufferIndex];

    uint32_t bufferSize =
#if defined(GPU_TEST_UTILS)
            fUseExactBuffSizes ? info.fCurBlockSize :
#endif
                                 sufficient_block_size(requiredBytes32, info.fCurBlockSize);

    sk_sp<Buffer> buffer = this->findReusableSbo(bufferSize);
    if (!buffer) {
        buffer = fResourceProvider->findOrCreateBuffer(
                bufferSize, BufferType::kStorage, AccessPattern::kGpuOnly, "ScratchStorageBuffer");

        if (!buffer) {
            this->onFailedBuffer();
            return {};
        }
    }
    return {requiredBytes32, info.fMinimumAlignment, std::move(buffer), this};
}

void DrawBufferManager::onFailedBuffer() {
    fMappingFailed = true;

    // Clean up and unmap everything now
    fClearList.clear();
    fReusableScratchStorageBuffers.clear();

    for (auto& [buffer, _] : fUsedBuffers) {
        if (buffer->isMapped()) {
            buffer->unmap();
        }
    }
    fUsedBuffers.clear();

    for (auto& info : fCurrentBuffers) {
        if (info.fBuffer && info.fBuffer->isMapped()) {
            info.fBuffer->unmap();
        }
        info.fBuffer = nullptr;
        info.fTransferBuffer = {};
        info.fOffset = 0;
    }
}

bool DrawBufferManager::transferToRecording(Recording* recording) {
    if (fMappingFailed) {
        // All state should have been reset by onFailedBuffer() except for this error flag.
        SkASSERT(fUsedBuffers.empty() &&
                 fClearList.empty() &&
                 fReusableScratchStorageBuffers.empty());
        fMappingFailed = false;
        return false;
    }

    if (!fClearList.empty()) {
        recording->priv().taskList()->add(ClearBuffersTask::Make(std::move(fClearList)));
    }

    // Transfer the buffers in the reuse pool to the recording.
    // TODO: Allow reuse across different Recordings?
    for (auto& buffer : fReusableScratchStorageBuffers) {
        recording->priv().addResourceRef(std::move(buffer));
    }
    fReusableScratchStorageBuffers.clear();

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

    // The current draw buffers have not been added to fUsedBuffers,
    // so we need to handle them as well.
    for (auto& info : fCurrentBuffers) {
        if (!info.fBuffer) {
            continue;
        }
        if (info.fTransferBuffer) {
            // A transfer buffer should always be mapped at this stage
            SkASSERT(info.fBuffer);
            SkASSERT(!fCaps->drawBufferCanBeMapped());
            // Since the transfer buffer is managed by the UploadManager, we don't manually unmap
            // it here or need to pass a ref into CopyBufferToBufferTask.
            recording->priv().taskList()->add(
                    CopyBufferToBufferTask::Make(info.fTransferBuffer.fBuffer,
                                                 info.fTransferBuffer.fOffset,
                                                 info.fBuffer,
                                                 /*dstOffset=*/0,
                                                 info.fBuffer->size()));
        } else {
            if (info.fBuffer->isMapped()) {
                info.fBuffer->unmap();
            }
            recording->priv().addResourceRef(std::move(info.fBuffer));
        }

        // For each buffer type, update the block size to use for new buffers, based on the total
        // storage used since the last flush.
        const uint32_t reqSize = SkAlignTo(info.fUsedSize + info.fOffset, info.fMinBlockSize);
        info.fCurBlockSize = std::clamp(reqSize, info.fMinBlockSize, info.fMaxBlockSize);
        info.fUsedSize = 0;

        info.fTransferBuffer = {};
        info.fOffset = 0;
    }

    return true;
}

// Only when defined(GPU_TEST_UTILS) do we allow enabling copying.
AccessPattern DrawBufferManager::getGpuAccessPattern(bool isGpuOnlyAccess) const {
    if (isGpuOnlyAccess) {
#if defined(GPU_TEST_UTILS)
        return fAllowCopyingGpuOnly ? AccessPattern::kGpuOnlyCopySrc : AccessPattern::kGpuOnly;
#else
        return AccessPattern::kGpuOnly;
#endif
    } else {
        return AccessPattern::kHostVisible;
    }
}

std::pair<void*, BindBufferInfo> DrawBufferManager::prepareMappedBindBuffer(
        BufferInfo* info,
        std::string_view label,
        uint32_t requiredBytes,
        uint32_t requiredAlignment) {
    BindBufferInfo bindInfo = this->prepareBindBuffer(info,
                                                      std::move(label),
                                                      requiredBytes,
                                                      requiredAlignment,
                                                      /*supportCpuUpload=*/true);
    if (!bindInfo) {
        // prepareBindBuffer() already called onFailedBuffer()
        SkASSERT(fMappingFailed);
        return {nullptr, {}};
    }

    // If there's a transfer buffer, its mapped pointer should already have been validated
    SkASSERT(!info->fTransferBuffer || info->fTransferMapPtr);
    void* mapPtr = info->fTransferBuffer ? info->fTransferMapPtr : info->fBuffer->map();
    if (!mapPtr) {
        // Mapping a direct draw buffer failed
        this->onFailedBuffer();
        return {nullptr, {}};
    }

    mapPtr = SkTAddOffset<void>(mapPtr, static_cast<ptrdiff_t>(bindInfo.fOffset));
    return {mapPtr, bindInfo};
}

BindBufferInfo DrawBufferManager::prepareBindBuffer(BufferInfo* info,
                                                    std::string_view label,
                                                    uint32_t requiredBytes,
                                                    uint32_t requiredAlignment,
                                                    bool supportCpuUpload,
                                                    ClearBuffer cleared) {
    SkASSERT(info);
    SkASSERT(requiredBytes);

    if (fMappingFailed) {
        return {};
    }

    auto offset = info->fBuffer ? can_offset_fit(requiredBytes,
                                                 SkTo<uint32_t>(info->fBuffer->size()),
                                                 info->fOffset,
                                                 info->fMinimumAlignment,
                                                 requiredAlignment)
                                : std::optional<uint32_t>(0);
    const bool overflowedBuffer = !offset.has_value();
    if (overflowedBuffer) {
        fUsedBuffers.emplace_back(std::move(info->fBuffer), info->fTransferBuffer);
        info->fTransferBuffer = {};
        info->fUsedSize += info->fOffset;
    } else {
        info->fOffset = offset.value();
    }

    // A transfer buffer is not necessary if the caller does not intend to upload CPU data to it.
    const bool useTransferBuffer = supportCpuUpload && !fCaps->drawBufferCanBeMapped();
    if (!info->fBuffer) {
        // Create the first buffer with the full fCurBlockSize, but create subsequent buffers with a
        // smaller size if fCurBlockSize has increased from the minimum. This way if we use just a
        // little more than fCurBlockSize total storage this frame, we won't necessarily double our
        // total storage allocation.
        const uint32_t blockSize = overflowedBuffer
                                           ? std::max(info->fCurBlockSize / 4, info->fMinBlockSize)
                                           : info->fCurBlockSize;
        const uint32_t bufferSize =
#if defined(GPU_TEST_UTILS)
            fUseExactBuffSizes ? info->fCurBlockSize :
#endif
                                 sufficient_block_size(requiredBytes, blockSize);

        // This buffer can be GPU-only if
        //     a) the caller does not intend to ever upload CPU data to the buffer; or
        //     b) CPU data will get uploaded to fBuffer only via a transfer buffer
        info->fBuffer = fResourceProvider->findOrCreateBuffer(
                bufferSize,
                info->fType,
                this->getGpuAccessPattern(useTransferBuffer || !supportCpuUpload),
                std::move(label));
        info->fOffset = 0;
        if (!info->fBuffer) {
            this->onFailedBuffer();
            return {};
        }
    }

    if (useTransferBuffer && !info->fTransferBuffer) {
        std::tie(info->fTransferMapPtr, info->fTransferBuffer) =
                fUploadManager->makeBindInfo(info->fBuffer->size(),
                                             fCaps->requiredTransferBufferAlignment(),
                                             "TransferForDataBuffer");
        if (!info->fTransferBuffer) {
            this->onFailedBuffer();
            return {};
        }
        SkASSERT(info->fTransferMapPtr);
    }

    SkASSERT(info->fOffset % (requiredAlignment ?
            requiredAlignment : info->fMinimumAlignment) == 0);
    BindBufferInfo bindInfo{info->fBuffer.get(), info->fOffset, requiredBytes};
    info->fOffset += requiredBytes;

    if (cleared == ClearBuffer::kYes) {
        fClearList.push_back(bindInfo);
    }

    SkASSERT(info->fOffset <= info->fBuffer->size());
    return bindInfo;
}

sk_sp<Buffer> DrawBufferManager::findReusableSbo(size_t bufferSize) {
    SkASSERT(bufferSize);
    SkASSERT(!fMappingFailed);

    for (int i = 0; i < fReusableScratchStorageBuffers.size(); ++i) {
        sk_sp<Buffer>* buffer = &fReusableScratchStorageBuffers[i];
        if ((*buffer)->size() >= bufferSize) {
            auto found = std::move(*buffer);
            // Fill the hole left by the move (if necessary) and shrink the pool.
            if (i < fReusableScratchStorageBuffers.size() - 1) {
                *buffer = std::move(fReusableScratchStorageBuffers.back());
            }
            fReusableScratchStorageBuffers.pop_back();
            return found;
        }
    }
    return nullptr;
}

// ------------------------------------------------------------------------------------------------
// StaticBufferManager

StaticBufferManager::StaticBufferManager(ResourceProvider* resourceProvider,
                                         const Caps* caps)
        : fResourceProvider(resourceProvider)
        , fUploadManager(resourceProvider, caps)
        , fRequiredTransferAlignment(SkTo<uint32_t>(caps->requiredTransferBufferAlignment()))
        , fVertexBufferInfo(BufferType::kVertex, caps)
        , fIndexBufferInfo(BufferType::kIndex, caps) {}
StaticBufferManager::~StaticBufferManager() = default;

StaticBufferManager::BufferInfo::BufferInfo(BufferType type, const Caps* caps)
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
    void* data = this->prepareStaticData(&fVertexBufferInfo, size, stride * 4, binding);
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
    void* data = this->prepareStaticData(&fIndexBufferInfo,
                                         size,
                                         fIndexBufferInfo.fMinimumAlignment,
                                         binding);
    return VertexWriter{data, size};
}

void* StaticBufferManager::prepareStaticData(BufferInfo* info,
                                             size_t requiredBytes,
                                             size_t requiredAlignment,
                                             BindBufferInfo* target) {
    // Zero-out the target binding in the event of any failure in actually transfering data later.
    SkASSERT(target);
    *target = {nullptr, 0};
    uint32_t size32 = validate_size(requiredBytes);
    if (!size32 || fMappingFailed) {
        return nullptr;
    }

    // Copy data must be aligned to the transfer alignment, so align the reserved size to the LCM
    // of the minimum alignment (already net buffer and transfer alignment) and the required
    // alignment stride.
    size32 = align_to_req_min_lcm(size32, requiredAlignment, info->fMinimumAlignment);
    auto [transferMapPtr, transferBindInfo] =
            fUploadManager.makeBindInfo(size32,
                                        fRequiredTransferAlignment,
                                        "TransferForStaticBuffer");
    if (!transferMapPtr) {
        SKGPU_LOG_E("Failed to create or map transfer buffer that initializes static GPU data.");
        fMappingFailed = true;
        return nullptr;
    }

#if defined(GPU_TEST_UTILS)
    info->fData.push_back({transferBindInfo, target, requiredAlignment, requiredBytes});
#else
    info->fData.push_back({transferBindInfo, target, requiredAlignment});
#endif
    info->fTotalRequiredBytes =
        align_to_req_min_lcm(info->fTotalRequiredBytes,
                             requiredAlignment,
                             info->fMinimumAlignment) + size32;
    return transferMapPtr;
}

bool StaticBufferManager::BufferInfo::createAndUpdateBindings(
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

    sk_sp<Buffer> staticBuffer = resourceProvider->findOrCreateBuffer(
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
        offset = align_to_req_min_lcm(offset, data.fRequiredAlignment, fMinimumAlignment);
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

    const size_t totalRequiredBytes = fVertexBufferInfo.fTotalRequiredBytes +
                                      fIndexBufferInfo.fTotalRequiredBytes;
    SkASSERT(totalRequiredBytes <= kMaxStaticDataSize);
    if (!totalRequiredBytes) {
        return FinishResult::kNoWork;
    }

    if (!fVertexBufferInfo.createAndUpdateBindings(fResourceProvider,
                                                   context,
                                                   queueManager,
                                                   globalCache,
                                                   "StaticVertexBuffer")) {
        return FinishResult::kFailure;
    }

#if defined(GPU_TEST_UTILS)
    skia_private::TArray<GlobalCache::StaticVertexCopyRanges> statVertCopy;
    for (const CopyRange& data : fVertexBufferInfo.fData) {
        statVertCopy.push_back({data.fTarget->fOffset,
                                data.fUnalignedSize,
                                data.fTarget->fSize,
                                data.fRequiredAlignment});
    }
    globalCache->testingOnly_SetStaticVertexInfo(
            statVertCopy,
            fVertexBufferInfo.fData[0].fTarget->fBuffer);
#endif

    if (!fIndexBufferInfo.createAndUpdateBindings(fResourceProvider,
                                                  context,
                                                  queueManager,
                                                  globalCache,
                                                  "StaticIndexBuffer")) {
        return FinishResult::kFailure;
    }
    queueManager->addUploadBufferManagerRefs(&fUploadManager);

    // Reset the static buffer manager since the Recording's copy tasks now manage ownership of
    // the transfer buffers and the GlobalCache owns the final static buffers.
    fVertexBufferInfo.reset();
    fIndexBufferInfo.reset();

    return FinishResult::kSuccess;
}

} // namespace skgpu::graphite
