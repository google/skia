/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkAndroidCodec.h"
#include "include/codec/SkCodec.h"
#include "include/codec/SkCodecAnimation.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/utils/SkAnimCodecPlayer.h"
#include "src/core/SkMakeUnique.h"
#include "tests/CodecPriv.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include <cstring>
#include <memory>
#include <utility>
#include <vector>

DEF_TEST(Codec_trunc, r) {
    sk_sp<SkData> data(GetResourceAsData("images/box.gif"));
    if (!data) {
        return;
    }
    // See also Codec_GifTruncated2 in GifTest.cpp for this magic 23.
    //
    // TODO: just move this getFrameInfo call to Codec_GifTruncated2?
    SkCodec::MakeFromData(SkData::MakeSubset(data.get(), 0, 23))->getFrameInfo();
}

// 565 does not support alpha, but there is no reason for it not to support an
// animated image with a frame that has alpha but then blends onto an opaque
// frame making the result opaque. Test that we can decode such a frame.
DEF_TEST(Codec_565, r) {
    sk_sp<SkData> data(GetResourceAsData("images/blendBG.webp"));
    if (!data) {
        return;
    }
    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(std::move(data)));
    auto info = codec->getInfo().makeColorType(kRGB_565_SkColorType);
    SkBitmap bm;
    bm.allocPixels(info);

    SkCodec::Options options;
    options.fFrameIndex = 1;
    options.fPriorFrame = SkCodec::kNoFrame;

    const auto result = codec->getPixels(info, bm.getPixels(), bm.rowBytes(),
                                         &options);
    REPORTER_ASSERT(r, result == SkCodec::kSuccess);
}

static bool restore_previous(const SkCodec::FrameInfo& info) {
    return info.fDisposalMethod == SkCodecAnimation::DisposalMethod::kRestorePrevious;
}

DEF_TEST(Codec_frames, r) {
    constexpr int kNoFrame = SkCodec::kNoFrame;
    constexpr SkAlphaType kOpaque = kOpaque_SkAlphaType;
    constexpr SkAlphaType kUnpremul = kUnpremul_SkAlphaType;
    constexpr SkCodecAnimation::DisposalMethod kKeep =
            SkCodecAnimation::DisposalMethod::kKeep;
    constexpr SkCodecAnimation::DisposalMethod kRestoreBG =
            SkCodecAnimation::DisposalMethod::kRestoreBGColor;
    constexpr SkCodecAnimation::DisposalMethod kRestorePrev =
            SkCodecAnimation::DisposalMethod::kRestorePrevious;

    static const struct {
        const char*                                   fName;
        int                                           fFrameCount;
        // One less than fFramecount, since the first frame is always
        // independent.
        std::vector<int>                              fRequiredFrames;
        // Same, since the first frame should match getInfo
        std::vector<SkAlphaType>                      fAlphas;
        // The size of this one should match fFrameCount for animated, empty
        // otherwise.
        std::vector<int>                              fDurations;
        int                                           fRepetitionCount;
        std::vector<SkCodecAnimation::DisposalMethod> fDisposalMethods;
    } gRecs[] = {
        { "images/required.gif", 7,
            { 0, 1, 2, 3, 4, 5 },
            { kOpaque, kUnpremul, kUnpremul, kUnpremul, kUnpremul, kUnpremul },
            { 100, 100, 100, 100, 100, 100, 100 },
            0,
            { kKeep, kRestoreBG, kKeep, kKeep, kKeep, kRestoreBG, kKeep } },
        { "images/alphabetAnim.gif", 13,
            { kNoFrame, 0, 0, 0, 0, 5, 6, kNoFrame, kNoFrame, 9, 10, 11 },
            { kUnpremul, kUnpremul, kUnpremul, kUnpremul, kUnpremul, kUnpremul,
              kUnpremul, kUnpremul, kUnpremul, kUnpremul, kUnpremul, kUnpremul },
            { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
            0,
            { kKeep, kRestorePrev, kRestorePrev, kRestorePrev, kRestorePrev,
              kRestoreBG, kKeep, kRestoreBG, kRestoreBG, kKeep, kKeep,
              kRestoreBG, kKeep } },
        { "images/randPixelsAnim2.gif", 4,
            // required frames
            { 0, 0, 1 },
            // alphas
            { kOpaque, kOpaque, kOpaque },
            // durations
            { 0, 1000, 170, 40 },
            // repetition count
            0,
            { kKeep, kKeep, kRestorePrev, kKeep } },
        { "images/randPixelsAnim.gif", 13,
            // required frames
            { 0, 1, 2, 3, 4, 3, 6, 7, 7, 7, 9, 9 },
            { kUnpremul, kUnpremul, kUnpremul, kUnpremul, kUnpremul, kUnpremul,
              kUnpremul, kUnpremul, kUnpremul, kUnpremul, kUnpremul, kUnpremul },
            // durations
            { 0, 1000, 170, 40, 220, 7770, 90, 90, 90, 90, 90, 90, 90 },
            // repetition count
            0,
            { kKeep, kKeep, kKeep, kKeep, kRestoreBG, kRestoreBG, kRestoreBG,
              kRestoreBG, kRestorePrev, kRestoreBG, kRestorePrev, kRestorePrev,
              kRestorePrev,  } },
        { "images/box.gif", 1, {}, {}, {}, 0, { kKeep } },
        { "images/color_wheel.gif", 1, {}, {}, {}, 0, { kKeep } },
        { "images/test640x479.gif", 4, { 0, 1, 2 },
                { kOpaque, kOpaque, kOpaque },
                { 200, 200, 200, 200 },
                SkCodec::kRepetitionCountInfinite,
                { kKeep, kKeep, kKeep, kKeep } },
        { "images/colorTables.gif", 2, { 0 }, { kOpaque }, { 1000, 1000 }, 5,
                { kKeep, kKeep } },

        { "images/arrow.png",  1, {}, {}, {}, 0, {} },
        { "images/google_chrome.ico", 1, {}, {}, {}, 0, {} },
        { "images/brickwork-texture.jpg", 1, {}, {}, {}, 0, {} },
#if defined(SK_CODEC_DECODES_RAW) && (!defined(_WIN32))
        { "images/dng_with_preview.dng", 1, {}, {}, {}, 0, {} },
#endif
        { "images/mandrill.wbmp", 1, {}, {}, {}, 0, {} },
        { "images/randPixels.bmp", 1, {}, {}, {}, 0, {} },
        { "images/yellow_rose.webp", 1, {}, {}, {}, 0, {} },
        { "images/webp-animated.webp", 3, { 0, 1 }, { kOpaque, kOpaque },
            { 1000, 500, 1000 }, SkCodec::kRepetitionCountInfinite,
            { kKeep, kKeep, kKeep } },
        { "images/blendBG.webp", 7,
            { 0, kNoFrame, kNoFrame, kNoFrame, 4, 4 },
            { kOpaque, kOpaque, kUnpremul, kOpaque, kUnpremul, kUnpremul },
            { 525, 500, 525, 437, 609, 729, 444 }, 7,
            { kKeep, kKeep, kKeep, kKeep, kKeep, kKeep, kKeep } },
        { "images/required.webp", 7,
            { 0, 1, 1, kNoFrame, 4, 4 },
            { kOpaque, kUnpremul, kUnpremul, kOpaque, kOpaque, kOpaque },
            { 100, 100, 100, 100, 100, 100, 100 },
            1,
            { kKeep, kRestoreBG, kKeep, kKeep, kKeep, kRestoreBG, kKeep } },
    };

    for (const auto& rec : gRecs) {
        sk_sp<SkData> data(GetResourceAsData(rec.fName));
        if (!data) {
            // Useful error statement, but sometimes people run tests without
            // resources, and they do not want to see these messages.
            //ERRORF(r, "Missing resources? Could not find '%s'", rec.fName);
            continue;
        }

        std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(data));
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
                   rec.fName, expected - 1, rec.fRequiredFrames.size());
            continue;
        }

        if (expected > 1) {
            if (rec.fDurations.size() != static_cast<size_t>(expected)) {
                ERRORF(r, "'%s' has wrong number entries in fDurations; expected: %i\tactual: %i",
                       rec.fName, expected, rec.fDurations.size());
                continue;
            }

            if (rec.fAlphas.size() + 1 != static_cast<size_t>(expected)) {
                ERRORF(r, "'%s' has wrong number entries in fAlphas; expected: %i\tactual: %i",
                       rec.fName, expected - 1, rec.fAlphas.size());
                continue;
            }

            if (rec.fDisposalMethods.size() != static_cast<size_t>(expected)) {
                ERRORF(r, "'%s' has wrong number entries in fDisposalMethods; "
                       "expected %i\tactual: %i",
                       rec.fName, expected, rec.fDisposalMethods.size());
                continue;
            }
        }

        enum class TestMode {
            kVector,
            kIndividual,
        };

        for (auto mode : { TestMode::kVector, TestMode::kIndividual }) {
            // Re-create the codec to reset state and test parsing.
            codec = SkCodec::MakeFromData(data);

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

                auto to_string = [](SkAlphaType alpha) {
                    switch (alpha) {
                        case kUnpremul_SkAlphaType:
                            return "unpremul";
                        case kOpaque_SkAlphaType:
                            return "opaque";
                        default:
                            SkASSERT(false);
                            return "unknown";
                    }
                };

                auto expectedAlpha = 0 == i ? codec->getInfo().alphaType() : rec.fAlphas[i-1];
                auto alpha = frameInfo.fAlphaType;
                if (expectedAlpha != alpha) {
                    ERRORF(r, "%s's frame %i has wrong alpha type! expected: %s\tactual: %s",
                           rec.fName, i, to_string(expectedAlpha), to_string(alpha));
                }

                if (0 == i) {
                    REPORTER_ASSERT(r, frameInfo.fRequiredFrame == SkCodec::kNoFrame);
                } else if (rec.fRequiredFrames[i-1] != frameInfo.fRequiredFrame) {
                    ERRORF(r, "%s's frame %i has wrong dependency! expected: %i\tactual: %i",
                           rec.fName, i, rec.fRequiredFrames[i-1], frameInfo.fRequiredFrame);
                }

                REPORTER_ASSERT(r, frameInfo.fDisposalMethod == rec.fDisposalMethods[i]);
            }

            if (TestMode::kIndividual == mode) {
                // No need to test decoding twice.
                continue;
            }

            // Compare decoding in multiple ways:
            // - Start from scratch for each frame. |codec| will have to decode the required frame
            //   (and any it depends on) to decode. This is stored in |cachedFrames|.
            // - Provide the frame that a frame depends on, so |codec| just has to blend.
            // - Provide a frame after the required frame, which will be covered up by the newest
            //   frame.
            // All should look the same.
            std::vector<SkBitmap> cachedFrames(frameCount);
            const auto info = codec->getInfo().makeColorType(kN32_SkColorType);

            auto decode = [&](SkBitmap* bm, int index, int cachedIndex) {
                auto decodeInfo = info;
                if (index > 0) {
                    decodeInfo = info.makeAlphaType(frameInfos[index].fAlphaType);
                }
                bm->allocPixels(decodeInfo);
                if (cachedIndex != SkCodec::kNoFrame) {
                    // First copy the pixels from the cached frame
                    const bool success =
                            ToolUtils::copy_to(bm, kN32_SkColorType, cachedFrames[cachedIndex]);
                    REPORTER_ASSERT(r, success);
                }
                SkCodec::Options opts;
                opts.fFrameIndex = index;
                opts.fPriorFrame = cachedIndex;
                const auto result = codec->getPixels(decodeInfo, bm->getPixels(), bm->rowBytes(),
                                                     &opts);
                if (cachedIndex != SkCodec::kNoFrame &&
                        restore_previous(frameInfos[cachedIndex])) {
                    if (result == SkCodec::kInvalidParameters) {
                        return true;
                    }
                    ERRORF(r, "Using a kRestorePrevious frame as fPriorFrame should fail");
                    return false;
                }
                if (result != SkCodec::kSuccess) {
                    ERRORF(r, "Failed to decode frame %i from %s when providing prior frame %i, "
                              "error %i", index, rec.fName, cachedIndex, result);
                }
                return result == SkCodec::kSuccess;
            };

            for (int i = 0; i < frameCount; i++) {
                SkBitmap& cachedFrame = cachedFrames[i];
                if (!decode(&cachedFrame, i, SkCodec::kNoFrame)) {
                    continue;
                }
                const auto reqFrame = frameInfos[i].fRequiredFrame;
                if (reqFrame == SkCodec::kNoFrame) {
                    // Nothing to compare against.
                    continue;
                }
                for (int j = reqFrame; j < i; j++) {
                    SkBitmap frame;
                    if (restore_previous(frameInfos[j])) {
                        (void) decode(&frame, i, j);
                        continue;
                    }
                    if (!decode(&frame, i, j)) {
                        continue;
                    }

                    // Now verify they're equal.
                    const size_t rowLen = info.bytesPerPixel() * info.width();
                    for (int y = 0; y < info.height(); y++) {
                        const void* cachedAddr = cachedFrame.getAddr(0, y);
                        SkASSERT(cachedAddr != nullptr);
                        const void* addr = frame.getAddr(0, y);
                        SkASSERT(addr != nullptr);
                        const bool lineMatches = memcmp(cachedAddr, addr, rowLen) == 0;
                        if (!lineMatches) {
                            SkString name = SkStringPrintf("cached_%i", i);
                            write_bm(name.c_str(), cachedFrame);
                            name = SkStringPrintf("frame_%i", i);
                            write_bm(name.c_str(), frame);
                            ERRORF(r, "%s's frame %i is different (starting from line %i) when "
                                      "providing prior frame %i!", rec.fName, i, y, j);
                            break;
                        }
                    }
                }
            }
        }
    }
}

// Verify that a webp image can be animated scaled down. This image has a
// kRestoreBG frame, so it is an interesting image to test. After decoding that
// frame, we have to erase its rectangle. The rectangle has to be adjusted
// based on the scaled size.
DEF_TEST(AndroidCodec_animated, r) {
    if (GetResourcePath().isEmpty()) {
        return;
    }

    const char* file = "images/required.webp";
    sk_sp<SkData> data(GetResourceAsData(file));
    if (!data) {
        ERRORF(r, "Missing %s", file);
        return;
    }

    auto codec = SkAndroidCodec::MakeFromCodec(SkCodec::MakeFromData(std::move(data)));
    if (!codec) {
        ERRORF(r, "Failed to decode %s", file);
        return;
    }

    auto info = codec->getInfo().makeAlphaType(kPremul_SkAlphaType);

    for (int sampleSize : { 8, 32, 100 }) {
        auto dimensions = codec->codec()->getScaledDimensions(1.0f / sampleSize);
        info = info.makeWH(dimensions.width(), dimensions.height());
        SkBitmap bm;
        bm.allocPixels(info);

        SkCodec::Options options;
        for (int i = 0; i < codec->codec()->getFrameCount(); ++i) {
            SkCodec::FrameInfo frameInfo;
            REPORTER_ASSERT(r, codec->codec()->getFrameInfo(i, &frameInfo));
            if (5 == i) {
                REPORTER_ASSERT(r, frameInfo.fDisposalMethod
                        == SkCodecAnimation::DisposalMethod::kRestoreBGColor);
            }
            options.fFrameIndex = i;
            options.fPriorFrame = i - 1;
            info = info.makeAlphaType(frameInfo.fAlphaType);

            auto result = codec->codec()->getPixels(info, bm.getPixels(), bm.rowBytes(),
                                                    &options);
            REPORTER_ASSERT(r, result == SkCodec::kSuccess);

            // Now compare to not using prior frame.
            SkBitmap bm2;
            bm2.allocPixels(info);

            options.fPriorFrame = SkCodec::kNoFrame;
            result = codec->codec()->getPixels(info, bm2.getPixels(), bm2.rowBytes(),
                                               &options);
            REPORTER_ASSERT(r, result == SkCodec::kSuccess);

            for (int y = 0; y < info.height(); ++y) {
                if (memcmp(bm.getAddr32(0, y), bm2.getAddr32(0, y), info.minRowBytes())) {
                    ERRORF(r, "pixel mismatch for sample size %i, frame %i resulting in "
                              "dimensions %i x %i line %i\n",
                              sampleSize, i, info.width(), info.height(), y);
                    break;
                }
            }
        }
    }
}

DEF_TEST(AnimCodecPlayer, r) {
    static constexpr struct {
        const char* fFile;
        uint32_t    fDuration;
        SkISize     fSize;
    } gTests[] = {
        { "images/alphabetAnim.gif", 1300, {100, 100} },
        { "images/randPixels.gif"  ,    0, {  8,   8} },
        { "images/randPixels.jpg"  ,    0, {  8,   8} },
        { "images/randPixels.png"  ,    0, {  8,   8} },
    };

    for (const auto& test : gTests) {
        auto codec = SkCodec::MakeFromData(GetResourceAsData(test.fFile));
        REPORTER_ASSERT(r, codec);

        auto player = skstd::make_unique<SkAnimCodecPlayer>(std::move(codec));
        if (player->duration() != test.fDuration) {
            printf("*** %d vs %d\n", player->duration(), test.fDuration);
        }
        REPORTER_ASSERT(r, player->duration() == test.fDuration);

        auto f0 = player->getFrame();
        REPORTER_ASSERT(r, f0);
        REPORTER_ASSERT(r, f0->bounds().size() == test.fSize);

        player->seek(500);
        auto f1 = player->getFrame();
        REPORTER_ASSERT(r, f1);
        REPORTER_ASSERT(r, f1->bounds().size() == test.fSize);
    }
}
