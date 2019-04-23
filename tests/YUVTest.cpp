/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkCodec.h"
#include "include/core/SkStream.h"
#include "include/core/SkYUVASizeInfo.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkAutoMalloc.h"
#include "tests/Test.h"
#include "tools/Resources.h"

static void codec_yuv(skiatest::Reporter* reporter,
                      const char path[],
                      SkISize expectedSizes[4]) {
    std::unique_ptr<SkStream> stream(GetResourceAsStream(path));
    if (!stream) {
        return;
    }
    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromStream(std::move(stream)));
    REPORTER_ASSERT(reporter, codec);
    if (!codec) {
        return;
    }

    // Test queryYUV8()
    SkYUVASizeInfo info;

    {
        bool success = codec->queryYUV8(nullptr, nullptr);
        REPORTER_ASSERT(reporter, !success);
        success = codec->queryYUV8(&info, nullptr);
        REPORTER_ASSERT(reporter, (expectedSizes == nullptr) == !success);
        if (!success) {
            return;
        }

        for (int i = 0; i < SkYUVASizeInfo::kMaxCount; ++i) {
            REPORTER_ASSERT(reporter, info.fSizes[i] == expectedSizes[i]);
            REPORTER_ASSERT(reporter,
                            info.fWidthBytes[i] == (uint32_t) SkAlign8(info.fSizes[i].width()));
        }
    }

    {
        SkYUVColorSpace colorSpace;
        bool success = codec->queryYUV8(&info, &colorSpace);
        REPORTER_ASSERT(reporter, (expectedSizes == nullptr) == !success);
        if (!success) {
            return;
        }

        for (int i = 0; i < SkYUVASizeInfo::kMaxCount; ++i) {
            REPORTER_ASSERT(reporter, info.fSizes[i] == expectedSizes[i]);
            REPORTER_ASSERT(reporter,
                            info.fWidthBytes[i] == (uint32_t) SkAlign8(info.fSizes[i].width()));
        }
        REPORTER_ASSERT(reporter, kJPEG_SkYUVColorSpace == colorSpace);
    }

    // Allocate the memory for the YUV decode
    size_t totalBytes = info.computeTotalBytes();

    SkAutoMalloc storage(totalBytes);
    void* planes[SkYUVASizeInfo::kMaxCount];

    info.computePlanes(storage.get(), planes);

    // Test getYUV8Planes()
    REPORTER_ASSERT(reporter, SkCodec::kInvalidInput == codec->getYUV8Planes(info, nullptr));
    REPORTER_ASSERT(reporter, SkCodec::kSuccess == codec->getYUV8Planes(info, planes));
}

DEF_TEST(Jpeg_YUV_Codec, r) {
    SkISize sizes[4];

    sizes[0].set(128, 128);
    sizes[1].set(64, 64);
    sizes[2].set(64, 64);
    sizes[3].set(0, 0);
    codec_yuv(r, "images/color_wheel.jpg", sizes);

    // H2V2
    sizes[0].set(512, 512);
    sizes[1].set(256, 256);
    sizes[2].set(256, 256);
    codec_yuv(r, "images/mandrill_512_q075.jpg", sizes);

    // H1V1
    sizes[1].set(512, 512);
    sizes[2].set(512, 512);
    codec_yuv(r, "images/mandrill_h1v1.jpg", sizes);

    // H2V1
    sizes[1].set(256, 512);
    sizes[2].set(256, 512);
    codec_yuv(r, "images/mandrill_h2v1.jpg", sizes);

    // Non-power of two dimensions
    sizes[0].set(439, 154);
    sizes[1].set(220, 77);
    sizes[2].set(220, 77);
    codec_yuv(r, "images/cropped_mandrill.jpg", sizes);

    sizes[0].set(8, 8);
    sizes[1].set(4, 4);
    sizes[2].set(4, 4);
    codec_yuv(r, "images/randPixels.jpg", sizes);

    // Progressive images
    sizes[0].set(512, 512);
    sizes[1].set(512, 512);
    sizes[2].set(512, 512);
    codec_yuv(r, "images/brickwork-texture.jpg", sizes);
    codec_yuv(r, "images/brickwork_normal-map.jpg", sizes);

    // A CMYK encoded image should fail.
    codec_yuv(r, "images/CMYK.jpg", nullptr);
    // A grayscale encoded image should fail.
    codec_yuv(r, "images/grayscale.jpg", nullptr);
    // A PNG should fail.
    codec_yuv(r, "images/arrow.png", nullptr);
}
