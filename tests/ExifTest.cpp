/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkColor.h"
#include "SkImageInfo.h"
#include "SkJpegEncoder.h"
#include "SkStream.h"
#include "Test.h"

DEF_TEST(ExifOrientation, r) {
    std::unique_ptr<SkStream> stream(GetResourceAsStream("exif-orientation-2-ur.jpg"));
    REPORTER_ASSERT(r, nullptr != stream);
    if (!stream) {
        return;
    }

    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromStream(std::move(stream)));
    REPORTER_ASSERT(r, nullptr != codec);
    SkEncodedOrigin origin = codec->getOrigin();
    REPORTER_ASSERT(r, kTopRight_SkEncodedOrigin == origin);

    codec = SkCodec::MakeFromStream(GetResourceAsStream("mandrill_512_q075.jpg"));
    REPORTER_ASSERT(r, nullptr != codec);
    origin = codec->getOrigin();
    REPORTER_ASSERT(r, kTopLeft_SkEncodedOrigin == origin);
}

#ifdef SK_HAS_EXIF_LIBRARY
DEF_TEST(ExifWriteOrientation, r) {
    SkBitmap bm;
    bm.allocPixels(SkImageInfo::MakeN32Premul(100, 100));
    bm.eraseColor(SK_ColorBLUE);
    SkPixmap pm;
    if (!bm.peekPixels(&pm)) {
        ERRORF(r, "failed to peek pixels");
        return;
    }
    for (auto o : { kTopLeft_SkEncodedOrigin,
                    kTopRight_SkEncodedOrigin,
                    kBottomRight_SkEncodedOrigin,
                    kBottomLeft_SkEncodedOrigin,
                    kLeftTop_SkEncodedOrigin,
                    kRightTop_SkEncodedOrigin,
                    kRightBottom_SkEncodedOrigin,
                    kLeftBottom_SkEncodedOrigin }) {
        SkDynamicMemoryWStream stream;
        SkJpegEncoder::Options options;
        options.fOrigin = o;
        if (!SkJpegEncoder::Encode(&stream, pm, options)) {
            ERRORF(r, "Failed to encode with orientation %i", o);
            return;
        }

        auto data = stream.detachAsData();
        auto codec = SkCodec::MakeFromData(std::move(data));
        if (!codec) {
            ERRORF(r, "Failed to create a codec with orientation %i", o);
            return;
        }

        REPORTER_ASSERT(r, codec->getOrigin() == o);
    }
}
#endif
