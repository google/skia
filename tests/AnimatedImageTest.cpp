/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/android/SkAnimatedImage.h"
#include "include/codec/SkAndroidCodec.h"
#include "include/codec/SkCodec.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/core/SkUnPreMultiply.h"
#include "tests/CodecPriv.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include <cstddef>
#include <initializer_list>
#include <memory>
#include <utility>
#include <vector>

DEF_TEST(AnimatedImage_simple, r) {
    if (GetResourcePath().isEmpty()) {
        return;
    }

    const char* file = "images/stoplight_h.webp";
    auto data = GetResourceAsData(file);
    if (!data) {
        ERRORF(r, "Could not get %s", file);
        return;
    }

    // An animated image with a non-default exif orientation is no longer
    // "simple"; verify that the assert has been removed.
    auto androidCodec = SkAndroidCodec::MakeFromData(std::move(data));
    auto animatedImage = SkAnimatedImage::Make(std::move(androidCodec));
    REPORTER_ASSERT(r, animatedImage);
}

DEF_TEST(AnimatedImage_rotation, r) {
    if (GetResourcePath().isEmpty()) {
        return;
    }

    // These images use different exif orientations to achieve the same final
    // dimensions
    const auto expectedBounds = SkRect::MakeIWH(100, 80);
    for (int i = 1; i <=8; i++) {
        for (const SkString& name : { SkStringPrintf("images/orientation/%d.webp", i),
                                      SkStringPrintf("images/orientation/%d_444.jpg", i) }) {

            const char* file = name.c_str();
            auto data = GetResourceAsData(file);
            if (!data) {
                ERRORF(r, "Could not get %s", file);
                return;
            }

            auto androidCodec = SkAndroidCodec::MakeFromData(std::move(data));
            auto animatedImage = SkAnimatedImage::Make(std::move(androidCodec));
            if (!animatedImage) {
                ERRORF(r, "Failed to create animated image from %s", file);
                return;
            }

            auto bounds = animatedImage->getBounds();
            if (bounds != expectedBounds) {
                ERRORF(r, "Mismatched bounds for %s", file);
                bounds.dump();
            }
        }
    }
}

DEF_TEST(AnimatedImage_invalidCrop, r) {
    if (GetResourcePath().isEmpty()) {
        return;
    }

    const char* file = "images/alphabetAnim.gif";
    auto data = GetResourceAsData(file);
    if (!data) {
        ERRORF(r, "Could not get %s", file);
        return;
    }

    const struct Rec {
        bool    valid;
        SkISize scaledSize;
        SkIRect cropRect;
    } gRecs[] = {
        // cropRect contained by original dimensions
        { true,  {100, 100}, {   0,  0, 100, 100} },
        { true,  {100, 100}, {   0,  0,  50,  50} },
        { true,  {100, 100}, {  10, 10, 100, 100} },
        { true,  {100, 100}, {   0,  0, 100, 100} },

        // unsorted cropRect
        { false, {100, 100}, {   0, 100, 100,   0} },
        { false, {100, 100}, { 100,   0,   0, 100} },

        // cropRect not contained by original dimensions
        { false, {100, 100}, {   0,   1, 100, 101} },
        { false, {100, 100}, {   0,  -1, 100,  99} },
        { false, {100, 100}, {  -1,   0,  99, 100} },
        { false, {100, 100}, { 100, 100, 200, 200} },

        // cropRect contained by scaled dimensions
        { true,  { 50,  50}, {   0,   0,  50,  50} },
        { true,  { 50,  50}, {   0,   0,  25,  25} },
        { true,  {200, 200}, {   0,   1, 100, 101} },

        // cropRect not contained by scaled dimensions
        { false, { 50,  50}, {   0,   0,  75,  25} },
        { false, { 50,  50}, {   0,   0,  25,  75} },

    };
    for (const auto& rec : gRecs) {
        auto codec = SkAndroidCodec::MakeFromData(data);
        if (!codec) {
            ERRORF(r, "Could not create codec for %s", file);
            return;
        }

        auto info = codec->getInfo();
        REPORTER_ASSERT(r, info.dimensions() == SkISize::Make(100, 100));

        auto image = SkAnimatedImage::Make(std::move(codec), info.makeDimensions(rec.scaledSize),
                rec.cropRect, nullptr);

        REPORTER_ASSERT(r, rec.valid == !!image.get());
    }
}

DEF_TEST(AnimatedImage_scaled, r) {
    if (GetResourcePath().isEmpty()) {
        return;
    }

    const char* file = "images/alphabetAnim.gif";
    auto data = GetResourceAsData(file);
    if (!data) {
        ERRORF(r, "Could not get %s", file);
        return;
    }

    auto codec = SkAndroidCodec::MakeFromCodec(SkCodec::MakeFromData(data));
    if (!codec) {
        ERRORF(r, "Could not create codec for %s", file);
        return;
    }

    // Force the drawable follow its special case that requires scaling.
    auto info = codec->getInfo();
    info = info.makeWH(info.width() - 5, info.height() - 5);
    auto rect = info.bounds();
    auto image = SkAnimatedImage::Make(std::move(codec), info, rect, nullptr);
    if (!image) {
        ERRORF(r, "Failed to create animated image for %s", file);
        return;
    }

    // Clear a bitmap to non-transparent and draw to it. pixels that are transparent
    // in the image should not replace the original non-transparent color.
    SkBitmap bm;
    bm.allocPixels(SkImageInfo::MakeN32Premul(info.width(), info.height()));
    bm.eraseColor(SK_ColorBLUE);
    SkCanvas canvas(bm);
    image->draw(&canvas);
    for (int i = 0; i < info.width();  ++i)
    for (int j = 0; j < info.height(); ++j) {
        if (*bm.getAddr32(i, j) == SK_ColorTRANSPARENT) {
            ERRORF(r, "Erased color underneath!");
            return;
        }
    }
}

static bool compare_bitmaps(skiatest::Reporter* r,
                            const char* file,
                            int expectedFrame,
                            const SkBitmap& expectedBm,
                            const SkBitmap& actualBm) {
    REPORTER_ASSERT(r, expectedBm.colorType() == actualBm.colorType());
    REPORTER_ASSERT(r, expectedBm.dimensions() == actualBm.dimensions());
    for (int i = 0; i < actualBm.width();  ++i)
    for (int j = 0; j < actualBm.height(); ++j) {
        SkColor expected = SkUnPreMultiply::PMColorToColor(*expectedBm.getAddr32(i, j));
        SkColor actual   = SkUnPreMultiply::PMColorToColor(*actualBm  .getAddr32(i, j));
        if (expected != actual) {
            ERRORF(r, "frame %i of %s does not match at pixel %i, %i!"
                            " expected %x\tactual: %x",
                            expectedFrame, file, i, j, expected, actual);
            SkString expected_name = SkStringPrintf("expected_%c", '0' + expectedFrame);
            SkString actual_name   = SkStringPrintf("actual_%c",   '0' + expectedFrame);
            write_bm(expected_name.c_str(), expectedBm);
            write_bm(actual_name.c_str(),   actualBm);
            return false;
        }
    }
    return true;
}

DEF_TEST(AnimatedImage_copyOnWrite, r) {
    if (GetResourcePath().isEmpty()) {
        return;
    }
    for (const char* file : { "images/alphabetAnim.gif",
                              "images/colorTables.gif",
                              "images/stoplight.webp",
                              "images/required.webp",
                              }) {
        auto data = GetResourceAsData(file);
        if (!data) {
            ERRORF(r, "Could not get %s", file);
            continue;
        }

        auto codec = SkCodec::MakeFromData(data);
        if (!codec) {
            ERRORF(r, "Could not create codec for %s", file);
            continue;
        }

        const auto imageInfo = codec->getInfo().makeAlphaType(kPremul_SkAlphaType);
        const int frameCount = codec->getFrameCount();
        auto androidCodec = SkAndroidCodec::MakeFromCodec(std::move(codec));
        if (!androidCodec) {
            ERRORF(r, "Could not create androidCodec for %s", file);
            continue;
        }

        auto animatedImage = SkAnimatedImage::Make(std::move(androidCodec));
        if (!animatedImage) {
            ERRORF(r, "Could not create animated image for %s", file);
            continue;
        }
        animatedImage->setRepetitionCount(0);

        std::vector<SkBitmap> expected(frameCount);
        std::vector<sk_sp<SkPicture>> pictures(frameCount);
        for (int i = 0; i < frameCount; i++) {
            SkBitmap& bm = expected[i];
            bm.allocPixels(imageInfo);
            bm.eraseColor(SK_ColorTRANSPARENT);
            SkCanvas canvas(bm);

            pictures[i].reset(animatedImage->newPictureSnapshot());
            canvas.drawPicture(pictures[i]);

            const auto duration = animatedImage->decodeNextFrame();
            // We're attempting to decode i + 1, so decodeNextFrame will return
            // kFinished if that is the last frame (or we attempt to decode one
            // more).
            if (i >= frameCount - 2) {
                REPORTER_ASSERT(r, duration == SkAnimatedImage::kFinished);
            } else {
                REPORTER_ASSERT(r, duration != SkAnimatedImage::kFinished);
            }
        }

        for (int i = 0; i < frameCount; i++) {
            SkBitmap test;
            test.allocPixels(imageInfo);
            test.eraseColor(SK_ColorTRANSPARENT);
            SkCanvas canvas(test);

            canvas.drawPicture(pictures[i]);

            compare_bitmaps(r, file, i, expected[i], test);
        }
    }
}

DEF_TEST(AnimatedImage, r) {
    if (GetResourcePath().isEmpty()) {
        return;
    }
    for (const char* file : { "images/alphabetAnim.gif",
                              "images/colorTables.gif",
                              "images/stoplight.webp",
                              "images/required.webp",
                              }) {
        auto data = GetResourceAsData(file);
        if (!data) {
            ERRORF(r, "Could not get %s", file);
            continue;
        }

        auto codec = SkCodec::MakeFromData(data);
        if (!codec) {
            ERRORF(r, "Could not create codec for %s", file);
            continue;
        }

        std::vector<SkCodec::FrameInfo> frameInfos = codec->getFrameInfo();
        std::vector<SkBitmap> frames(frameInfos.size());
        // Used down below for our test image.
        const auto imageInfo = codec->getInfo().makeAlphaType(kPremul_SkAlphaType);

        // Get the repetition count after the codec->getFrameInfo() call above
        // has walked to the end of the encoded image.
        //
        // At the file format level, GIF images can declare their repetition
        // count multiple times and our codec goes with "last one wins".
        // Furthermore, for single-frame (still) GIF images, a zero, positive
        // or infinite repetition count are all equivalent in practice (in all
        // cases, the pixels do not change over time), so the codec has some
        // leeway in what to return for single-frame GIF images, but it cannot
        // distinguish single-frame from multiple-frame GIFs until we count the
        // number of frames (e.g. call getFrameInfo).
        const int defaultRepetitionCount = codec->getRepetitionCount();

        for (size_t i = 0; i < frameInfos.size(); ++i) {
            auto info = codec->getInfo().makeAlphaType(frameInfos[i].fAlphaType);
            auto& bm = frames[i];

            SkCodec::Options options;
            options.fFrameIndex = (int) i;
            options.fPriorFrame = frameInfos[i].fRequiredFrame;
            if (options.fPriorFrame == SkCodec::kNoFrame) {
                bm.allocPixels(info);
                bm.eraseColor(0);
            } else {
                const SkBitmap& priorFrame = frames[options.fPriorFrame];
                if (!ToolUtils::copy_to(&bm, priorFrame.colorType(), priorFrame)) {
                    ERRORF(r, "Failed to copy %s frame %i", file, options.fPriorFrame);
                    options.fPriorFrame = SkCodec::kNoFrame;
                }
                REPORTER_ASSERT(r, bm.setAlphaType(frameInfos[i].fAlphaType));
            }

            auto result = codec->getPixels(info, bm.getPixels(), bm.rowBytes(), &options);
            if (result != SkCodec::kSuccess) {
                ERRORF(r, "error in %s frame %zu: %s", file, i, SkCodec::ResultToString(result));
            }
        }

        auto androidCodec = SkAndroidCodec::MakeFromCodec(std::move(codec));
        if (!androidCodec) {
            ERRORF(r, "Could not create androidCodec for %s", file);
            continue;
        }

        auto animatedImage = SkAnimatedImage::Make(std::move(androidCodec));
        if (!animatedImage) {
            ERRORF(r, "Could not create animated image for %s", file);
            continue;
        }

        REPORTER_ASSERT(r, defaultRepetitionCount == animatedImage->getRepetitionCount());

        auto testDraw = [r, &frames, &imageInfo, file](const sk_sp<SkAnimatedImage>& animatedImage,
                                                       int expectedFrame) {
            SkBitmap test;
            test.allocPixels(imageInfo);
            test.eraseColor(0);
            SkCanvas c(test);
            animatedImage->draw(&c);

            const SkBitmap& frame = frames[expectedFrame];
            return compare_bitmaps(r, file, expectedFrame, frame, test);
        };

        REPORTER_ASSERT(r, animatedImage->currentFrameDuration() == frameInfos[0].fDuration);

        if (!testDraw(animatedImage, 0)) {
            ERRORF(r, "Did not start with frame 0");
            continue;
        }

        // Start at an arbitrary time.
        bool failed = false;
        for (size_t i = 1; i < frameInfos.size(); ++i) {
            const int frameTime = animatedImage->decodeNextFrame();
            REPORTER_ASSERT(r, frameTime == animatedImage->currentFrameDuration());

            if (i == frameInfos.size() - 1 && defaultRepetitionCount == 0) {
                REPORTER_ASSERT(r, frameTime == SkAnimatedImage::kFinished);
                REPORTER_ASSERT(r, animatedImage->isFinished());
            } else {
                REPORTER_ASSERT(r, frameTime == frameInfos[i].fDuration);
                REPORTER_ASSERT(r, !animatedImage->isFinished());
            }

            if (!testDraw(animatedImage, i)) {
                ERRORF(r, "Did not update to %zu properly", i);
                failed = true;
                break;
            }
        }

        if (failed) {
            continue;
        }

        animatedImage->reset();
        REPORTER_ASSERT(r, !animatedImage->isFinished());
        if (!testDraw(animatedImage, 0)) {
            ERRORF(r, "reset failed");
            continue;
        }

        // Test reset from all the frames.
        // j is the frame to call reset on.
        for (int j = 0; j < (int) frameInfos.size(); ++j) {
            if (failed) {
                break;
            }

            // i is the frame to decode.
            for (int i = 0; i <= j; ++i) {
                if (i == j) {
                    animatedImage->reset();
                    if (!testDraw(animatedImage, 0)) {
                        ERRORF(r, "reset failed for image %s from frame %i",
                                file, i);
                        failed = true;
                        break;
                    }
                } else if (i != 0) {
                    animatedImage->decodeNextFrame();
                    if (!testDraw(animatedImage, i)) {
                        ERRORF(r, "failed to match frame %i in %s on iteration %i",
                                i, file, j);
                        failed = true;
                        break;
                    }
                }
            }
        }

        if (failed) {
            continue;
        }

        for (int loopCount : { 0, 1, 2, 5 }) {
            animatedImage = SkAnimatedImage::Make(SkAndroidCodec::MakeFromCodec(
                        SkCodec::MakeFromData(data)));
            animatedImage->setRepetitionCount(loopCount);
            REPORTER_ASSERT(r, animatedImage->getRepetitionCount() == loopCount);

            for (int loops = 0; loops <= loopCount; loops++) {
                if (failed) {
                    break;
                }
                REPORTER_ASSERT(r, !animatedImage->isFinished());
                for (size_t i = 1; i <= frameInfos.size(); ++i) {
                    const int frameTime = animatedImage->decodeNextFrame();
                    if (frameTime == SkAnimatedImage::kFinished) {
                        if (loops != loopCount) {
                            ERRORF(r, "%s animation stopped early: loops: %i\tloopCount: %i",
                                    file, loops, loopCount);
                            failed = true;
                        }
                        if (i != frameInfos.size() - 1) {
                            ERRORF(r, "%s animation stopped early: i: %zu\tsize: %zu",
                                    file, i, frameInfos.size());
                            failed = true;
                        }
                        break;
                    }
                }
            }

            if (!animatedImage->isFinished()) {
                ERRORF(r, "%s animation should have finished with specified loop count (%i)",
                          file, loopCount);
            }
        }
    }
}
