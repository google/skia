/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCapture_DEFINED
#define SkCapture_DEFINED

#include "include/core/SkAlphaType.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSerialProcs.h"
#include "include/private/SkTArray.h"

#include <optional>

class SkData;
class SkImage;
class SkPicture;

class SkCanvas;
class SkCaptureCanvas;

/**
 * Binary Serialization Layout (.capt):
 *
 * +-----------------------------------------------------------------------------+
 * | Magic Bytes 1 ('skia')                     | 4 Bytes (uint32_t)             |
 * +-----------------------------------------------------------------------------+
 * | Magic Bytes 2 ('capt')                     | 4 Bytes (uint32_t)             |
 * +-----------------------------------------------------------------------------+
 * | Version (kVersion)                         | 4 Bytes (uint32_t)             |
 * +-----------------------------------------------------------------------------+
 * | Asset Count (numAssets)                    | 4 Bytes (uint32_t)             |
 * +-----------------------------------------------------------------------------+
 * | RecordingCapture Count (numRecCaptures)    | 4 Bytes (uint32_t)             |
 * +=============================================================================+
 * |                       ASSETS SECTION (N = numAssets)                        |
 * +-----------------------------------------------------------------------------+
 * | Picture 0 Data Size                        | 4 Bytes (uint32_t)             |
 * +-----------------------------------------------------------------------------+
 * | Picture 0 Bytes                            | [Picture 0 Size] bytes         |
 * +-----------------------------------------------------------------------------+
 * | ...                                        |                                |
 * +-----------------------------------------------------------------------------+
 * | Picture N Data Size                        | 4 Bytes (uint32_t)             |
 * +-----------------------------------------------------------------------------+
 * | Picture N Bytes                            | [Picture N Size] bytes         |
 * +=============================================================================+
 * |                     TIMELINE SECTION (K = numRecCaptures)                   |
 * +-----------------------------------------------------------------------------+
 * | Recording 0 Task Count (taskCount)         | 4 Bytes (uint32_t)             |
 * + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
 * | Recording 0 Tasks (fAssetIndex)            | taskCount * sizeof(DrawTask)   |
 * +-----------------------------------------------------------------------------+
 * | ...                                        |                                |
 * +-----------------------------------------------------------------------------+
 * | Recording K Task Count (taskCount)         | 4 Bytes (uint32_t)             |
 * + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
 * | Recording K Tasks (fAssetIndex)            | taskCount * sizeof(DrawTask)   |
 * +-----------------------------------------------------------------------------+
 */

class SkCapture : public SkRefCnt {
public:
    struct DrawTask {
        uint32_t fAssetIndex;
    };

    struct RecordingCapture {
        skia_private::TArray<DrawTask> fDrawTasks;
    };

    struct Metadata {
        uint32_t version;
        uint32_t numAssets;
        uint32_t numRecordingCaptures;
    };

    static sk_sp<SkCapture> MakeFromData(sk_sp<const SkData>);
    static sk_sp<SkCapture> MakeEmpty();

    void addAsset(sk_sp<SkPicture>);
    void addRecordingCapture(RecordingCapture);

    sk_sp<SkData> serializeCapture();

    sk_sp<SkPicture> getAsset(int i) const;
    const RecordingCapture* getRecordingCapture(int i) const;
    Metadata getMetadata() const;

private:
    static SkSerialReturnType serializeImageProc(SkImage* img, void* ctx);
    static sk_sp<SkImage> deserializeImageProc(sk_sp<SkData>,
                                               std::optional<SkAlphaType>, void* ctx);

    Metadata fMetadata;
    skia_private::TArray<sk_sp<SkPicture>> fAssets;
    skia_private::TArray<RecordingCapture> fTimeline;

    static const uint32_t kVersion = 0;
};

#endif //SkCapture_DEFINED
