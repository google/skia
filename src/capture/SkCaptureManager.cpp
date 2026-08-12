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
#include "src/capture/SkCaptureCanvas.h"
#include "src/image/SkSurface_Base.h"

#include <memory>

SkCaptureManager::SkCaptureManager() {}

SkCanvas* SkCaptureManager::makeCaptureCanvas(SkCanvas* canvas) {
    auto newCanvas = std::make_unique<SkCaptureCanvas>(canvas, this);
    auto rawCanvasPtr = newCanvas.get();
    fTrackedCanvases.emplace_back(std::move(newCanvas));
    return rawCanvasPtr;
}

sk_sp<SkPicture> SkCaptureManager::snapAndIncrement(SkCaptureCanvas* canvas) {
    auto picture = canvas->snapPicture();
    if (picture) {
        if (auto storage = asSB(canvas->getBaseCanvasSurface())->getPixelStorage()) {
            storage->incrementContentId();
        }
    }
    return picture;
}

void SkCaptureManager::captureUninsertedDrawTasks() {
    for (auto& canvas : fTrackedCanvases) {
        if (canvas) {
            auto picture = this->snapAndIncrement(canvas.get());
            if (picture && fActiveCapture) {
                fActiveCapture->addAsset(std::move(picture));
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

sk_sp<SkPicture> SkCaptureManager::snapPicture(SkSurface* surface) {
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

skia_private::TArray<sk_sp<SkPicture>> SkCaptureManager::captureDrawTasksForRecording() {
    skia_private::TArray<sk_sp<SkPicture>> snapped;
    if (!fIsCurrentlyCapturing) {
        return snapped;
    }

    for (auto& canvas : fTrackedCanvases) {
        if (canvas) {
            auto picture = this->snapAndIncrement(canvas.get());
            if (picture) {
                snapped.push_back(std::move(picture));
            }
        }
    }
    return snapped;
}

void SkCaptureManager::onInsertRecording(const skia_private::TArray<sk_sp<SkPicture>>& capturedPictures) {
    if (!fIsCurrentlyCapturing || !fActiveCapture) return;

    skia_private::TArray<SkCapture::DrawTask> drawTasks;
    for (const auto& pic : capturedPictures) {
        fActiveCapture->addAsset(pic);
        uint32_t assetIdx = fActiveCapture->getMetadata().numAssets - 1;
        drawTasks.push_back({assetIdx});
    }

    SkCapture::RecordingCapture rec = {
        std::move(drawTasks)
    };

    fActiveCapture->addRecordingCapture(std::move(rec));
}
