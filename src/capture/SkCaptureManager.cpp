/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/capture/SkCaptureManager.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "src/capture/SkCapture.h"
#include "src/capture/SkCaptureCanvas.h"

#include <memory>

SkCaptureManager::SkCaptureManager() {}

SkCanvas* SkCaptureManager::makeCaptureCanvas(SkCanvas* canvas) {
    auto newCanvas = std::make_unique<SkCaptureCanvas>(canvas, this);
    auto rawCanvasPtr = newCanvas.get();
    fTrackedCanvases.emplace_back(std::move(newCanvas));
    return rawCanvasPtr;
}

void SkCaptureManager::snapPictures() {
    for (auto& canvas : fTrackedCanvases) {
        if (canvas) {
            auto picture = canvas->snapPicture();
            if (picture) {
                fPictures.emplace_back(picture);
            }
        }
    }
}

// TODO: make thread saffe by using exchange() and a mutex.
void SkCaptureManager::toggleCapture(bool capturing) {
    if (capturing != fIsCurrentlyCapturing && !capturing) {
        // on capture stop, save the capture and reset
        this->snapPictures();
        fLastCapture = SkCapture::MakeFromPictures(fPictures);
        fPictures.clear();
    }
    fIsCurrentlyCapturing = capturing;
}

void SkCaptureManager::snapPicture(SkSurface* surface) {
    for (auto& canvas : fTrackedCanvases) {
        if (canvas) {
            if (canvas->getSurface() == surface) {
                auto picture = canvas->snapPicture();
                if (picture) {
                    // TODO(412351769): for every storing of a picture, we should track a content id
                    // and the surface it was drawn to.
                    fPictures.emplace_back(picture);
                }
                return;
            }
        }
    }
}

sk_sp<SkCapture> SkCaptureManager::getLastCapture() const {
   return fLastCapture;
}
