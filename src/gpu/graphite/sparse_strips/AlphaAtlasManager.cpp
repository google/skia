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

namespace skgpu::graphite {

AlphaAtlasManager::AlphaAtlasManager(Recorder* recorder)
        : fRecorder(recorder)
        , fActiveSlot(0)
        , fNextPageRowCount(SparseStripConfig::kInitialAtlasRows) {}

bool AlphaAtlasManager::createPageInSlot(int slot, int32_t minRequiredBytes) {
    SkASSERT(slot < SparseStripConfig::kMaxTexturePages);
    SkASSERT(!fPages[slot].isValid());

    int32_t rows = fNextPageRowCount;
    if (minRequiredBytes > 0) {
        int32_t neededRows = (minRequiredBytes + SparseStripConfig::kAtlasWidthBytes - 1) /
                             SparseStripConfig::kAtlasWidthBytes;
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

    // Double row count for the next created page, clamped to max atlas rows
    fNextPageRowCount = std::min(SparseStripConfig::kMaxAtlasRows, rows * 2);
    return true;
}

std::optional<AlphaAtlasManager::AlphaAllocation> AlphaAtlasManager::requestAlphaSpace(
        int32_t numBytes) {
    if (numBytes <= 0 || numBytes > SparseStripConfig::kMaxCapBytes) {
        return std::nullopt;
    }

    // 1. Ensure current active slot has a valid page allocated
    if (!fPages[fActiveSlot].isValid()) {
        if (!this->createPageInSlot(fActiveSlot, numBytes)) {
            return std::nullopt;
        }
    }

    TexturePage* activePage = &fPages[fActiveSlot];

    // 2. Check if the active slot has enough remaining room.
    // An endcap's alphas must be atomic to a single texture page (never split across pages).
    if (activePage->fUsedBytes + numBytes <= activePage->fCapacityBytes) {
        int32_t alphaIdx = activePage->fUsedBytes;
        activePage->fUsedBytes += numBytes;
        uint8_t* ptr = activePage->fAlphaBuffer.append(numBytes);
        return AlphaAllocation{ptr, alphaIdx, static_cast<uint16_t>(fActiveSlot)};
    }

    // 3. Current active slot does not have enough room.
    // Flip-flop to the other slot: nextSlot = fActiveSlot ^ 1
    int nextSlot = fActiveSlot ^ 1;

    // If nextSlot is already in use, both slots are full for this batch!
    if (fPages[nextSlot].isValid()) {
        return std::nullopt;
    }

    if (!this->createPageInSlot(nextSlot, numBytes)) {
        return std::nullopt;
    }

    fActiveSlot = nextSlot;
    activePage = &fPages[fActiveSlot];

    int32_t alphaIdx = 0;
    activePage->fUsedBytes = numBytes;
    uint8_t* ptr = activePage->fAlphaBuffer.append(numBytes);
    return AlphaAllocation{ptr, alphaIdx, static_cast<uint16_t>(fActiveSlot)};
}

void AlphaAtlasManager::recordUploads(DrawContext* dc) {
    for (int i = 0; i < SparseStripConfig::kMaxTexturePages; ++i) {
        TexturePage& page = fPages[i];
        if (page.isValid() && !page.fAlphaBuffer.empty()) {
            SkASSERT(page.fTexture);

            // Compute how many rows are in the pending buffer and pad buffer to full rows
            int32_t pendingRows =
                    (page.fAlphaBuffer.size() + SparseStripConfig::kAtlasWidthBytes - 1) /
                    SparseStripConfig::kAtlasWidthBytes;
            int32_t paddedSize = pendingRows * SparseStripConfig::kAtlasWidthBytes;
            if (page.fAlphaBuffer.size() < paddedSize) {
                int32_t diff = paddedSize - page.fAlphaBuffer.size();
                uint8_t* ptr = page.fAlphaBuffer.append(diff);
                std::memset(ptr, 0, diff);
            }

            MipLevel level;
            level.fPixels = page.fAlphaBuffer.data();
            level.fRowBytes = SparseStripConfig::kAtlasWidthBytes;
            int32_t startRow = page.fUploadedRows;
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

            page.fUploadedRows += pendingRows;
            page.fUsedBytes = page.fUploadedRows * SparseStripConfig::kAtlasWidthBytes;
            page.fAlphaBuffer.clear();
        }
    }

    // At flush time, retire the older slot (fActiveSlot ^ 1).
    // Graphite's resource management keeps it alive for any pending GPU tasks.
    int oldSlot = fActiveSlot ^ 1;
    if (fPages[oldSlot].isValid()) {
        fPages[oldSlot].reset();
    }
}

void AlphaAtlasManager::freeGpuResources() {
    fPages[0].reset();
    fPages[1].reset();
    fActiveSlot = 0;
    fNextPageRowCount = SparseStripConfig::kInitialAtlasRows;
}

}  // namespace skgpu::graphite
