/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkMipMap.h"
#include "tests/Test.h"

static void make_bitmap(SkBitmap* bm, int width, int height) {
    bm->allocN32Pixels(width, height);
    bm->eraseColor(SK_ColorWHITE);
}

DEF_TEST(MipMap, reporter) {
    SkBitmap bm;
    SkRandom rand;

    for (int i = 0; i < 500; ++i) {
        int width = 1 + rand.nextU() % 1000;
        int height = 1 + rand.nextU() % 1000;
        make_bitmap(&bm, width, height);
        sk_sp<SkMipMap> mm(SkMipMap::Build(bm, nullptr));

        REPORTER_ASSERT(reporter, mm->countLevels() == SkMipMap::ComputeLevelCount(width, height));
        REPORTER_ASSERT(reporter, !mm->extractLevel(SkSize::Make(SK_Scalar1, SK_Scalar1),
                                                    nullptr));
        REPORTER_ASSERT(reporter, !mm->extractLevel(SkSize::Make(SK_Scalar1 * 2, SK_Scalar1 * 2),
                                                    nullptr));

        SkMipMap::Level prevLevel;
        sk_bzero(&prevLevel, sizeof(prevLevel));

        SkScalar scale = SK_Scalar1;
        for (int j = 0; j < 30; ++j) {
            scale = scale * 2 / 3;

            SkMipMap::Level level;
            if (mm->extractLevel(SkSize::Make(scale, scale), &level)) {
                REPORTER_ASSERT(reporter, level.fPixmap.addr());
                REPORTER_ASSERT(reporter, level.fPixmap.width() > 0);
                REPORTER_ASSERT(reporter, level.fPixmap.height() > 0);
                REPORTER_ASSERT(reporter, (int)level.fPixmap.rowBytes() >= level.fPixmap.width() * 4);

                if (prevLevel.fPixmap.addr()) {
                    REPORTER_ASSERT(reporter, level.fPixmap.width() <= prevLevel.fPixmap.width());
                    REPORTER_ASSERT(reporter, level.fPixmap.height() <= prevLevel.fPixmap.height());
                }
                prevLevel = level;
            }
        }
    }
}

static void test_mipmap_generation(int width, int height, int expectedMipLevelCount,
                                   skiatest::Reporter* reporter) {
    SkBitmap bm;
    bm.allocN32Pixels(width, height);
    bm.eraseColor(SK_ColorWHITE);
    sk_sp<SkMipMap> mm(SkMipMap::Build(bm, nullptr));

    const int mipLevelCount = mm->countLevels();
    REPORTER_ASSERT(reporter, mipLevelCount == expectedMipLevelCount);
    REPORTER_ASSERT(reporter, mipLevelCount == SkMipMap::ComputeLevelCount(width, height));
    for (int i = 0; i < mipLevelCount; ++i) {
        SkMipMap::Level level;
        REPORTER_ASSERT(reporter, mm->getLevel(i, &level));
        // Make sure the mipmaps contain valid data and that the sizes are correct
        REPORTER_ASSERT(reporter, level.fPixmap.addr());
        SkISize size = SkMipMap::ComputeLevelSize(width, height, i);
        REPORTER_ASSERT(reporter, level.fPixmap.width() == size.width());
        REPORTER_ASSERT(reporter, level.fPixmap.height() == size.height());

        // + 1 because SkMipMap does not include the base mipmap level.
        int twoToTheMipLevel = 1 << (i + 1);
        int currentWidth = width / twoToTheMipLevel;
        int currentHeight = height / twoToTheMipLevel;
        REPORTER_ASSERT(reporter, level.fPixmap.width() == currentWidth);
        REPORTER_ASSERT(reporter, level.fPixmap.height() == currentHeight);
    }
}

DEF_TEST(MipMap_DirectLevelAccess, reporter) {
    // create mipmap with invalid size
    {
        // SkMipMap current requires the dimensions be greater than 2x2
        SkBitmap bm;
        bm.allocN32Pixels(1, 1);
        bm.eraseColor(SK_ColorWHITE);
        sk_sp<SkMipMap> mm(SkMipMap::Build(bm, nullptr));

        REPORTER_ASSERT(reporter, mm == nullptr);
    }

    // check small mipmap's count and levels
    // There should be 5 mipmap levels generated:
    // 16x16, 8x8, 4x4, 2x2, 1x1
    test_mipmap_generation(32, 32, 5, reporter);

    // check large mipmap's count and levels
    // There should be 9 mipmap levels generated:
    // 500x500, 250x250, 125x125, 62x62, 31x31, 15x15, 7x7, 3x3, 1x1
    test_mipmap_generation(1000, 1000, 9, reporter);
}

struct LevelCountScenario {
    int fWidth;
    int fHeight;
    int fExpectedLevelCount;
};

DEF_TEST(MipMap_ComputeLevelCount, reporter) {
    const LevelCountScenario tests[] = {
        // Test mipmaps with negative sizes
        {-100, 100, 0},
        {100, -100, 0},
        {-100, -100, 0},

        // Test mipmaps with 0, 1, 2 as dimensions
        // (SkMipMap::Build requires a min size of 1)
        //
        // 0
        {0, 100, 0},
        {100, 0, 0},
        {0, 0, 0},
        // 1
        {1, 100, 6},
        {100, 1, 6},
        {1, 1, 0},
        // 2
        {2, 100, 6},
        {100, 2, 6},
        {2, 2, 1},

        // Test a handful of boundaries such as 63x63 and 64x64
        {63, 63, 5},
        {64, 64, 6},
        {127, 127, 6},
        {128, 128, 7},
        {255, 255, 7},
        {256, 256, 8},

        // Test different dimensions, such as 256x64
        {64, 129, 7},
        {255, 32, 7},
        {500, 1000, 9}
    };

    for (auto& currentTest : tests) {
        int levelCount = SkMipMap::ComputeLevelCount(currentTest.fWidth, currentTest.fHeight);
        REPORTER_ASSERT(reporter, currentTest.fExpectedLevelCount == levelCount);
    }
}

struct LevelSizeScenario {
    int fBaseWidth;
    int fBaseHeight;
    int fLevel;
    SkISize fExpectedMipMapLevelSize;
};

DEF_TEST(MipMap_ComputeLevelSize, reporter) {
    const LevelSizeScenario tests[] = {
        // Test mipmaps with negative sizes
        {-100, 100, 0, SkISize::Make(0, 0)},
        {100, -100, 0, SkISize::Make(0, 0)},
        {-100, -100, 0, SkISize::Make(0, 0)},

        // Test mipmaps with 0, 1, 2 as dimensions
        // (SkMipMap::Build requires a min size of 1)
        //
        // 0
        {0, 100, 0, SkISize::Make(0, 0)},
        {100, 0, 0, SkISize::Make(0, 0)},
        {0, 0, 0, SkISize::Make(0, 0)},
        // 1

        {1, 100, 0, SkISize::Make(1, 50)},
        {100, 1, 0, SkISize::Make(50, 1)},
        {1, 1, 0, SkISize::Make(0, 0)},
        // 2
        {2, 100, 0, SkISize::Make(1, 50)},
        {100, 2, 1, SkISize::Make(25, 1)},
        {2, 2, 0, SkISize::Make(1, 1)},

        // Test a handful of cases
        {63, 63, 2, SkISize::Make(7, 7)},
        {64, 64, 2, SkISize::Make(8, 8)},
        {127, 127, 2, SkISize::Make(15, 15)},
        {64, 129, 3, SkISize::Make(4, 8)},
        {255, 32, 6, SkISize::Make(1, 1)},
        {500, 1000, 1, SkISize::Make(125, 250)},
    };

    for (auto& currentTest : tests) {
        SkISize levelSize = SkMipMap::ComputeLevelSize(currentTest.fBaseWidth,
                                                       currentTest.fBaseHeight,
                                                       currentTest.fLevel);
        REPORTER_ASSERT(reporter, currentTest.fExpectedMipMapLevelSize == levelSize);
    }
}

DEF_TEST(MipMap_F16, reporter) {
    SkBitmap bmp;
    bmp.allocPixels(SkImageInfo::Make(10, 10, kRGBA_F16_SkColorType, kPremul_SkAlphaType));
    bmp.eraseColor(0);
    sk_sp<SkMipMap> mipmap(SkMipMap::Build(bmp, nullptr));
}
