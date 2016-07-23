/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkCodec.h"
#include "Test.h"

static SkStreamAsset* resource(const char path[]) {
    SkString fullPath = GetResourcePath(path);
    return SkStream::NewFromFile(fullPath.c_str());
}

DEF_TEST(ExifOrientation, r) {
    SkAutoTDelete<SkStream> stream(resource("exif-orientation-2-ur.jpg"));
    REPORTER_ASSERT(r, nullptr != stream);
    if (!stream) {
        return;
    }

    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromStream(stream.release()));
    REPORTER_ASSERT(r, nullptr != codec);
    SkCodec::Origin origin = codec->getOrigin();
    REPORTER_ASSERT(r, SkCodec::kTopRight_Origin == origin);

    stream.reset(resource("mandrill_512_q075.jpg"));
    codec.reset(SkCodec::NewFromStream(stream.release()));
    REPORTER_ASSERT(r, nullptr != codec);
    origin = codec->getOrigin();
    REPORTER_ASSERT(r, SkCodec::kTopLeft_Origin == origin);
}
