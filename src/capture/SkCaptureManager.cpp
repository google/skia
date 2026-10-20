/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/capture/SkCaptureManager.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "src/capture/SkCapture.h"
#include "src/capture/SkCaptureAsset.h"
#include "src/capture/SkCaptureCanvas.h"
#include "src/image/SkSurface_Base.h"

#include <algorithm>
#include <memory>

SkCaptureManager::SkCaptureManager() {}

SkCanvas* SkCaptureManager::makeCaptureCanvas(SkCanvas* canvas) {
    auto newCanvas = std::make_unique<SkCaptureCanvas>(canvas, this);
    auto rawCanvasPtr = newCanvas.get();
    fTrackedCanvases.emplace_back(std::move(newCanvas));
    return rawCanvasPtr;
}

void SkCaptureManager::deregisterCaptureCanvas(SkCanvas* canvas) {
    if (!canvas) {
        return;
    }
    for (int i = 0; i < fTrackedCanvases.size(); ++i) {
        if (fTrackedCanvases[i].get() != canvas) { continue; }

        // Make sure we gather any draws that haven't been inserted into a recording
        // before removing the canvas.
        if (fIsCurrentlyCapturing) {
            auto asset = this->snapAndIncrement(fTrackedCanvases[i].get());
            if (asset && asset->fPicture && fActiveCapture) {
                // Temporary staging shim: SkCapture stores assets as a flat list and identifies
                // DrawTasks by flat asset index. In the followup CL, SkCapture will transition to a
                // (storageID, contentID) map and DrawTask will store {fPixelStorageID, fContentID}
                fActiveCapture->addAsset(asset->fPicture);
            }
        }
        fTrackedCanvases.removeShuffle(i);
        return;
    }
}

sk_sp<SkCaptureAsset> SkCaptureManager::snapAndIncrement(SkCaptureCanvas* canvas) {
    auto asset = sk_make_sp<SkCaptureAsset>();
    asset->fPicture = canvas->snapPicture();
    if (asset->fPicture) {
        if (SkSurface* surface = canvas->getBaseCanvasSurface()) {
            if (sk_sp<SkPixelStorage> storage = asSB(surface)->getPixelStorage()) {
                asset->fPixelStorageID = storage->getPixelStorageId();
                asset->fContentID = storage->incrementContentId();
            }
        }
    }
    return asset;
}

void SkCaptureManager::captureUninsertedDrawTasks() {
    for (auto& canvas : fTrackedCanvases) {
        if (canvas) {
            sk_sp<SkCaptureAsset> asset = this->snapAndIncrement(canvas.get());
            if (asset && asset->fPicture && fActiveCapture) {
                // Temporary staging shim: SkCapture stores assets as a flat list and identifies
                // DrawTasks by flat asset index. In the followup CL, SkCapture will transition to a
                // (storageID, contentID) map and DrawTask will store {fPixelStorageID, fContentID}
                fActiveCapture->addAsset(asset->fPicture);
            }
        }
    }
}

// TODO: make thread safe by using exchange() and a mutex.
void SkCaptureManager::toggleCapture(bool capturing) {
    if (capturing != fIsCurrentlyCapturing) {
        if (capturing) {
            fActiveCapture = SkCapture::MakeEmpty();
        } else {
            // on capture stop, save the capture and reset
            this->captureUninsertedDrawTasks();
            fLastCapture = std::move(fActiveCapture);
        }
    }
    fIsCurrentlyCapturing = capturing;
}

sk_sp<SkCaptureAsset> SkCaptureManager::snapPicture(SkSurface* surface) {
    for (auto& canvas : fTrackedCanvases) {
        if (canvas) {
            if (canvas->getBaseCanvasSurface() == surface) {
                return this->snapAndIncrement(canvas.get());
            }
        }
    }
    return nullptr;
}


sk_sp<SkCapture> SkCaptureManager::getLastCapture() const {
   return fLastCapture;
}

skia_private::TArray<sk_sp<SkCaptureAsset>> SkCaptureManager::snapDrawTasksForStorageIDs(
        SkSpan<const uint32_t> storageIds) {
    skia_private::TArray<sk_sp<SkCaptureAsset>> snapped;
    if (!fIsCurrentlyCapturing || storageIds.empty()) {
        return snapped;
    }

    for (auto& canvas : fTrackedCanvases) {
        if (!canvas) {
            continue;
        }
        SkSurface* surface = canvas->getBaseCanvasSurface();
        if (!surface) {
            continue;
        }
        auto storage = asSB(surface)->getPixelStorage();
        if (!storage) {
            continue;
        }
        uint32_t id = storage->getPixelStorageId();
        if (std::find(storageIds.begin(), storageIds.end(), id) != storageIds.end()) {
            auto asset = this->snapAndIncrement(canvas.get());
            if (asset && asset->fPicture) {
                snapped.push_back(std::move(asset));
            }
        }
    }
    return snapped;
}

void SkCaptureManager::onInsertRecording(const skia_private::TArray<sk_sp<SkCaptureAsset>>&
    capturedPictures) {
    if (!fIsCurrentlyCapturing || !fActiveCapture) return;

    skia_private::TArray<SkCapture::DrawTask> drawTasks;
    for (const auto& asset : capturedPictures) {
        if (!asset || !asset->fPicture) continue;
        // Temporary staging shim: SkCapture still stores assets as a flat list and identifies
        // DrawTasks by flat asset index. In the followup CL, SkCapture will transition to a
        // (storageID, contentID) map and DrawTask will store {fPixelStorageID, fContentID}
        fActiveCapture->addAsset(asset->fPicture);
        uint32_t assetIdx = fActiveCapture->getMetadata().numAssets - 1;
        drawTasks.push_back({assetIdx});
    }

    SkCapture::RecordingCapture rec = {
        std::move(drawTasks)
    };

    fActiveCapture->addRecordingCapture(std::move(rec));
}
