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

#include "include/core/SkMatrix44.h"
#include "include/effects/SkColorMatrix.h"
#include "src/core/SkYUVMath.h"

// Be sure that the two matrices are inverses of each other
// (i.e. rgb2yuv and yuv2rgb
DEF_TEST(YUVMath, reporter) {
    const SkYUVColorSpace spaces[] = {
        kJPEG_SkYUVColorSpace,
        kRec601_SkYUVColorSpace,
        kRec709_SkYUVColorSpace,
        kBT2020_SkYUVColorSpace,
        kIdentity_SkYUVColorSpace,
    };

    // Not sure what the theoretical precision we can hope for is, so pick a big value that
    // passes (when I think we're correct).
    const float tolerance = 1.0f/(1 << 18);

    for (auto cs : spaces) {
        float r2y[20], y2r[20];
        SkColorMatrix_RGB2YUV(cs, r2y);
        SkColorMatrix_YUV2RGB(cs, y2r);

        SkColorMatrix r2ym, y2rm;
        r2ym.setRowMajor(r2y);
        y2rm.setRowMajor(y2r);
        r2ym.postConcat(y2rm);

        float tmp[20];
        r2ym.getRowMajor(tmp);
        for (int i = 0; i < 20; ++i) {
            float expected = 0;
            if (i % 6 == 0) {   // diagonal
                expected = 1;
            }
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(tmp[i], expected, tolerance));
        }
    }
}

DEF_TEST(YUVChromium, reporter) {
    // Dump 8, 10, and 12-bit limited range BT2020 matrices, using chromium's math:
    const float Kr = 0.2627f,
                Kb = 0.0593f,
                Kg = 1.0f - Kr - Kb;
    const float u_m = 0.5f / (1.0f - Kb),
                v_m = 0.5f / (1.0f - Kr);
    const float data[16] = {
                         Kr,        Kg,                Kb, 0.0f,  // Y
                  u_m * -Kr, u_m * -Kg, u_m * (1.0f - Kb), 0.5f,  // U
          v_m * (1.0f - Kr), v_m * -Kg,         v_m * -Kb, 0.5f,  // V
                       0.0f,      0.0f,              0.0f, 1.0f,
    };
    SkMatrix44 transferMatrix;  // RGB -> YUV
    transferMatrix.setRowMajorf(data);
    SkDebugf("BT2020 Transfer matrix:\n");
    transferMatrix.dump();

    for (int bit_depth : {8, 10, 12}) {
        SkMatrix44 rangeMatrix;
        const int shift = bit_depth - 8;
        const float a_y = 219 << shift;
        const float c = (1 << bit_depth) - 1;
        const float scale_y = c / a_y;

        const float a_uv = 224 << shift;
        const float scale_uv = c / a_uv;
        const float translate_uv = (a_uv - c) / (2.0f * a_uv);
        rangeMatrix.setScale(scale_y, scale_uv, scale_uv);
        rangeMatrix.postTranslate(-16.0f/219.0f, translate_uv, translate_uv);

        SkDebugf("\n%d bit range adjust:\n", bit_depth);
        rangeMatrix.dump();

        SkMatrix44 invRangeMatrix;
        SkAssertResult(rangeMatrix.invert(&invRangeMatrix));
        SkDebugf("\n%d bit inv-range:\n", bit_depth);
        invRangeMatrix.dump();

        SkMatrix44 combined;
        combined.setConcat(invRangeMatrix, transferMatrix);
        SkDebugf("\nCombined RGB -> YUV:\n");
        combined.dump();
    }
}
