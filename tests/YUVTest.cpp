/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkAutoMalloc.h"
#include "SkCodec.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkYUVSizeInfo.h"
#include "Test.h"

static void codec_yuv(skiatest::Reporter* reporter,
                  const char path[],
                  SkISize expectedSizes[3]) {
    std::unique_ptr<SkStream> stream(GetResourceAsStream(path));
    if (!stream) {
        return;
    }
    std::unique_ptr<SkCodec> codec(SkCodec::NewFromStream(stream.release()));
    REPORTER_ASSERT(reporter, codec);
    if (!codec) {
        return;
    }

    // Test queryYUV8()
    SkYUVSizeInfo info;
    bool success = codec->queryYUV8(nullptr, nullptr);
    REPORTER_ASSERT(reporter, !success);
    success = codec->queryYUV8(&info, nullptr);
    REPORTER_ASSERT(reporter, (expectedSizes == nullptr) == !success);
    if (!success) {
        return;
    }
    REPORTER_ASSERT(reporter,
            0 == memcmp((const void*) &info, (const void*) expectedSizes, 3 * sizeof(SkISize)));
    REPORTER_ASSERT(reporter, info.fWidthBytes[SkYUVSizeInfo::kY] ==
            (uint32_t) SkAlign8(info.fSizes[SkYUVSizeInfo::kY].width()));
    REPORTER_ASSERT(reporter, info.fWidthBytes[SkYUVSizeInfo::kU] ==
            (uint32_t) SkAlign8(info.fSizes[SkYUVSizeInfo::kU].width()));
    REPORTER_ASSERT(reporter, info.fWidthBytes[SkYUVSizeInfo::kV] ==
            (uint32_t) SkAlign8(info.fSizes[SkYUVSizeInfo::kV].width()));
    SkYUVColorSpace colorSpace;
    success = codec->queryYUV8(&info, &colorSpace);
    REPORTER_ASSERT(reporter,
            0 == memcmp((const void*) &info, (const void*) expectedSizes, 3 * sizeof(SkISize)));
    REPORTER_ASSERT(reporter, info.fWidthBytes[SkYUVSizeInfo::kY] ==
            (uint32_t) SkAlign8(info.fSizes[SkYUVSizeInfo::kY].width()));
    REPORTER_ASSERT(reporter, info.fWidthBytes[SkYUVSizeInfo::kU] ==
            (uint32_t) SkAlign8(info.fSizes[SkYUVSizeInfo::kU].width()));
    REPORTER_ASSERT(reporter, info.fWidthBytes[SkYUVSizeInfo::kV] ==
            (uint32_t) SkAlign8(info.fSizes[SkYUVSizeInfo::kV].width()));
    REPORTER_ASSERT(reporter, kJPEG_SkYUVColorSpace == colorSpace);

    // Allocate the memory for the YUV decode
    size_t totalBytes =
            info.fWidthBytes[SkYUVSizeInfo::kY] * info.fSizes[SkYUVSizeInfo::kY].height() +
            info.fWidthBytes[SkYUVSizeInfo::kU] * info.fSizes[SkYUVSizeInfo::kU].height() +
            info.fWidthBytes[SkYUVSizeInfo::kV] * info.fSizes[SkYUVSizeInfo::kV].height();
    SkAutoMalloc storage(totalBytes);
    void* planes[3];
    planes[0] = storage.get();
    planes[1] = SkTAddOffset<void>(planes[0],
            info.fWidthBytes[SkYUVSizeInfo::kY] * info.fSizes[SkYUVSizeInfo::kY].height());
    planes[2] = SkTAddOffset<void>(planes[1],
            info.fWidthBytes[SkYUVSizeInfo::kU] * info.fSizes[SkYUVSizeInfo::kU].height());

    // Test getYUV8Planes()
    REPORTER_ASSERT(reporter, SkCodec::kInvalidInput ==
            codec->getYUV8Planes(info, nullptr));
    REPORTER_ASSERT(reporter, SkCodec::kSuccess ==
            codec->getYUV8Planes(info, planes));
}

DEF_TEST(Jpeg_YUV_Codec, r) {
    SkISize sizes[3];

    sizes[0].set(128, 128);
    sizes[1].set(64, 64);
    sizes[2].set(64, 64);
    codec_yuv(r, "color_wheel.jpg", sizes);

    // H2V2
    sizes[0].set(512, 512);
    sizes[1].set(256, 256);
    sizes[2].set(256, 256);
    codec_yuv(r, "mandrill_512_q075.jpg", sizes);

    // H1V1
    sizes[1].set(512, 512);
    sizes[2].set(512, 512);
    codec_yuv(r, "mandrill_h1v1.jpg", sizes);

    // H2V1
    sizes[1].set(256, 512);
    sizes[2].set(256, 512);
    codec_yuv(r, "mandrill_h2v1.jpg", sizes);

    // Non-power of two dimensions
    sizes[0].set(439, 154);
    sizes[1].set(220, 77);
    sizes[2].set(220, 77);
    codec_yuv(r, "cropped_mandrill.jpg", sizes);

    sizes[0].set(8, 8);
    sizes[1].set(4, 4);
    sizes[2].set(4, 4);
    codec_yuv(r, "randPixels.jpg", sizes);

    // Progressive images
    sizes[0].set(512, 512);
    sizes[1].set(512, 512);
    sizes[2].set(512, 512);
    codec_yuv(r, "brickwork-texture.jpg", sizes);
    codec_yuv(r, "brickwork_normal-map.jpg", sizes);

    // A CMYK encoded image should fail.
    codec_yuv(r, "CMYK.jpg", nullptr);
    // A grayscale encoded image should fail.
    codec_yuv(r, "grayscale.jpg", nullptr);
    // A PNG should fail.
    codec_yuv(r, "arrow.png", nullptr);
}
