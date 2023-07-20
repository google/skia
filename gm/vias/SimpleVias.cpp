/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "gm/vias/Draw.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/encode/SkPngEncoder.h"
#include "include/utils/SkBase64.h"

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
    size_t len = SkBase64::Encode(pngData->data(), pngData->size(), nullptr);

    // The PNG can be almost arbitrarily large. We don't want to fill our logs with enormous URLs
    // and should only output them on failure.
    static const size_t kMaxBase64Length = 1024 * 1024;
    if (len > kMaxBase64Length) {
        return SkStringPrintf("Encoded image too large (%lu bytes)", len).c_str();
    }

    std::string out;
    out.resize(len);
    out.data();
    SkBase64::Encode(pngData->data(), pngData->size(), out.data());
    return "data:image/png;base64," + out;
}

// Implements the "picture_serialization" via.
//
// Based on DM's ViaSerialization class here:
// https://skia.googlesource.com/skia/+/dcc56df202cca129edda3f6f8bae04ec306b264e/dm/DMSrcSink.cpp#2281.
static GMOutput draw_via_picture_serialization(skiagm::GM* gm, SkSurface* surface) {
    // Draw GM on a recording canvas.
    SkPictureRecorder recorder;
    SkCanvas* recordingCanvas =
            recorder.beginRecording(gm->getISize().width(), gm->getISize().height());
    SkString msg;
    skiagm::DrawResult result = gm->draw(recordingCanvas, &msg);
    if (result != skiagm::DrawResult::kOk) {
        return {result, msg.c_str()};
    }

    // Serialize and deserialize the recorded picture. The pic->serialize() call uses the default
    // behavior from SkSerialProcs, which implies a dependency on libpng.
    sk_sp<SkPicture> pic = recorder.finishRecordingAsPicture();
    sk_sp<SkPicture> deserializedPic = SkPicture::MakeFromData(pic->serialize().get());

    // Draw the deserialized picture on the surface under test and extract it as a bitmap.
    surface->getCanvas()->drawPicture(deserializedPic);
    SkBitmap deserializedBitmap;
    deserializedBitmap.allocPixelsFlags(surface->getCanvas()->imageInfo(),
                                        SkBitmap::kZeroPixels_AllocFlag);
    if (!surface->readPixels(deserializedBitmap, 0, 0)) {
        return {skiagm::DrawResult::kFail, "Could not read deserialized pixels from surface"};
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
        return {skiagm::DrawResult::kFail, "Could not read reference pixels from surface"};
    }

    // The deserialized and reference bitmaps should be identical.
    if (deserializedBitmap.computeByteSize() != referenceBitmap.computeByteSize()) {
        return {skiagm::DrawResult::kFail,
                SkStringPrintf("Deserialized and reference bitmap dimensions do not match: "
                               "expected byte size %lu, width %d and height %d; "
                               "got %lu, %d and %d",
                               referenceBitmap.computeByteSize(),
                               referenceBitmap.bounds().width(),
                               referenceBitmap.bounds().height(),
                               deserializedBitmap.computeByteSize(),
                               deserializedBitmap.bounds().width(),
                               deserializedBitmap.bounds().height())
                        .c_str()};
    }
    if (0 != memcmp(deserializedBitmap.getPixels(),
                    referenceBitmap.getPixels(),
                    referenceBitmap.computeByteSize())) {
        return {skiagm::DrawResult::kFail,
                SkStringPrintf("Deserialized and reference bitmap pixels do not match.\n"
                               "Deserialized image:\n%s\nReference image:\n%s",
                               bitmap_to_base64_data_uri(deserializedBitmap).c_str(),
                               bitmap_to_base64_data_uri(referenceBitmap).c_str())
                        .c_str()};
    }

    return {result, msg.c_str(), referenceBitmap};
}

// This draw() implementation supports the "direct" and "picture_serialization" vias.
//
// The "direct" via draws the GM directly on the surface under test with no additional behaviors.
// It is equivalent to running a GM with DM without using a via.
//
// The "picture_serialization" via tests that if we record a GM using an SkPictureRecorder, the
// bitmap produced by serializing the recorded picture, deserializing it, and drawing it on the
// surface under test is the same as the bitmap obtained by drawing the GM directly on the surface
// under test.
GMOutput draw(skiagm::GM* gm, SkSurface* surface, std::string via) {
    if (via == "direct") {
        return draw_direct(gm, surface);
    } else if (via == "picture_serialization") {
        return draw_via_picture_serialization(gm, surface);
    }
    SK_ABORT("unknown --via flag value: %s", via.c_str());
}
