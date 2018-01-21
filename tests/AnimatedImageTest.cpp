/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAndroidCodec.h"
#include "SkAnimatedImage.h"
#include "SkCanvas.h"
#include "SkCodec.h"
#include "SkUnPreMultiply.h"

#include "CodecPriv.h"
#include "Resources.h"
#include "Test.h"
#include "sk_tool_utils.h"

#include <vector>

DEF_TEST(AnimatedImage, r) {
    if (GetResourcePath().isEmpty()) {
        return;
    }
    for (const char* file : { "images/alphabetAnim.gif",
                              "images/colorTables.gif",
                              "images/webp-animated.webp",
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

        const int defaultRepetitionCount = codec->getRepetitionCount();
        std::vector<SkCodec::FrameInfo> frameInfos = codec->getFrameInfo();
        std::vector<SkBitmap> frames(frameInfos.size());
        // Used down below for our test image.
        const auto imageInfo = codec->getInfo().makeAlphaType(kPremul_SkAlphaType);

        for (size_t i = 0; i < frameInfos.size(); ++i) {
            auto info = codec->getInfo().makeAlphaType(frameInfos[i].fAlphaType);
            auto& bm = frames[i];

            SkCodec::Options options;
            options.fFrameIndex = (int) i;
            options.fPriorFrame = frameInfos[i].fRequiredFrame;
            if (options.fPriorFrame == SkCodec::kNone) {
                bm.allocPixels(info);
                bm.eraseColor(0);
            } else {
                const SkBitmap& priorFrame = frames[options.fPriorFrame];
                if (!sk_tool_utils::copy_to(&bm, priorFrame.colorType(), priorFrame)) {
                    ERRORF(r, "Failed to copy %s frame %i", file, options.fPriorFrame);
                    options.fPriorFrame = SkCodec::kNone;
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

        auto testDraw = [r, &frames, &imageInfo, file](const sk_sp<SkAnimatedImage>& animatedImage,
                                                       int expectedFrame) {
            SkBitmap test;
            test.allocPixels(imageInfo);
            test.eraseColor(0);
            SkCanvas c(test);
            animatedImage->draw(&c);

            const SkBitmap& frame = frames[expectedFrame];
            REPORTER_ASSERT(r, frame.colorType() == test.colorType());
            REPORTER_ASSERT(r, frame.dimensions() == test.dimensions());
            for (int i = 0; i < test.width();  ++i)
            for (int j = 0; j < test.height(); ++j) {
                SkColor expected = SkUnPreMultiply::PMColorToColor(*frame.getAddr32(i, j));
                SkColor actual   = SkUnPreMultiply::PMColorToColor(*test .getAddr32(i, j));
                if (expected != actual) {
                    ERRORF(r, "frame %i of %s does not match at pixel %i, %i!"
                            " expected %x\tactual: %x",
                            expectedFrame, file, i, j, expected, actual);
                    SkString expected_name = SkStringPrintf("expected_%c", '0' + expectedFrame);
                    SkString actual_name   = SkStringPrintf("actual_%c",   '0' + expectedFrame);
                    write_bm(expected_name.c_str(), frame);
                    write_bm(actual_name.c_str(),   test);
                    return false;
                }
            }
            return true;
        };

        REPORTER_ASSERT(r, !animatedImage->isRunning());
        if (!testDraw(animatedImage, 0)) {
            ERRORF(r, "Did not start with frame 0");
            continue;
        }

        animatedImage->start();
        REPORTER_ASSERT(r, animatedImage->isRunning());
        if (!testDraw(animatedImage, 0)) {
            ERRORF(r, "After starting, still not on frame 0");
            continue;
        }

        // Start at an arbitrary time.
        double currentTime = 100000;
        bool failed = false;
        for (size_t i = 0; i < frameInfos.size(); ++i) {
            double next = animatedImage->update(currentTime);
            if (i == frameInfos.size() - 1 && defaultRepetitionCount == 0) {
                REPORTER_ASSERT(r, next == std::numeric_limits<double>::max());
                REPORTER_ASSERT(r, !animatedImage->isRunning());
                REPORTER_ASSERT(r, animatedImage->isFinished());
            } else {
                REPORTER_ASSERT(r, animatedImage->isRunning());
                REPORTER_ASSERT(r, !animatedImage->isFinished());
                double expectedNext = currentTime + frameInfos[i].fDuration;
                if (next != expectedNext) {
                    ERRORF(r, "Time did not match for frame %i: next: %g expected: %g",
                            i, next, expectedNext);
                    failed = true;
                    break;
                }
            }

            if (!testDraw(animatedImage, i)) {
                ERRORF(r, "Did not update to %i properly", i);
                failed = true;
                break;
            }

            // Update, but not to the next frame.
            REPORTER_ASSERT(r, animatedImage->update((next - currentTime) / 2) == next);
            if (!testDraw(animatedImage, i)) {
                ERRORF(r, "Should still be on frame %i", i);
                failed = true;
                break;
            }

            currentTime = next;
        }

        if (failed) {
            continue;
        }

        // Create a new animated image and test stop.
        animatedImage = SkAnimatedImage::Make(SkAndroidCodec::MakeFromCodec(
                    SkCodec::MakeFromData(data)));

        animatedImage->start();
        currentTime = 100000;
        // Do not go to the last frame, so it should still be running after.
        for (size_t i = 0; i < frameInfos.size() - 1; ++i) {
            double next = animatedImage->update(currentTime);
            if (!testDraw(animatedImage, i)) {
                ERRORF(r, "Error during stop tests.");
                failed = true;
                break;
            }

            double interval = next - currentTime;
            animatedImage->stop();
            REPORTER_ASSERT(r, !animatedImage->isRunning());
            REPORTER_ASSERT(r, !animatedImage->isFinished());

            currentTime = next;
            double stoppedNext = animatedImage->update(currentTime);
            REPORTER_ASSERT(r, stoppedNext == std::numeric_limits<double>::max());
            if (!testDraw(animatedImage, i)) {
                ERRORF(r, "Advanced the frame while stopped?");
                failed = true;
                break;
            }

            animatedImage->start();
            currentTime += interval;
        }

        if (failed) {
            return;
        }

        REPORTER_ASSERT(r, animatedImage->isRunning());
        REPORTER_ASSERT(r, !animatedImage->isFinished());
        animatedImage->reset();
        if (!testDraw(animatedImage, 0)) {
            ERRORF(r, "reset failed");
            continue;
        }

        for (int loopCount : { 0, 1, 2, 5 }) {
            animatedImage = SkAnimatedImage::Make(SkAndroidCodec::MakeFromCodec(
                        SkCodec::MakeFromData(data)));
            animatedImage->start();
            animatedImage->setRepetitionCount(loopCount);
            for (int loops = 0; loops <= loopCount; loops++) {
                REPORTER_ASSERT(r, animatedImage->isRunning());
                REPORTER_ASSERT(r, !animatedImage->isFinished());
                for (size_t i = 0; i < frameInfos.size(); ++i) {
                    double next = animatedImage->update(currentTime);
                    if (animatedImage->isRunning()) {
                        currentTime = next;
                    }
                }
            }
            if (animatedImage->isRunning()) {
                ERRORF(r, "%s animation still running after %i loops", file, loopCount);
            }

            if (!animatedImage->isFinished()) {
                ERRORF(r, "%s animation should have finished with specified loop count (%i)",
                          file, loopCount);
            }
        }
    }
}
