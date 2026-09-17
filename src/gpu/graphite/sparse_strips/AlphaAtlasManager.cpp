/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"

#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/task/UploadTask.h"

#include <algorithm>
#include <cstring>

namespace skgpu::graphite {

AlphaAtlasManager::AlphaAtlasManager(Recorder* recorder)
        : fRecorder(recorder)
        , fNullBufferUsedBytes(0)
        , fActiveSlot(0)
        , fLastGpuSlot(0)
        , fLastRetiredSlot(kInvalidSlot)
        , fNextPageRowCount(SparseStripConfig::kInitialAtlasRows) {}

bool AlphaAtlasManager::createPageInSlot(int32_t slot, int32_t minRequiredBytes) {
    SkASSERT(slot < SparseStripConfig::kMaxTexturePages);
    SkASSERT(!fPages[slot].isValid());

    int32_t rows = fNextPageRowCount;
    if (minRequiredBytes > 0) {
        int32_t neededRows = (minRequiredBytes + SparseStripConfig::kAtlasWidthBytes - 1) /
                             SparseStripConfig::kAtlasWidthBytes;
        if (neededRows > SparseStripConfig::kMaxAtlasRows) {
            return false;
        }
        rows = std::max(rows, neededRows);
    }
    rows = std::min(rows, SparseStripConfig::kMaxAtlasRows);

    const Caps* caps = fRecorder->priv().caps();
    TextureInfo info = caps->getDefaultSampledTextureInfo(SparseStripConfig::kColorType,
                                                          Mipmapped::kNo,
                                                          fRecorder->priv().isProtected(),
                                                          Renderable::kNo);

    sk_sp<TextureProxy> proxy =
            TextureProxy::Make(caps,
                               fRecorder->priv().resourceProvider(),
                               SkISize::Make(SparseStripConfig::kAtlasWidth, rows),
                               info,
                               "AlphaAtlas",
                               Budgeted::kYes);

    if (!proxy) {
        return false;
    }

    TexturePage& page = fPages[slot];
    page.fTexture = std::move(proxy);
    page.fRowCount = rows;
    page.fCapacityBytes = rows * SparseStripConfig::kAtlasWidthBytes;
    page.fUsedBytes = 0;
    page.fUploadedRows = 0;
    page.fAlphaBuffer.clear();
    page.fAlphaBuffer.reserve(page.fCapacityBytes);

    // Double row count for the next created page, clamped to max atlas rows
    fNextPageRowCount = std::min(SparseStripConfig::kMaxAtlasRows, rows * 2);
    return true;
}

uint8_t* AlphaAtlasManager::requestAlphaSpace(int32_t numBytes) {
    if (numBytes <= 0 || numBytes > SparseStripConfig::kMaxAtlasBytes) {
        return nullptr;
    }

    // 0. If already on the nullbuffer slot, append directly to it.
    if (fActiveSlot == kNullSlot) {
        return fNullBuffer.append(numBytes);
    }

    // 1. Ensure current active slot has a valid page allocated
    SkASSERT(fActiveSlot >= 0 && fActiveSlot < SparseStripConfig::kMaxTexturePages);
    if (!fPages[fActiveSlot].isValid()) {
        if (!this->createPageInSlot(fActiveSlot, numBytes)) {
            return nullptr;
        }
    }

    // 2. If the active page is already full, proactively flip to the other slot
    if (fPages[fActiveSlot].fUsedBytes >= fPages[fActiveSlot].fCapacityBytes) {
        int32_t nextSlot = fActiveSlot ^ 1;
        // Both pages are in use, transition to null buffer.
        if (fPages[nextSlot].isValid()) {
            fActiveSlot = kNullSlot;
            return fNullBuffer.append(numBytes);
        }
        if (!this->createPageInSlot(nextSlot, numBytes)) {
            return nullptr;
        }
        fActiveSlot = nextSlot;
    }

    TexturePage& activePage = fPages[fActiveSlot];
    fLastGpuSlot = fActiveSlot;
    return activePage.fAlphaBuffer.append(numBytes);
}

std::optional<std::pair<int32_t, uint16_t>> AlphaAtlasManager::finalizeRun() {
    // Case 0: We are already on the nullbuffer
    if (fActiveSlot == kNullSlot) {
        SkASSERT(fNullBuffer.size() > fNullBufferUsedBytes);
        int32_t runStartIdx = fNullBufferUsedBytes;
        fNullBufferUsedBytes = fNullBuffer.size();
        return std::make_pair(runStartIdx, kNullSlot);
    }

    TexturePage& activePage = fPages[fActiveSlot];
    if (!activePage.isValid()) {
        return std::nullopt;
    }

    SkASSERT(activePage.fAlphaBuffer.size() > activePage.fUsedBytes);
    int32_t runBytes = activePage.fAlphaBuffer.size() - activePage.fUsedBytes;

    // Case 1: Run completely fits in active page (no straddle)
    int32_t runStartIdx = activePage.fUsedBytes;
    if (activePage.fAlphaBuffer.size() <= activePage.fCapacityBytes) {
        activePage.fUsedBytes = activePage.fAlphaBuffer.size();
        return std::make_pair(runStartIdx, static_cast<uint16_t>(fActiveSlot));
    }

    // Case 2: Run straddled the page boundary!
    int32_t nextSlot = fActiveSlot ^ 1;
    if (fPages[nextSlot].isValid()) {
        // Both GPU slots are full! Switch to null buffer.
        // Currently this only occurs when all atlas pages have been exhausted, so fNullBuffer is
        // guaranteed to be empty at this point, making offset 0 (below) valid. If fNullBuffer is
        // ever reused or shared across draws in the future, this should return the appended run's
        // actual start offset (runStartIdx in fNullBuffer) instead.
        SkASSERT(fNullBuffer.empty());

        uint8_t* dst = fNullBuffer.append(runBytes);
        std::memcpy(dst,
                    activePage.fAlphaBuffer.data() + runStartIdx,
                    runBytes);

        // Trim overrun from activePage back to its committed size
        activePage.fAlphaBuffer.resize(activePage.fUsedBytes);

        // Switch active slot to kNullSlot
        fActiveSlot = kNullSlot;
        fNullBufferUsedBytes = fNullBuffer.size();

        return std::make_pair(0, kNullSlot);
    }

    if (!this->createPageInSlot(nextSlot, runBytes)) {
        return std::nullopt;
    }

    TexturePage& newPage = fPages[nextSlot];

    // Append runBytes to newPage and copy the overrun data
    uint8_t* dst = newPage.fAlphaBuffer.append(runBytes);
    std::memcpy(dst,
                activePage.fAlphaBuffer.data() + runStartIdx,
                runBytes);

    // Trim overrun from activePage back to its committed size
    activePage.fAlphaBuffer.resize(activePage.fUsedBytes);

    // Switch active slot to nextSlot
    fActiveSlot = nextSlot;
    newPage.fUsedBytes = runBytes;

    return std::make_pair(0, static_cast<uint16_t>(fActiveSlot));
}

void AlphaAtlasManager::recordUploads(DrawContext* dc) {
    for (int32_t i = 0; i < SparseStripConfig::kMaxTexturePages; ++i) {
        TexturePage& page = fPages[i];
        if (!page.isValid()) {
            continue;
        }
        int32_t startRow = page.fUploadedRows;
        int32_t totalUsedRows =
                (page.fUsedBytes + SparseStripConfig::kAtlasWidthBytes - 1) /
                SparseStripConfig::kAtlasWidthBytes;
        int32_t pendingRows = totalUsedRows - startRow;
        if (pendingRows > 0) {
            SkASSERT(page.fTexture);

            int32_t paddedSize = totalUsedRows * SparseStripConfig::kAtlasWidthBytes;
            if (page.fAlphaBuffer.size() < paddedSize) {
                int32_t diff = paddedSize - page.fAlphaBuffer.size();
                uint8_t* ptr = page.fAlphaBuffer.append(diff);
                std::memset(ptr, 0, diff);
            }

            MipLevel level;
            level.fPixels =
                    page.fAlphaBuffer.data() + (startRow * SparseStripConfig::kAtlasWidthBytes);
            level.fRowBytes = SparseStripConfig::kAtlasWidthBytes;
            SkIRect dstRect =
                    SkIRect::MakeXYWH(0, startRow, SparseStripConfig::kAtlasWidth, pendingRows);

            Swizzle readSwizzle = ReadSwizzleForColorType(
                    SparseStripConfig::kColorType,
                    TextureInfoPriv::ViewFormat(page.fTexture->textureInfo()));
            TextureProxyView proxyView(page.fTexture, readSwizzle);

            SkColorInfo srcColorInfo(SparseStripConfig::kColorType, kPremul_SkAlphaType, nullptr);
            SkColorInfo dstColorInfo(SparseStripConfig::kColorType, kPremul_SkAlphaType, nullptr);
            UploadSource source = UploadSource::Make(fRecorder->priv().caps(),
                                                     proxyView,
                                                     srcColorInfo,
                                                     dstColorInfo,
                                                     SkSpan<const MipLevel>(&level, 1),
                                                     dstRect);

            if (dc) {
                dc->recordUpload(fRecorder, source, nullptr);
            }

            page.fUploadedRows = totalUsedRows;
            page.fUsedBytes = paddedSize;
        }
    }

    // At flush time, retire the older GPU slot if both were valid.
    if (fPages[0].isValid() && fPages[1].isValid()) {
        // If we're on the null texture, the active slot is instead the last gpu slot.
        int32_t activeGpuSlot =
                (fActiveSlot < SparseStripConfig::kMaxTexturePages) ? fActiveSlot : fLastGpuSlot;
        int32_t oldSlot = activeGpuSlot ^ 1;
        SkASSERT(fPages[oldSlot].isValid());
        fPages[oldSlot].reset();
        fLastRetiredSlot = oldSlot;
    }
}

bool AlphaAtlasManager::resolveNullCaps(EndCaps* ends) {
    SkASSERT(ends && ends->hasNullCaps() && fNullBuffer.size() > 0);
    int32_t firstNull = ends->firstNullCapIndex();
    auto& caps = ends->caps();
    SkASSERT(firstNull >= 0 && firstNull < caps.size());

    // Use the slot that was freed at flush time
    if (fLastRetiredSlot == kInvalidSlot) {
        return false;
    }
    SkASSERT(!fPages[fLastRetiredSlot].isValid());
    int32_t currentSlot = fLastRetiredSlot;

    // Allocate currentSlot up to max capacity
    int32_t nullBytes = fNullBuffer.size() - caps[firstNull].fAlphaIndex;
    if (!this->createPageInSlot(currentSlot,
                                std::min(nullBytes, SparseStripConfig::kMaxAtlasBytes))) {
        return false;
    }
    fLastRetiredSlot = kInvalidSlot;

    // Copy the data from the null buffer into the new texture pages.
    int32_t nextNull = firstNull;
    int32_t roundStartCap = firstNull;
    for (; nextNull < caps.size(); ++nextNull) {
        int32_t alphaIndex = caps[nextNull].fAlphaIndex;
        int32_t capBytes = caps[nextNull].fWidth * SparseStripConfig::kTileHeight;
        SkASSERT(capBytes > 0);

        // If the run exceeds the current slot, try to instantiate the other page
        if (fPages[currentSlot].fUsedBytes + capBytes > fPages[currentSlot].fCapacityBytes) {
            int32_t otherSlot = currentSlot ^ 1;
            if (!fPages[otherSlot].isValid() || fPages[otherSlot].fUsedBytes == 0) {
                if (fPages[otherSlot].isValid()) {
                    fPages[otherSlot].reset();
                }
                int32_t remainingBytes = fNullBuffer.size() - alphaIndex;
                if (!this->createPageInSlot(
                            otherSlot,
                            std::min(remainingBytes, SparseStripConfig::kMaxAtlasBytes))) {
                    break;
                }
                if (fPages[otherSlot].fUsedBytes + capBytes >
                    fPages[otherSlot].fCapacityBytes) {
                    fPages[otherSlot].reset();
                    break;
                }
                currentSlot = otherSlot;
            } else {
                // Both slots have been filled for this round
                break;
            }
        }

        TexturePage& page = fPages[currentSlot];
        uint8_t* dst = page.fAlphaBuffer.append(capBytes);
        std::memcpy(dst, fNullBuffer.data() + alphaIndex, capBytes);

        caps[nextNull].fTexPage = static_cast<uint16_t>(currentSlot);
        caps[nextNull].fAlphaIndex = page.fUsedBytes;
        page.fUsedBytes += capBytes;
    }

    if (nextNull == roundStartCap) {
        // Made zero progress
        return false;
    }

    ends->setDrawStartIndex(roundStartCap);
    if (nextNull == caps.size()) {
        // We're done, no more rounds of flushing are necessary.
        ends->clearNullCaps();
        fNullBuffer.clear();
        fNullBufferUsedBytes = 0;
    } else {
        ends->setFirstNullCapIndex(nextNull);
    }

    this->populateProxies(ends);

    fActiveSlot = currentSlot;
    fLastGpuSlot = currentSlot;
    return true;
}

void AlphaAtlasManager::populateProxies(EndCaps* ends) const {
    int32_t startSlot = 0;
    while (startSlot < SparseStripConfig::kMaxTexturePages && !fPages[startSlot].isValid()) {
        startSlot++;
    }
    SkASSERT(startSlot < SparseStripConfig::kMaxTexturePages);

    sk_sp<TextureProxy> lastValid = fPages[startSlot].fTexture;
    for (int32_t i = 0; i < SparseStripConfig::kMaxTexturePages; ++i) {
        int32_t slot = (startSlot + i) % SparseStripConfig::kMaxTexturePages;
        if (fPages[slot].isValid()) {
            lastValid = fPages[slot].fTexture;
        }
        ends->setProxy(slot, lastValid);
    }
}

void AlphaAtlasManager::freeGpuResources() {
    for (int32_t i = 0; i < SparseStripConfig::kMaxTexturePages; ++i) {
        fPages[i].reset();
    }
    fNullBuffer.clear();
    fNullBufferUsedBytes = 0;
    fActiveSlot = 0;
    fLastGpuSlot = 0;
    fLastRetiredSlot = kInvalidSlot;
    fNextPageRowCount = SparseStripConfig::kInitialAtlasRows;
}

}  // namespace skgpu::graphite
