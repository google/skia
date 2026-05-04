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

SkContentID SkCaptureManager::processCanvasContent(SkCaptureCanvas* canvas) {
    auto picture = canvas->snapPicture();
    if (picture) {
        uint32_t surfaceID = asSB(canvas->getBaseCanvasSurface())->getPixelStorageID();
        SkContentID contentID = fSurfaceContentCounters[surfaceID];
        contentID++;
        fPictures.emplace_back(picture);
        return contentID;
    }
    return SkContentID();
}

void SkCaptureManager::snapPictures() {
    for (auto& canvas : fTrackedCanvases) {
        if (canvas) {
            processCanvasContent(canvas.get());
        }
    }
}

// TODO: make thread safe by using exchange() and a mutex.
void SkCaptureManager::toggleCapture(bool capturing) {
    if (capturing != fIsCurrentlyCapturing && !capturing) {
        // on capture stop, save the capture and reset
        this->snapPictures();
        fLastCapture = SkCapture::MakeFromPictures(fPictures);
        fPictures.clear();
        fSurfaceContentCounters.clear();
    }
    fIsCurrentlyCapturing = capturing;
}

SkContentID SkCaptureManager::snapPicture(SkSurface* surface) {
    for (auto& canvas : fTrackedCanvases) {
        if (canvas) {
            if (canvas->getBaseCanvasSurface() == surface) {
                return processCanvasContent(canvas.get());
            }
        }
    }
    return SkContentID();
}

sk_sp<SkCapture> SkCaptureManager::getLastCapture() const {
   return fLastCapture;
}
