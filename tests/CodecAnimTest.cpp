/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkCommonFlags.h"
#include "SkImageEncoder.h"
#include "SkOSPath.h"
#include "SkStream.h"

#include "Resources.h"
#include "Test.h"
#include "sk_tool_utils.h"

#include <initializer_list>
#include <vector>

static void write_bm(const char* name, const SkBitmap& bm) {
    if (FLAGS_writePath.isEmpty()) {
        return;
    }

    SkString filename = SkOSPath::Join(FLAGS_writePath[0], name);
    filename.appendf(".png");
    SkFILEWStream file(filename.c_str());
    if (!SkEncodeImage(&file, bm, SkEncodedImageFormat::kPNG, 100)) {
        SkDebugf("failed to write '%s'\n", filename.c_str());
    }
}

DEF_TEST(Codec_trunc, r) {
    sk_sp<SkData> data(GetResourceAsData("box.gif"));
    data = SkData::MakeSubset(data.get(), 0, 23);
    std::unique_ptr<SkCodec> codec(SkCodec::NewFromData(data));
    codec->getFrameInfo();
}

DEF_TEST(Codec_frames, r) {
    #define kOpaque     kOpaque_SkAlphaType
    #define kUnpremul   kUnpremul_SkAlphaType
    static const struct {
        const char*              fName;
        int                      fFrameCount;
        // One less than fFramecount, since the first frame is always
        // independent.
        std::vector<int>         fRequiredFrames;
        // Same, since the first frame should match getInfo.
        std::vector<SkAlphaType> fAlphaTypes;
        // The size of this one should match fFrameCount for animated, empty
        // otherwise.
        std::vector<int>         fDurations;
        int                      fRepetitionCount;
    } gRecs[] = {
        { "alphabetAnim.gif", 13,
            { SkCodec::kNone, 0, 0, 0, 0, 5, 6, SkCodec::kNone,
              SkCodec::kNone, SkCodec::kNone, 10, 11 },
            { kUnpremul, kUnpremul, kUnpremul, kUnpremul, kUnpremul, kUnpremul,
              kUnpremul, kUnpremul, kUnpremul, kOpaque, kOpaque, kUnpremul },
            { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
            0 },
        { "randPixelsAnim2.gif", 4,
            // required frames
            { 0, 0, 1 },
            // alphas
            { kOpaque, kOpaque, kOpaque },
            // durations
            { 0, 1000, 170, 40 },
            // repetition count
            0 },
        { "randPixelsAnim.gif", 13,
            // required frames
            { SkCodec::kNone, 1, 2, 3, 4, 3, 6, 7, 7, 7, 9, 9 },
            { kUnpremul, kUnpremul, kUnpremul, kUnpremul, kUnpremul, kUnpremul,
              kUnpremul, kUnpremul, kUnpremul, kUnpremul, kUnpremul, kUnpremul },
            // durations
            { 0, 1000, 170, 40, 220, 7770, 90, 90, 90, 90, 90, 90, 90 },
            // repetition count
            0 },
        { "box.gif", 1, {}, {}, {}, 0 },
        { "color_wheel.gif", 1, {}, {}, {}, 0 },
        { "test640x479.gif", 4, { 0, 1, 2 },
                { kOpaque, kOpaque, kOpaque },
                { 200, 200, 200, 200 },
                SkCodec::kRepetitionCountInfinite },
        { "colorTables.gif", 2, { 0 }, { kOpaque }, { 1000, 1000 }, 5 },

        { "arrow.png",  1, {}, {}, {}, 0 },
        { "google_chrome.ico", 1, {}, {}, {}, 0 },
        { "brickwork-texture.jpg", 1, {}, {}, {}, 0 },
#if defined(SK_CODEC_DECODES_RAW) && (!defined(_WIN32))
        { "dng_with_preview.dng", 1, {}, {}, {}, 0 },
#endif
        { "mandrill.wbmp", 1, {}, {}, {}, 0 },
        { "randPixels.bmp", 1, {}, {}, {}, 0 },
        { "yellow_rose.webp", 1, {}, {}, {}, 0 },
    };
    #undef kOpaque
    #undef kUnpremul

    for (const auto& rec : gRecs) {
        sk_sp<SkData> data(GetResourceAsData(rec.fName));
        if (!data) {
            // Useful error statement, but sometimes people run tests without
            // resources, and they do not want to see these messages.
            //ERRORF(r, "Missing resources? Could not find '%s'", rec.fName);
            continue;
        }

        std::unique_ptr<SkCodec> codec(SkCodec::NewFromData(data));
        if (!codec) {
            ERRORF(r, "Failed to create an SkCodec from '%s'", rec.fName);
            continue;
        }

        {
            SkCodec::FrameInfo frameInfo;
            REPORTER_ASSERT(r, !codec->getFrameInfo(0, &frameInfo));
        }

        const int repetitionCount = codec->getRepetitionCount();
        if (repetitionCount != rec.fRepetitionCount) {
            ERRORF(r, "%s repetition count does not match! expected: %i\tactual: %i",
                      rec.fName, rec.fRepetitionCount, repetitionCount);
        }

        const int expected = rec.fFrameCount;
        if (rec.fRequiredFrames.size() + 1 != static_cast<size_t>(expected)) {
            ERRORF(r, "'%s' has wrong number entries in fRequiredFrames; expected: %i\tactual: %i",
                   rec.fName, expected, rec.fRequiredFrames.size() + 1);
            continue;
        }

        if (rec.fDurations.size() != static_cast<size_t>(expected)) {
            ERRORF(r, "'%s' has wrong number entries in fDurations; expected: %i\tactual: %i",
                   rec.fName, expected, rec.fDurations.size());
            continue;
        }

        enum class TestMode {
            kVector,
            kIndividual,
        };

        for (auto mode : { TestMode::kVector, TestMode::kIndividual }) {
            // Re-create the codec to reset state and test parsing.
            codec.reset(SkCodec::NewFromData(data));

            int frameCount;
            std::vector<SkCodec::FrameInfo> frameInfos;
            switch (mode) {
                case TestMode::kVector:
                    frameInfos = codec->getFrameInfo();
                    // getFrameInfo returns empty set for non-animated.
                    frameCount = frameInfos.empty() ? 1 : frameInfos.size();
                    break;
                case TestMode::kIndividual:
                    frameCount = codec->getFrameCount();
                    break;
            }

            if (frameCount != expected) {
                ERRORF(r, "'%s' expected frame count: %i\tactual: %i",
                       rec.fName, expected, frameCount);
                continue;
            }

            // From here on, we are only concerned with animated images.
            if (1 == frameCount) {
                continue;
            }

            for (int i = 0; i < frameCount; i++) {
                SkCodec::FrameInfo frameInfo;
                switch (mode) {
                    case TestMode::kVector:
                        frameInfo = frameInfos[i];
                        break;
                    case TestMode::kIndividual:
                        REPORTER_ASSERT(r, codec->getFrameInfo(i, nullptr));
                        REPORTER_ASSERT(r, codec->getFrameInfo(i, &frameInfo));
                        break;
                }

                if (rec.fDurations[i] != frameInfo.fDuration) {
                    ERRORF(r, "%s frame %i's durations do not match! expected: %i\tactual: %i",
                           rec.fName, i, rec.fDurations[i], frameInfo.fDuration);
                }

                if (0 == i) {
                    REPORTER_ASSERT(r, frameInfo.fRequiredFrame == SkCodec::kNone);
                    REPORTER_ASSERT(r, frameInfo.fAlphaType == codec->getInfo().alphaType());
                    continue;
                }

                if (rec.fRequiredFrames[i-1] != frameInfo.fRequiredFrame) {
                    ERRORF(r, "%s's frame %i has wrong dependency! expected: %i\tactual: %i",
                           rec.fName, i, rec.fRequiredFrames[i-1], frameInfo.fRequiredFrame);
                }

                auto to_string = [](SkAlphaType type) {
                    switch (type) {
                        case kUnpremul_SkAlphaType:
                            return "unpremul";
                        case kOpaque_SkAlphaType:
                            return "opaque";
                        default:
                            return "other";
                    }
                };

                auto expectedAlpha = rec.fAlphaTypes[i-1];
                auto alpha = frameInfo.fAlphaType;
                if (expectedAlpha != alpha) {
                    ERRORF(r, "%s's frame %i has wrong alpha type! expected: %s\tactual: %s",
                           rec.fName, i, to_string(expectedAlpha), to_string(alpha));
                }
            }

            if (TestMode::kIndividual == mode) {
                // No need to test decoding twice.
                return;
            }

            // Compare decoding in two ways:
            // 1. Provide the frame that a frame depends on, so the codec just has to blend.
            //    (in the array cachedFrames)
            // 2. Do not provide the frame that a frame depends on, so the codec has to decode
            //    all the way back to a key-frame. (in a local variable uncachedFrame)
            // The two should look the same.
            std::vector<SkBitmap> cachedFrames(frameCount);
            const auto& info = codec->getInfo().makeColorType(kN32_SkColorType);

            auto decode = [&](SkBitmap* bm, bool cached, int index) {
                bm->allocPixels(info);
                if (cached) {
                    // First copy the pixels from the cached frame
                    const int requiredFrame = frameInfos[index].fRequiredFrame;
                    if (requiredFrame != SkCodec::kNone) {
                        const bool success = sk_tool_utils::copy_to(bm, kN32_SkColorType,
                                cachedFrames[requiredFrame]);
                        REPORTER_ASSERT(r, success);
                    }
                }
                SkCodec::Options opts;
                opts.fFrameIndex = index;
                opts.fHasPriorFrame = cached;
                auto result = codec->getPixels(info, bm->getPixels(), bm->rowBytes(),
                                               &opts, nullptr, nullptr);
                REPORTER_ASSERT(r, result == SkCodec::kSuccess);
            };

            for (int i = 0; i < frameCount; i++) {
                SkBitmap& cachedFrame = cachedFrames[i];
                decode(&cachedFrame, true, i);
                SkBitmap uncachedFrame;
                decode(&uncachedFrame, false, i);

                // Now verify they're equal.
                const size_t rowLen = info.bytesPerPixel() * info.width();
                for (int y = 0; y < info.height(); y++) {
                    const void* cachedAddr = cachedFrame.getAddr(0, y);
                    SkASSERT(cachedAddr != nullptr);
                    const void* uncachedAddr = uncachedFrame.getAddr(0, y);
                    SkASSERT(uncachedAddr != nullptr);
                    const bool lineMatches = memcmp(cachedAddr, uncachedAddr, rowLen) == 0;
                    if (!lineMatches) {
                        SkString name = SkStringPrintf("cached_%i", i);
                        write_bm(name.c_str(), cachedFrame);
                        name = SkStringPrintf("uncached_%i", i);
                        write_bm(name.c_str(), uncachedFrame);
                        ERRORF(r, "%s's frame %i is different depending on caching!", rec.fName, i);
                        break;
                    }
                }
            }
        }
    }
}
