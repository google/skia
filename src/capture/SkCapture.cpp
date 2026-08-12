/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/capture/SkCapture.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkFourByteTag.h"
#include "include/core/SkImage.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkStream.h"
#include "include/private/SkLog.h"
#include "include/private/SkTArray.h"
#include "src/capture/SkCaptureManager.h"

constexpr SkFourByteTag kMagic1  = SkSetFourByteTag('s','k','i','a');
constexpr SkFourByteTag kMagic2  = SkSetFourByteTag('c','a','p','t');

sk_sp<SkCapture> SkCapture::MakeFromData(sk_sp<const SkData> data) {
    if (!data) {
        return nullptr;
    }

    // 1. Setup Stream
    SkMemoryStream stream(data->data(), data->size());

    // 2. Read and Validate Magic Number
    uint32_t magic1;
    uint32_t magic2;
    if (!stream.readU32(&magic1) || !stream.readU32(&magic2) ||
        magic1 != kMagic1 || magic2 != kMagic2) {
        SKIA_LOG_E("Invalid magic number for SkCapture.");
        return nullptr;
    }

    // 3. Read and Validate Version
    uint32_t version;
    if (!stream.readU32(&version) || version != kVersion) {
        SKIA_LOG_E("Unsupported SkCapture version: %u.", version);
        return nullptr;
    }

    // 4. Read Asset and RecordingCapture Counts
    uint32_t assetCount;
    uint32_t recordingCaptureCount;
    if (!stream.readU32(&assetCount) || !stream.readU32(&recordingCaptureCount)) {
        SKIA_LOG_E("Failed to read asset or recording capture counts.");
        return nullptr;
    }

    auto capture = sk_make_sp<SkCapture>();
    capture->fMetadata = {version, assetCount, recordingCaptureCount};

    // 5. Loop and Deserialize Each Asset (SkPicture)
    for (uint32_t i = 0; i < assetCount; ++i) {
        uint32_t pictureDataSize;
        if (!stream.readU32(&pictureDataSize)) {
            SKIA_LOG_E("Failed to read picture data size for asset %u.", i);
            return nullptr;
        }

        sk_sp<SkData> pictureData = SkData::MakeUninitialized(pictureDataSize);
        if (!pictureData || stream.read(pictureData->writable_data(),
                                        pictureDataSize) != pictureDataSize) {
            SKIA_LOG_E("Failed to read picture data for asset %u or allocation failed.", i);
            return nullptr;
        }

        SkDeserialProcs procs;
        procs.fImageDataProc = SkCapture::deserializeImageProc;
        sk_sp<SkPicture> picture = SkPicture::MakeFromData(pictureData.get(), &procs);
        if (!picture) {
            SKIA_LOG_E("Failed to deserialize SkPicture for asset %u.", i);
            return nullptr;
        }

        capture->fAssets.emplace_back(std::move(picture));
    }

    // 6. Loop and Deserialize Each RecordingCapture
    for (uint32_t i = 0; i < recordingCaptureCount; ++i) {
        RecordingCapture rec;
        uint32_t taskCount;
        if (!stream.readU32(&taskCount)) {
            SKIA_LOG_E("Failed to read task count for recording capture %u.", i);
            return nullptr;
        }
        for (uint32_t j = 0; j < taskCount; ++j) {
            uint32_t assetIdx;
            if (!stream.readU32(&assetIdx)) {
                SKIA_LOG_E("Failed to read task %u for recording capture %u.", j, i);
                return nullptr;
            }
            if (assetIdx >= static_cast<uint32_t>(capture->fAssets.size())) {
                SKIA_LOG_E("Out-of-bounds asset index %u parsed for task %u inside recording %u.",
                           assetIdx, j, i);
                return nullptr;
            }
            rec.fDrawTasks.push_back({assetIdx});
        }
        capture->fTimeline.push_back(std::move(rec));
    }

    SKIA_LOG_I("Successfully read %d assets and %d recording captures into SkCapture.",
               (int)capture->fAssets.size(), (int)capture->fTimeline.size());
    return capture;
}

sk_sp<SkCapture> SkCapture::MakeEmpty() {
    auto capture = sk_make_sp<SkCapture>();
    capture->fMetadata = {SkCapture::kVersion, 0, 0};
    return capture;
}

void SkCapture::addAsset(sk_sp<SkPicture> picture) {
    if (picture) {
        fAssets.push_back(std::move(picture));
        fMetadata.numAssets = fAssets.size();
    }
}

void SkCapture::addRecordingCapture(RecordingCapture rec) {
    fTimeline.push_back(std::move(rec));
    fMetadata.numRecordingCaptures = fTimeline.size();
}

sk_sp<SkPicture> SkCapture::getAsset(int i) const {
    if (i >= 0 && i < fAssets.size()) {
        return fAssets[i];
    }
    return nullptr;
}

const SkCapture::RecordingCapture* SkCapture::getRecordingCapture(int i) const {
    if (i >= 0 && i < fTimeline.size()) {
        return &fTimeline[i];
    }
    return nullptr;
}

SkCapture::Metadata SkCapture::getMetadata() const {
    return fMetadata;
}

sk_sp<SkData> SkCapture::serializeCapture() {
    SkDynamicMemoryWStream stream;

    stream.write32(kMagic1);
    stream.write32(kMagic2);
    stream.write32(SkCapture::kVersion);

    // Number of assets and recording captures
    stream.write32(fAssets.size());
    stream.write32(fTimeline.size());

    // 1. Assets Section: Serialized Pictures
    for (const auto& picture : fAssets) {
        SkDynamicMemoryWStream pictureStream;
        SkSerialProcs procs;
        procs.fImageProc = SkCapture::serializeImageProc;
        picture->serialize(&pictureStream, &procs);
        sk_sp<SkData> pictureData = pictureStream.detachAsData();

        // Write size and then data
        stream.write32(pictureData->size());
        stream.write(pictureData->data(), pictureData->size());
    }

    // 2. Timeline Section: RecordingCaptures
    for (const auto& rec : fTimeline) {
        // Write DrawTasks count
        stream.write32(rec.fDrawTasks.size());
        for (const auto& task : rec.fDrawTasks) {
            stream.write32(task.fAssetIndex);
        }
    }

    auto data = stream.detachAsData();
    SKIA_LOG_I("Wrote %d assets and %d recording captures to SkData block.",
               (int)fAssets.size(), (int)fTimeline.size());
    return data;
}

// TODO(b/412351769): When serializing our SkPictures, the images that are drawn through drawImage
// and similar functions will also need to be serialized. Instead of naively encoding all images as
// PNGs, we want the images that refer to content created from an SkSurface to point to its
// corresponding SkPicture. This means that we need to create a context for the proc to track these
// contentIDs.
SkSerialReturnType SkCapture::serializeImageProc(SkImage* img, void* ctx) {
    const int contentID = -1; // TODO: replace with real content ID.
    return SkData::MakeWithCopy(&contentID, sizeof(int));
}

sk_sp<SkImage> SkCapture::deserializeImageProc(sk_sp<SkData>, std::optional<SkAlphaType>, void*) {
    // TODO: set up the SkCapture context and inspect it to grab SkPictures and pass them as images.
    SkBitmap b;
    b.allocN32Pixels(5, 5);
    SkCanvas canvas(b);
    canvas.drawColor(SK_ColorMAGENTA);
    return b.asImage();
}
