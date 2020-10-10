/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkCodec.h"
#include "tests/Test.h"
#include "tools/Resources.h"

DEF_TEST(ExifOrientation, r) {
    std::unique_ptr<SkStream> stream(GetResourceAsStream("images/exif-orientation-2-ur.jpg"));
    REPORTER_ASSERT(r, nullptr != stream);
    if (!stream) {
        return;
    }

    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromStream(std::move(stream)));
    REPORTER_ASSERT(r, nullptr != codec);
    SkEncodedOrigin origin = codec->getOrigin();
    REPORTER_ASSERT(r, kTopRight_SkEncodedOrigin == origin);

    codec = SkCodec::MakeFromStream(GetResourceAsStream("images/mandrill_512_q075.jpg"));
    REPORTER_ASSERT(r, nullptr != codec);
    origin = codec->getOrigin();
    REPORTER_ASSERT(r, kTopLeft_SkEncodedOrigin == origin);
}

DEF_TEST(ExifOrientationInExif, r) {
    std::unique_ptr<SkStream> stream(GetResourceAsStream("images/orientation/exif.jpg"));

    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromStream(std::move(stream));
    REPORTER_ASSERT(r, nullptr != codec);
    SkEncodedOrigin origin = codec->getOrigin();
    REPORTER_ASSERT(r, kLeftBottom_SkEncodedOrigin == origin);
}

DEF_TEST(ExifOrientationInSubIFD, r) {
    std::unique_ptr<SkStream> stream(GetResourceAsStream("images/orientation/subifd.jpg"));

    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromStream(std::move(stream));
    REPORTER_ASSERT(r, nullptr != codec);
    SkEncodedOrigin origin = codec->getOrigin();
    REPORTER_ASSERT(r, kLeftBottom_SkEncodedOrigin == origin);
}
