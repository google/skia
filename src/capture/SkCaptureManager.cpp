/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/capture/SkCaptureManager.h"

#include "include/core/SkCanvas.h"
#include "include/private/base/SkDebug.h"
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

void SkCaptureManager::serializeCapture() {
    // TODO (412351769): return a serialized file via SkData, for now this will print the contents
    // of the capture for inspection.
    SkDebugf("Tracked canvases: %d. SkPictures: %d\n", fTrackedCanvases.size(), fPictures.size());
}
