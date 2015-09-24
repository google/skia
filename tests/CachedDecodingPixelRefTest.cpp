/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkDiscardableMemoryPool.h"
#include "SkImageDecoder.h"
#include "SkImageGeneratorPriv.h"
#include "SkResourceCache.h"
#include "SkStream.h"
#include "SkUtils.h"

#include "Test.h"

/**
 * Fill this bitmap with some color.
 */
static void make_test_image(SkBitmap* bm) {
    const int W = 50, H = 50;
    bm->allocN32Pixels(W, H);
    bm->eraseColor(SK_ColorBLACK);
    SkCanvas canvas(*bm);
    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    canvas.drawRectCoords(0, 0, SkIntToScalar(W/2),
                          SkIntToScalar(H/2), paint);
    paint.setColor(SK_ColorWHITE);
    canvas.drawRectCoords(SkIntToScalar(W/2), SkIntToScalar(H/2),
                          SkIntToScalar(W), SkIntToScalar(H), paint);
}

/**
 * encode this bitmap into some data via SkImageEncoder
 */
static SkData* create_data_from_bitmap(const SkBitmap& bm,
                                       SkImageEncoder::Type type) {
    SkDynamicMemoryWStream stream;
    if (SkImageEncoder::EncodeStream(&stream, bm, type, 100)) {
        return stream.copyToData();
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////

static void compare_bitmaps(skiatest::Reporter* reporter,
                            const SkBitmap& b1, const SkBitmap& b2,
                            bool pixelPerfect = true) {
    REPORTER_ASSERT(reporter, b1.empty() == b2.empty());
    REPORTER_ASSERT(reporter, b1.width() == b2.width());
    REPORTER_ASSERT(reporter, b1.height() == b2.height());
    REPORTER_ASSERT(reporter, b1.isNull() == b2.isNull());
    SkAutoLockPixels autoLockPixels1(b1);
    SkAutoLockPixels autoLockPixels2(b2);
    REPORTER_ASSERT(reporter, b1.isNull() == b2.isNull());
    if (b1.isNull() || b1.empty()) {
        return;
    }
    REPORTER_ASSERT(reporter, b1.getPixels());
    REPORTER_ASSERT(reporter, b2.getPixels());
    if ((!(b1.getPixels())) || (!(b2.getPixels()))) {
        return;
    }
    if ((b1.width() != b2.width()) ||
        (b1.height() != b2.height())) {
        return;
    }
    if (!pixelPerfect) {
        return;
    }

    int pixelErrors = 0;
    for (int y = 0; y < b2.height(); ++y) {
        for (int x = 0; x < b2.width(); ++x) {
            if (b1.getColor(x, y) != b2.getColor(x, y)) {
                ++pixelErrors;
            }
        }
    }
    REPORTER_ASSERT(reporter, 0 == pixelErrors);
}

typedef bool (*InstallEncoded)(SkData* encoded, SkBitmap* dst);

/**
   This function tests three differently encoded images against the
   original bitmap */
static void test_three_encodings(skiatest::Reporter* reporter,
                                 InstallEncoded install) {
    SkBitmap original;
    make_test_image(&original);
    REPORTER_ASSERT(reporter, !original.empty());
    REPORTER_ASSERT(reporter, !original.isNull());
    if (original.empty() || original.isNull()) {
        return;
    }
    static const SkImageEncoder::Type types[] = {
        SkImageEncoder::kPNG_Type,
        SkImageEncoder::kJPEG_Type,
        SkImageEncoder::kWEBP_Type
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(types); i++) {
        SkImageEncoder::Type type = types[i];
        SkAutoDataUnref encoded(create_data_from_bitmap(original, type));
        REPORTER_ASSERT(reporter, encoded.get() != nullptr);
        if (nullptr == encoded.get()) {
            continue;
        }
        SkBitmap lazy;
        bool installSuccess = install(encoded.get(), &lazy);
        REPORTER_ASSERT(reporter, installSuccess);
        if (!installSuccess) {
            continue;
        }
        REPORTER_ASSERT(reporter, nullptr == lazy.getPixels());
        {
            SkAutoLockPixels autoLockPixels(lazy);  // now pixels are good.
            REPORTER_ASSERT(reporter, lazy.getPixels());
            if (nullptr == lazy.getPixels()) {
                continue;
            }
        }
        // pixels should be gone!
        REPORTER_ASSERT(reporter, nullptr == lazy.getPixels());
        {
            SkAutoLockPixels autoLockPixels(lazy);  // now pixels are good.
            REPORTER_ASSERT(reporter, lazy.getPixels());
            if (nullptr == lazy.getPixels()) {
                continue;
            }
        }
        bool comparePixels = (SkImageEncoder::kPNG_Type == type);
        compare_bitmaps(reporter, original, lazy, comparePixels);
    }
}

////////////////////////////////////////////////////////////////////////////////
static bool install_skDiscardablePixelRef(SkData* encoded, SkBitmap* dst) {
    // Use system-default discardable memory.
    return SkInstallDiscardablePixelRef(encoded, dst);
}

////////////////////////////////////////////////////////////////////////////////
/**
 *  This checks to see that SkDiscardablePixelRef works as advertised with a
 *  SkDecodingImageGenerator.
 */
DEF_TEST(DecodingImageGenerator, reporter) {
    test_three_encodings(reporter, install_skDiscardablePixelRef);
}

class TestImageGenerator : public SkImageGenerator {
public:
    enum TestType {
        kFailGetPixels_TestType,
        kSucceedGetPixels_TestType,
        kLast_TestType = kSucceedGetPixels_TestType
    };
    static int Width() { return 10; }
    static int Height() { return 10; }
    static uint32_t Color() { return 0xff123456; }
    TestImageGenerator(TestType type, skiatest::Reporter* reporter)
    : INHERITED(GetMyInfo()), fType(type), fReporter(reporter) {
        SkASSERT((fType <= kLast_TestType) && (fType >= 0));
    }
    virtual ~TestImageGenerator() { }

protected:
    static SkImageInfo GetMyInfo() {
        return SkImageInfo::MakeN32(TestImageGenerator::Width(), TestImageGenerator::Height(),
                                    kOpaque_SkAlphaType);
    }

    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                     SkPMColor ctable[], int* ctableCount) override {
        REPORTER_ASSERT(fReporter, pixels != nullptr);
        REPORTER_ASSERT(fReporter, rowBytes >= info.minRowBytes());
        if (fType != kSucceedGetPixels_TestType) {
            return false;
        }
        if (info.colorType() != kN32_SkColorType) {
            return false;
        }
        char* bytePtr = static_cast<char*>(pixels);
        for (int y = 0; y < info.height(); ++y) {
            sk_memset32(reinterpret_cast<SkColor*>(bytePtr),
                        TestImageGenerator::Color(), info.width());
            bytePtr += rowBytes;
        }
        return true;
    }

private:
    const TestType fType;
    skiatest::Reporter* const fReporter;

    typedef SkImageGenerator INHERITED;
};

static void check_test_image_generator_bitmap(skiatest::Reporter* reporter,
                                              const SkBitmap& bm) {
    REPORTER_ASSERT(reporter, TestImageGenerator::Width() == bm.width());
    REPORTER_ASSERT(reporter, TestImageGenerator::Height() == bm.height());
    SkAutoLockPixels autoLockPixels(bm);
    REPORTER_ASSERT(reporter, bm.getPixels());
    if (nullptr == bm.getPixels()) {
        return;
    }
    int errors = 0;
    for (int y = 0; y < bm.height(); ++y) {
        for (int x = 0; x < bm.width(); ++x) {
            if (TestImageGenerator::Color() != *bm.getAddr32(x, y)) {
                ++errors;
            }
        }
    }
    REPORTER_ASSERT(reporter, 0 == errors);
}

static void check_pixelref(TestImageGenerator::TestType type,
                           skiatest::Reporter* reporter,
                           SkDiscardableMemory::Factory* factory) {
    SkAutoTDelete<SkImageGenerator> gen(new TestImageGenerator(type, reporter));
    REPORTER_ASSERT(reporter, gen.get() != nullptr);
    SkBitmap lazy;
    bool success = SkInstallDiscardablePixelRef(gen.detach(), nullptr, &lazy, factory);

    REPORTER_ASSERT(reporter, success);
    if (TestImageGenerator::kSucceedGetPixels_TestType == type) {
        check_test_image_generator_bitmap(reporter, lazy);
    } else if (TestImageGenerator::kFailGetPixels_TestType == type) {
        SkAutoLockPixels autoLockPixels(lazy);
        REPORTER_ASSERT(reporter, nullptr == lazy.getPixels());
    }
}

/**
 *  This tests the basic functionality of SkDiscardablePixelRef with a
 *  basic SkImageGenerator implementation and several
 *  SkDiscardableMemory::Factory choices.
 */
DEF_TEST(DiscardableAndCachingPixelRef, reporter) {
    check_pixelref(TestImageGenerator::kFailGetPixels_TestType, reporter, nullptr);
    check_pixelref(TestImageGenerator::kSucceedGetPixels_TestType, reporter, nullptr);

    SkAutoTUnref<SkDiscardableMemoryPool> pool(
        SkDiscardableMemoryPool::Create(1, nullptr));
    REPORTER_ASSERT(reporter, 0 == pool->getRAMUsed());
    check_pixelref(TestImageGenerator::kFailGetPixels_TestType, reporter, pool);
    REPORTER_ASSERT(reporter, 0 == pool->getRAMUsed());
    check_pixelref(TestImageGenerator::kSucceedGetPixels_TestType, reporter, pool);
    REPORTER_ASSERT(reporter, 0 == pool->getRAMUsed());

    SkDiscardableMemoryPool* globalPool = SkGetGlobalDiscardableMemoryPool();
    // Only acts differently from nullptr on a platform that has a
    // default discardable memory implementation that differs from the
    // global DM pool.
    check_pixelref(TestImageGenerator::kFailGetPixels_TestType, reporter, globalPool);
    check_pixelref(TestImageGenerator::kSucceedGetPixels_TestType, reporter, globalPool);
}

////////////////////////////////////////////////////////////////////////////////

DEF_TEST(Image_NewFromGenerator, r) {
    TestImageGenerator::TestType testTypes[] = {
        TestImageGenerator::kFailGetPixels_TestType,
        TestImageGenerator::kSucceedGetPixels_TestType,
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(testTypes); ++i) {
        TestImageGenerator::TestType test = testTypes[i];
        SkImageGenerator* gen = new TestImageGenerator(test, r);
        SkAutoTUnref<SkImage> image(SkImage::NewFromGenerator(gen));
        if (nullptr == image.get()) {
            ERRORF(r, "SkImage::NewFromGenerator unexpecedly failed ["
                   SK_SIZE_T_SPECIFIER "]", i);
            continue;
        }
        REPORTER_ASSERT(r, TestImageGenerator::Width() == image->width());
        REPORTER_ASSERT(r, TestImageGenerator::Height() == image->height());
        REPORTER_ASSERT(r, image->isLazyGenerated());

        SkBitmap bitmap;
        bitmap.allocN32Pixels(TestImageGenerator::Width(), TestImageGenerator::Height());
        SkCanvas canvas(bitmap);
        const SkColor kDefaultColor = 0xffabcdef;
        canvas.clear(kDefaultColor);
        canvas.drawImage(image, 0, 0, nullptr);
        if (TestImageGenerator::kSucceedGetPixels_TestType == test) {
            REPORTER_ASSERT(
                    r, TestImageGenerator::Color() == *bitmap.getAddr32(0, 0));
        } else {
            REPORTER_ASSERT(r, kDefaultColor == bitmap.getColor(0,0));
        }
    }
}
