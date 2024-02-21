/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/codec/SkPngDecoder.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/encode/SkPngEncoder.h"
#include "include/private/base/SkAssert.h"
#include "src/base/SkBase64.h"
#include "tools/testrunners/gm/vias/Draw.h"

#include <sstream>
#include <string>

// Implements the "direct" via. It draws the GM directly on the surface under test without any
// additional behaviors. Equivalent to running a GM with DM without using any vias.
static GMOutput draw_direct(skiagm::GM* gm, SkSurface* surface) {
    // Run the GM.
    SkString msg;
    skiagm::GM::DrawResult result = gm->draw(surface->getCanvas(), &msg);
    if (result != skiagm::DrawResult::kOk) {
        return {result, msg.c_str()};
    }

    // Extract a bitmap from the surface.
    SkBitmap bitmap;
    bitmap.allocPixelsFlags(surface->getCanvas()->imageInfo(), SkBitmap::kZeroPixels_AllocFlag);
    if (!surface->readPixels(bitmap, 0, 0)) {
        return {skiagm::DrawResult::kFail, "Could not read pixels from surface"};
    }
    return {result, msg.c_str(), bitmap};
}

// Encodes a bitmap as a base-64 image/png URI. In the presence of errors, the returned string will
// contain a human-friendly error message. Otherwise the string will start with "data:image/png".
//
// Based on
// https://skia.googlesource.com/skia/+/650c980daa72d887602e701db8f84072e26d4d48/tests/TestUtils.cpp#127.
static std::string bitmap_to_base64_data_uri(const SkBitmap& bitmap) {
    SkPixmap pm;
    if (!bitmap.peekPixels(&pm)) {
        return "peekPixels failed";
    }

    // We're going to embed this PNG in a data URI, so make it as small as possible.
    SkPngEncoder::Options options;
    options.fFilterFlags = SkPngEncoder::FilterFlag::kAll;
    options.fZLibLevel = 9;

    SkDynamicMemoryWStream wStream;
    if (!SkPngEncoder::Encode(&wStream, pm, options)) {
        return "SkPngEncoder::Encode failed";
    }

    sk_sp<SkData> pngData = wStream.detachAsData();
    size_t len = SkBase64::EncodedSize(pngData->size());

    // The PNG can be almost arbitrarily large. We don't want to fill our logs with enormous URLs
    // and should only output them on failure.
    static const size_t kMaxBase64Length = 1024 * 1024;
    if (len > kMaxBase64Length) {
        return SkStringPrintf("Encoded image too large (%lu bytes)", len).c_str();
    }

    std::string out;
    out.resize(len);
    SkBase64::Encode(pngData->data(), pngData->size(), out.data());
    return "data:image/png;base64," + out;
}

// Implements the "picture" and "picture_serialization" vias. The "serialize" argument determines
// which of the two vias is used.
//
// The "picture" via is based on DM's ViaPicture class here:
// https://skia.googlesource.com/skia/+/dcc56df202cca129edda3f6f8bae04ec306b264e/dm/DMSrcSink.cpp#2310.
//
// The "picture_serialization" via is based on DM's ViaSerialization class here:
// https://skia.googlesource.com/skia/+/dcc56df202cca129edda3f6f8bae04ec306b264e/dm/DMSrcSink.cpp#2281.
static GMOutput draw_via_picture(skiagm::GM* gm, SkSurface* surface, bool serialize) {
    // Draw GM on a recording canvas.
    SkPictureRecorder recorder;
    SkCanvas* recordingCanvas =
            recorder.beginRecording(gm->getISize().width(), gm->getISize().height());
    SkString msg;
    skiagm::DrawResult result = gm->draw(recordingCanvas, &msg);
    if (result != skiagm::DrawResult::kOk) {
        return {result, msg.c_str()};
    }

    // Finish recording, and optionally serialize and then deserialize the resulting picture using
    // the PNG encoder/decoder.
    sk_sp<SkPicture> pic = recorder.finishRecordingAsPicture();
    if (serialize) {
        SkSerialProcs serialProcs = {.fImageProc = [](SkImage* img, void*) -> sk_sp<SkData> {
            SkASSERT_RELEASE(!img->isTextureBacked());
            return SkPngEncoder::Encode(nullptr, img, SkPngEncoder::Options{});
        }};
        SkDeserialProcs deserialProcs = {.fImageDataProc = [](sk_sp<SkData> data,
                                                              std::optional<SkAlphaType>,
                                                              void* ctx) -> sk_sp<SkImage> {
            SkCodec::Result decodeResult;
            std::unique_ptr<SkCodec> codec = SkPngDecoder::Decode(data, &decodeResult);
            SkASSERT(decodeResult == SkCodec::Result::kSuccess);
            auto [image, getImageResult] = codec->getImage();
            SkASSERT(getImageResult == SkCodec::Result::kSuccess);
            return image;
        }};
        pic = SkPicture::MakeFromData(pic->serialize(&serialProcs).get(), &deserialProcs);
    }

    // Draw the recorded picture on the surface under test and extract it as a bitmap.
    surface->getCanvas()->drawPicture(pic);
    SkBitmap recordedBitmap;
    recordedBitmap.allocPixelsFlags(surface->getCanvas()->imageInfo(),
                                    SkBitmap::kZeroPixels_AllocFlag);
    if (!surface->readPixels(recordedBitmap, 0, 0)) {
        return {skiagm::DrawResult::kFail, "Could not read recorded picture pixels from surface"};
    }

    // Draw GM on the surface under test and extract the reference bitmap.
    result = gm->draw(surface->getCanvas(), &msg);
    if (result != skiagm::DrawResult::kOk) {
        return {result, msg.c_str()};
    }
    SkBitmap referenceBitmap;
    referenceBitmap.allocPixelsFlags(surface->getCanvas()->imageInfo(),
                                     SkBitmap::kZeroPixels_AllocFlag);
    if (!surface->readPixels(referenceBitmap, 0, 0)) {
        return {skiagm::DrawResult::kFail, "Could not read reference picture pixels from surface"};
    }

    // The recorded and reference bitmaps should be identical.
    if (recordedBitmap.computeByteSize() != referenceBitmap.computeByteSize()) {
        return {skiagm::DrawResult::kFail,
                SkStringPrintf("Recorded and reference bitmap dimensions do not match: "
                               "expected byte size %lu, width %d and height %d; "
                               "got %lu, %d and %d",
                               referenceBitmap.computeByteSize(),
                               referenceBitmap.bounds().width(),
                               referenceBitmap.bounds().height(),
                               recordedBitmap.computeByteSize(),
                               recordedBitmap.bounds().width(),
                               recordedBitmap.bounds().height())
                        .c_str()};
    }
    if (0 != memcmp(recordedBitmap.getPixels(),
                    referenceBitmap.getPixels(),
                    referenceBitmap.computeByteSize())) {
        return {skiagm::DrawResult::kFail,
                SkStringPrintf("Recorded and reference bitmap pixels do not match.\n"
                               "Recorded image:\n%s\nReference image:\n%s",
                               bitmap_to_base64_data_uri(recordedBitmap).c_str(),
                               bitmap_to_base64_data_uri(referenceBitmap).c_str())
                        .c_str()};
    }

    return {result, msg.c_str(), referenceBitmap};
}

// This draw() implementation supports the "direct", "picture" and "picture_serialization" vias.
//
// The "direct" via draws the GM directly on the surface under test with no additional behaviors.
// It is equivalent to running a GM with DM without using a via.
//
// The "picture" via tests that if we record a GM using an SkPictureRecorder, the bitmap produced
// by drawing the recorded picture on the surface under test is the same as the bitmap obtained by
// drawing the GM directly on the surface under test.
//
// The "picture_serialization" via is identical to the "picture" via, except that the recorded
// picture is serialized and then deserialized before being drawn on the surface under test.
GMOutput draw(skiagm::GM* gm, SkSurface* surface, std::string via) {
    if (via == "direct") {
        return draw_direct(gm, surface);
    } else if (via == "picture") {
        return draw_via_picture(gm, surface, /* serialize= */ false);
    } else if (via == "picture_serialization") {
        return draw_via_picture(gm, surface, /* serialize= */ true);
    }
    SK_ABORT("unknown --via flag value: %s", via.c_str());
}
