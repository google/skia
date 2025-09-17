/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCaptureManager_DEFINED
#define SkCaptureManager_DEFINED

#include "include/core/SkPicture.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkTArray.h"

#include <atomic>

class SkCanvas;
class SkCaptureCanvas;
class SkSurface;

class SkCaptureManager : public SkRefCnt {
public:
    SkCaptureManager();

    SkCanvas* makeCaptureCanvas(SkCanvas* canvas);
    void snapPictures();
    void snapPicture(SkSurface*);
    void serializeCapture();

    void toggleCapture(bool capturing) {
        if (capturing != fIsCurrentlyCapturing && !capturing) {
            this->snapPictures();
        }
        fIsCurrentlyCapturing = capturing;
    }

    bool isCurrentlyCapturing() const {
        return fIsCurrentlyCapturing;
    }

private:
    std::atomic<bool> fIsCurrentlyCapturing = false;
    skia_private::TArray<std::unique_ptr<SkCaptureCanvas>> fTrackedCanvases;
    skia_private::TArray<sk_sp<SkPicture>>  fPictures;
};

#endif  // SkCaptureManager_DEFINED
