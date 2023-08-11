/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/private/SkColorData.h"
#include "src/core/SkMemset.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

class TestImageGenerator : public SkImageGenerator {
public:
    enum TestType {
        kFailGetPixels_TestType,
        kSucceedGetPixels_TestType,
        kLast_TestType = kSucceedGetPixels_TestType
    };
    static int Width() { return 10; }
    static int Height() { return 10; }
    // value choosen so that there is no loss when converting to to RGB565 and back
    static SkColor   Color() { return ToolUtils::color_to_565(0xffaabbcc); }
    static SkPMColor PMColor() { return SkPreMultiplyColor(Color()); }

    TestImageGenerator(TestType type, skiatest::Reporter* reporter,
                       SkColorType colorType = kN32_SkColorType)
    : SkImageGenerator(GetMyInfo(colorType)), fType(type), fReporter(reporter) {
        SkASSERT((fType <= kLast_TestType) && (fType >= 0));
    }
    ~TestImageGenerator() override {}

protected:
    static SkImageInfo GetMyInfo(SkColorType colorType) {
        return SkImageInfo::Make(TestImageGenerator::Width(), TestImageGenerator::Height(),
                                 colorType, kOpaque_SkAlphaType);
    }

    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                     const Options& options) override {
        REPORTER_ASSERT(fReporter, pixels != nullptr);
        REPORTER_ASSERT(fReporter, rowBytes >= info.minRowBytes());
        if (fType != kSucceedGetPixels_TestType) {
            return false;
        }
        if (info.colorType() != kN32_SkColorType && info.colorType() != getInfo().colorType()) {
            return false;
        }
        char* bytePtr = static_cast<char*>(pixels);
        switch (info.colorType()) {
            case kN32_SkColorType:
                for (int y = 0; y < info.height(); ++y) {
                    SkOpts::memset32((uint32_t*)bytePtr,
                                TestImageGenerator::PMColor(), info.width());
                    bytePtr += rowBytes;
                }
                break;
            case kRGB_565_SkColorType:
                for (int y = 0; y < info.height(); ++y) {
                    SkOpts::memset16((uint16_t*)bytePtr,
                        SkPixel32ToPixel16(TestImageGenerator::PMColor()), info.width());
                    bytePtr += rowBytes;
                }
                break;
            default:
                return false;
        }
        return true;
    }

private:
    const TestType fType;
    skiatest::Reporter* const fReporter;
};

////////////////////////////////////////////////////////////////////////////////

DEF_TEST(Image_NewFromGenerator, r) {
    const TestImageGenerator::TestType testTypes[] = {
        TestImageGenerator::kFailGetPixels_TestType,
        TestImageGenerator::kSucceedGetPixels_TestType,
    };
    const SkColorType testColorTypes[] = {
        kN32_SkColorType,
        kRGB_565_SkColorType
    };
    for (size_t i = 0; i < std::size(testTypes); ++i) {
        TestImageGenerator::TestType test = testTypes[i];
        for (const SkColorType testColorType : testColorTypes) {
            auto gen = std::make_unique<TestImageGenerator>(test, r, testColorType);
            sk_sp<SkImage> image(SkImages::DeferredFromGenerator(std::move(gen)));
            if (nullptr == image) {
                ERRORF(r, "SkImage::NewFromGenerator unexpecedly failed [%zu]", i);
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
            canvas.drawImage(image, 0, 0);
            if (TestImageGenerator::kSucceedGetPixels_TestType == test) {
                REPORTER_ASSERT(
                    r, TestImageGenerator::Color() == bitmap.getColor(0, 0));
            }
            else {
                REPORTER_ASSERT(r, kDefaultColor == bitmap.getColor(0, 0));
            }
        }
    }
}
