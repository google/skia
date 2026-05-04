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
#include <cstdint>
#include <map>

class SkCanvas;
class SkCapture;
class SkCaptureCanvas;
class SkSurface;

class SkContentID {
    public:
        SkContentID() = default;

        explicit SkContentID(uint32_t id) : fId(id) {}

        uint32_t asUInt() const { return fId; }

        bool operator==(const SkContentID& other) const { return fId == other.fId; }
        bool operator!=(const SkContentID& other) const { return !(*this == other); }
        operator bool() const { return fId != 0; }

        SkContentID operator++() {
            ++fId;
            return *this;
        }

        SkContentID operator++(int) {
            SkContentID temp = *this;
            ++fId;
            return temp;
        }

    private:
        uint32_t fId = 0;
    };

/**
 * SkCaptureManager is in charge of knowing the current state of capture, handling the creation of
 * capture canvases, and tracking and recording metadata to the final SkCapture.
 */
class SkCaptureManager : public SkRefCnt {
public:
    SkCaptureManager();

    SkCanvas* makeCaptureCanvas(SkCanvas* canvas);
    void snapPictures();
    SkContentID snapPicture(SkSurface*);

    void toggleCapture(bool capturing);

    bool isCurrentlyCapturing() const {
        return fIsCurrentlyCapturing;
    }

    sk_sp<SkCapture> getLastCapture() const;

private:
    SkContentID processCanvasContent(SkCaptureCanvas*);

    std::atomic<bool> fIsCurrentlyCapturing = false;
    skia_private::TArray<std::unique_ptr<SkCaptureCanvas>> fTrackedCanvases;
    skia_private::TArray<sk_sp<SkPicture>>  fPictures;

    sk_sp<SkCapture> fLastCapture;
    std::map<uint32_t, SkContentID> fSurfaceContentCounters;
};

#endif  // SkCaptureManager_DEFINED
