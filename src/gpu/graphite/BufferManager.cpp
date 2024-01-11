/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/BufferManager.h"

#include "include/gpu/graphite/Recording.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ClearBuffersTask.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/CopyTask.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/QueueManager.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/SharedContext.h"
#include "src/gpu/graphite/UploadBufferManager.h"

namespace skgpu::graphite {

namespace {

// TODO: Tune these values on real world data
static constexpr size_t kVertexBufferSize = 16 << 10; // 16 KB
static constexpr size_t kIndexBufferSize =   2 << 10; //  2 KB
static constexpr size_t kUniformBufferSize = 2 << 10; //  2 KB
static constexpr size_t kStorageBufferSize = 2 << 10; //  2 KB

// The limit for all data created by the StaticBufferManager. This data remains alive for
// the entire SharedContext so we want to keep it small and give a concrete upper bound to
// clients for our steady-state memory usage.
// FIXME The current usage is 4732 bytes across static vertex and index buffers, but that includes
// multiple copies of tessellation data, and an unoptimized AnalyticRRect mesh. Once those issues
// are addressed, we can tighten this and decide on the transfer buffer sizing as well.
[[maybe_unused]] static constexpr size_t kMaxStaticDataSize = 6 << 10;

size_t sufficient_block_size(size_t requiredBytes, size_t blockSize) {
    // Always request a buffer at least 'requiredBytes', but keep them in multiples of
    // 'blockSize' for improved reuse.
    static constexpr size_t kMaxSize   = std::numeric_limits<size_t>::max();
    size_t maxBlocks = kMaxSize / blockSize;
    size_t blocks = (requiredBytes / blockSize) + 1;
    size_t bufferSize = blocks > maxBlocks ? kMaxSize : (blocks * blockSize);
    SkASSERT(requiredBytes < bufferSize);
    return bufferSize;
}

bool can_fit(size_t requestedSize,
             size_t allocatedSize,
             size_t currentOffset,
             size_t alignment) {
    size_t startOffset = SkAlignTo(currentOffset, alignment);
    return requestedSize <= (allocatedSize - startOffset);
}

size_t starting_alignment(BufferType type, bool useTransferBuffers, const Caps* caps) {
    // Both vertex and index data is aligned to 4 bytes by default
    size_t alignment = 4;
    if (type == BufferType::kUniform) {
        alignment = caps->requiredUniformBufferAlignment();
    } else if (type == BufferType::kStorage || type == BufferType::kVertexStorage ||
               type == BufferType::kIndexStorage || type == BufferType::kIndirect) {
        alignment = caps->requiredStorageBufferAlignment();
    }
    if (useTransferBuffers) {
        alignment = std::max(alignment, caps->requiredTransferBufferAlignment());
    }
    return alignment;
}

} // anonymous namespace

// ------------------------------------------------------------------------------------------------
// DrawBufferManager

DrawBufferManager::DrawBufferManager(ResourceProvider* resourceProvider,
                                     const Caps* caps,
                                     UploadBufferManager* uploadManager)
        : fResourceProvider(resourceProvider)
        , fCaps(caps)
        , fUploadManager(uploadManager)
        , fCurrentBuffers{{
                { BufferType::kVertex,        kVertexBufferSize,  caps },
                { BufferType::kIndex,         kIndexBufferSize,   caps },
                { BufferType::kUniform,       kUniformBufferSize, caps },
                { BufferType::kStorage,       kStorageBufferSize, caps },  // mapped storage
                { BufferType::kStorage,       kStorageBufferSize, caps },  // GPU-only storage
                { BufferType::kVertexStorage, kVertexBufferSize,  caps },
                { BufferType::kIndexStorage,  kIndexBufferSize,   caps },
                { BufferType::kIndirect,      kStorageBufferSize, caps } }} {}

DrawBufferManager::~DrawBufferManager() {}

// For simplicity, if transfer buffers are being used, we align the data to the max alignment of
// either the final buffer type or cpu->gpu transfer alignment so that the buffers are laid out
// the same in memory.
DrawBufferManager::BufferInfo::BufferInfo(BufferType type, size_t blockSize, const Caps* caps)
        : fType(type)
        , fStartAlignment(starting_alignment(type, !caps->drawBufferCanBeMapped(), caps))
        , fBlockSize(SkAlignTo(blockSize, fStartAlignment)) {}

std::tuple<VertexWriter, BindBufferInfo> DrawBufferManager::getVertexWriter(size_t requiredBytes) {
    if (!requiredBytes) {
        return {};
    }

    auto& info = fCurrentBuffers[kVertexBufferIndex];
    auto [ptr, bindInfo] = this->prepareMappedBindBuffer(&info, requiredBytes);
    if (!ptr) {
        return {};
    }

    return {VertexWriter(ptr, requiredBytes), bindInfo};
}

void DrawBufferManager::returnVertexBytes(size_t unusedBytes) {
    SkASSERT(fCurrentBuffers[kVertexBufferIndex].fOffset >= unusedBytes);
    fCurrentBuffers[kVertexBufferIndex].fOffset -= unusedBytes;
}

std::tuple<IndexWriter, BindBufferInfo> DrawBufferManager::getIndexWriter(size_t requiredBytes) {
    if (!requiredBytes) {
        return {};
    }

    auto& info = fCurrentBuffers[kIndexBufferIndex];
    auto [ptr, bindInfo] = this->prepareMappedBindBuffer(&info, requiredBytes);
    if (!ptr) {
        return {};
    }

    return {IndexWriter(ptr, requiredBytes), bindInfo};
}

std::tuple<UniformWriter, BindBufferInfo> DrawBufferManager::getUniformWriter(
        size_t requiredBytes) {
    if (!requiredBytes) {
        return {};
    }

    auto& info = fCurrentBuffers[kUniformBufferIndex];
    auto [ptr, bindInfo] = this->prepareMappedBindBuffer(&info, requiredBytes);
    if (!ptr) {
        return {};
    }

    return {UniformWriter(ptr, requiredBytes), bindInfo};
}

std::tuple<UniformWriter, BindBufferInfo> DrawBufferManager::getSsboWriter(size_t requiredBytes) {
    if (!requiredBytes) {
        return {};
    }

    auto& info = fCurrentBuffers[kStorageBufferIndex];
    auto [ptr, bindInfo] = this->prepareMappedBindBuffer(&info, requiredBytes);
    if (!ptr) {
        return {};
    }
    return {UniformWriter(ptr, requiredBytes), bindInfo};
}

std::tuple<void*, BindBufferInfo> DrawBufferManager::getUniformPointer(size_t requiredBytes) {
    if (!requiredBytes) {
        return {};
    }

    auto& info = fCurrentBuffers[kUniformBufferIndex];
    return this->prepareMappedBindBuffer(&info, requiredBytes);
}

std::tuple<void*, BindBufferInfo> DrawBufferManager::getStoragePointer(size_t requiredBytes) {
    if (!requiredBytes) {
        return {};
    }

    auto& info = fCurrentBuffers[kStorageBufferIndex];
    return this->prepareMappedBindBuffer(&info, requiredBytes);
}

BindBufferInfo DrawBufferManager::getStorage(size_t requiredBytes, ClearBuffer cleared) {
    if (!requiredBytes) {
        return {};
    }

    auto& info = fCurrentBuffers[kGpuOnlyStorageBufferIndex];
    return this->prepareBindBuffer(&info, requiredBytes, /*supportCpuUpload=*/false, cleared);
}

BindBufferInfo DrawBufferManager::getVertexStorage(size_t requiredBytes) {
    if (!requiredBytes) {
        return {};
    }

    auto& info = fCurrentBuffers[kVertexStorageBufferIndex];
    return this->prepareBindBuffer(&info, requiredBytes);
}

BindBufferInfo DrawBufferManager::getIndexStorage(size_t requiredBytes) {
    if (!requiredBytes) {
        return {};
    }

    auto& info = fCurrentBuffers[kIndexStorageBufferIndex];
    return this->prepareBindBuffer(&info, requiredBytes);
}

BindBufferInfo DrawBufferManager::getIndirectStorage(size_t requiredBytes, ClearBuffer cleared) {
    if (!requiredBytes) {
        return {};
    }

    auto& info = fCurrentBuffers[kIndirectStorageBufferIndex];
    return this->prepareBindBuffer(&info, requiredBytes, /*supportCpuUpload=*/false, cleared);
}

void DrawBufferManager::transferToRecording(Recording* recording) {
    if (!fClearList.empty()) {
        recording->priv().addTask(ClearBuffersTask::Make(std::move(fClearList)));
    }

    for (auto& [buffer, transferBuffer] : fUsedBuffers) {
        if (transferBuffer) {
            SkASSERT(buffer);
            SkASSERT(!fCaps->drawBufferCanBeMapped());
            // Since the transfer buffer is managed by the UploadManager, we don't manually unmap
            // it here or need to pass a ref into CopyBufferToBufferTask.
            size_t copySize = buffer->size();
            recording->priv().addTask(
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
    for (auto &info : fCurrentBuffers) {
        if (!info.fBuffer) {
            continue;
        }
        if (info.fTransferBuffer) {
            // A transfer buffer should always be mapped at this stage
            SkASSERT(info.fBuffer);
            SkASSERT(!fCaps->drawBufferCanBeMapped());
            // Since the transfer buffer is managed by the UploadManager, we don't manually unmap
            // it here or need to pass a ref into CopyBufferToBufferTask.
            recording->priv().addTask(
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
        info.fTransferBuffer = {};
        info.fOffset = 0;
    }
}

std::pair<void*, BindBufferInfo> DrawBufferManager::prepareMappedBindBuffer(BufferInfo* info,
                                                                            size_t requiredBytes) {
    BindBufferInfo bindInfo = this->prepareBindBuffer(info,
                                                      requiredBytes,
                                                      /*supportCpuUpload=*/true);
    if (!bindInfo) {
        return {nullptr, {}};
    }

    void* mapPtr = info->fTransferBuffer ? info->fTransferMapPtr : info->fBuffer->map();

    mapPtr = SkTAddOffset<void>(mapPtr, static_cast<ptrdiff_t>(bindInfo.fOffset));
    return {mapPtr, bindInfo};
}

BindBufferInfo DrawBufferManager::prepareBindBuffer(BufferInfo* info,
                                                    size_t requiredBytes,
                                                    bool supportCpuUpload,
                                                    ClearBuffer cleared) {
    SkASSERT(info);
    SkASSERT(requiredBytes);

    // A transfer buffer is not necessary if the caller does not intend to upload CPU data to it.
    bool useTransferBuffer = supportCpuUpload && !fCaps->drawBufferCanBeMapped();

    if (info->fBuffer &&
        !can_fit(requiredBytes, info->fBuffer->size(), info->fOffset, info->fStartAlignment)) {
        fUsedBuffers.emplace_back(std::move(info->fBuffer), info->fTransferBuffer);
        info->fTransferBuffer = {};
    }

    if (!info->fBuffer) {
        // This buffer can be GPU-only if
        //     a) the caller does not intend to ever upload CPU data to the buffer; or
        //     b) CPU data will get uploaded to fBuffer only via a transfer buffer
        AccessPattern accessPattern = (useTransferBuffer || !supportCpuUpload)
                                              ? AccessPattern::kGpuOnly
                                              : AccessPattern::kHostVisible;
        size_t bufferSize = sufficient_block_size(requiredBytes, info->fBlockSize);
        info->fBuffer =
                fResourceProvider->findOrCreateBuffer(bufferSize, info->fType, accessPattern);
        info->fOffset = 0;
        if (!info->fBuffer) {
            return {};
        }
    }

    if (useTransferBuffer && !info->fTransferBuffer) {
        std::tie(info->fTransferMapPtr, info->fTransferBuffer) = fUploadManager->makeBindInfo(
                info->fBuffer->size(), fCaps->requiredTransferBufferAlignment());

        if (!info->fTransferBuffer) {
            return {};
        }
        SkASSERT(info->fTransferMapPtr);
    }

    info->fOffset = SkAlignTo(info->fOffset, info->fStartAlignment);
    BindBufferInfo bindInfo{info->fBuffer.get(), info->fOffset};
    info->fOffset += requiredBytes;

    if (cleared == ClearBuffer::kYes) {
        fClearList.push_back({bindInfo.fBuffer, bindInfo.fOffset, requiredBytes});
    }

    return bindInfo;
}

// ------------------------------------------------------------------------------------------------
// StaticBufferManager

StaticBufferManager::StaticBufferManager(ResourceProvider* resourceProvider,
                                         const Caps* caps)
        : fResourceProvider(resourceProvider)
        , fUploadManager(resourceProvider, caps)
        , fVertexBufferInfo(BufferType::kVertex, caps)
        , fIndexBufferInfo(BufferType::kIndex, caps)
        , fRequiredTransferAlignment(caps->requiredTransferBufferAlignment()) {}
StaticBufferManager::~StaticBufferManager() = default;

StaticBufferManager::BufferInfo::BufferInfo(BufferType type, const Caps* caps)
        : fBufferType(type)
        , fAlignment(starting_alignment(type, /*useTransferBuffers=*/true, caps))
        , fTotalRequiredBytes(0) {}

VertexWriter StaticBufferManager::getVertexWriter(size_t size, BindBufferInfo* binding) {
    void* data = this->prepareStaticData(&fVertexBufferInfo, size, binding);
    return VertexWriter{data, size};
}

VertexWriter StaticBufferManager::getIndexWriter(size_t size, BindBufferInfo* binding) {
    void* data = this->prepareStaticData(&fIndexBufferInfo, size, binding);
    return VertexWriter{data, size};
}

void* StaticBufferManager::prepareStaticData(BufferInfo* info,
                                             size_t size,
                                             BindBufferInfo* target) {
    // Zero-out the target binding in the event of any failure in actually transfering data later.
    SkASSERT(target);
    *target = {nullptr, 0};
    if (!size) {
        return nullptr;
    }

    // Both the transfer buffer and static buffers are aligned to the max required alignment for
    // the pair of buffer types involved (transfer cpu->gpu and either index or vertex). Copies
    // must also copy an aligned amount of bytes.
    size = SkAlignTo(size, info->fAlignment);

    auto [transferMapPtr, transferBindInfo] =
            fUploadManager.makeBindInfo(size, fRequiredTransferAlignment);

    info->fData.push_back({transferBindInfo, target, size});
    info->fTotalRequiredBytes += size;
    return transferMapPtr;
}

bool StaticBufferManager::BufferInfo::createAndUpdateBindings(
        ResourceProvider* resourceProvider,
        Context* context,
        QueueManager* queueManager,
        GlobalCache* globalCache) const {
    if (!fTotalRequiredBytes) {
        return true; // No buffer needed
    }

    sk_sp<Buffer> staticBuffer = resourceProvider->findOrCreateBuffer(
            fTotalRequiredBytes, fBufferType, AccessPattern::kGpuOnly);
    if (!staticBuffer) {
        SKGPU_LOG_E("Failed to create static buffer for type %d of size %zu bytes.\n",
                    (int) fBufferType, fTotalRequiredBytes);
        return false;
    }

    size_t offset = 0;
    for (const CopyRange& data : fData) {
        // Each copy range's size should be aligned to the max of the required buffer alignment and
        // the transfer alignment, so we can just increment the offset into the static buffer.
        SkASSERT(offset % fAlignment == 0);
        data.fTarget->fBuffer = staticBuffer.get();
        data.fTarget->fOffset = offset;

        auto copyTask = CopyBufferToBufferTask::Make(
                data.fSource.fBuffer, data.fSource.fOffset,
                sk_ref_sp(data.fTarget->fBuffer), data.fTarget->fOffset,
                data.fSize);
        if (!queueManager->addTask(copyTask.get(), context)) {
            SKGPU_LOG_E("Failed to copy data to static buffer.\n");
            return false;
        }

        offset += data.fSize;
    }

    SkASSERT(offset == fTotalRequiredBytes);
    globalCache->addStaticResource(std::move(staticBuffer));
    return true;
}

StaticBufferManager::FinishResult StaticBufferManager::finalize(Context* context,
                                                                QueueManager* queueManager,
                                                                GlobalCache* globalCache) {
    const size_t totalRequiredBytes = fVertexBufferInfo.fTotalRequiredBytes +
                                      fIndexBufferInfo.fTotalRequiredBytes;
    SkASSERT(totalRequiredBytes <= kMaxStaticDataSize);
    if (!totalRequiredBytes) {
        return FinishResult::kNoWork;
    }

    if (!fVertexBufferInfo.createAndUpdateBindings(fResourceProvider, context,
                                                   queueManager, globalCache)) {
        return FinishResult::kFailure;
    }
    if (!fIndexBufferInfo.createAndUpdateBindings(fResourceProvider, context,
                                                  queueManager, globalCache)) {
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
