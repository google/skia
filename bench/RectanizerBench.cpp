/*
* Copyright 2014 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "Benchmark.h"
#include "SkRandom.h"
#include "SkSize.h"
#include "SkTDArray.h"

#if SK_SUPPORT_GPU

#include "GrRectanizer_pow2.h"
#include "GrRectanizer_skyline.h"

/**
 * This bench exercises Ganesh' GrRectanizer classes. It exercises the following
 * rectanizers:
 *      Pow2 Rectanizer
 *      Skyline Rectanizer
 * in the following cases:
 *      random rects (e.g., pull-save-layers forward use case)
 *      random power of two rects
 *      small constant sized power of 2 rects (e.g., glyph cache use case)
 */
class RectanizerBench : public Benchmark {
public:
    static const int kWidth = 1024;
    static const int kHeight = 1024;

    enum RectanizerType {
        kPow2_RectanizerType,
        kSkyline_RectanizerType,
    };

    enum RectType {
        kRand_RectType,
        kRandPow2_RectType,
        kSmallPow2_RectType
    };

    RectanizerBench(RectanizerType rectanizerType, RectType rectType) 
        : fName("rectanizer_")
        , fRectanizerType(rectanizerType)
        , fRectType(rectType) {

        if (kPow2_RectanizerType == fRectanizerType) {
            fName.append("pow2_");
        } else {
            SkASSERT(kSkyline_RectanizerType == fRectanizerType);
            fName.append("skyline_");
        }

        if (kRand_RectType == fRectType) {
            fName.append("rand");
        } else if (kRandPow2_RectType == fRectType) {
            fName.append("rand2");
        } else {
            SkASSERT(kSmallPow2_RectType == fRectType);
            fName.append("sm2");
        }
    }

protected:
    bool isSuitableFor(Backend backend) override {
        return kNonRendering_Backend == backend;
    }

    const char* onGetName() override {
        return fName.c_str();
    }

    void onDelayedSetup() override {
        SkASSERT(nullptr == fRectanizer.get());

        if (kPow2_RectanizerType == fRectanizerType) {
            fRectanizer.reset(new GrRectanizerPow2(kWidth, kHeight));
        } else {
            SkASSERT(kSkyline_RectanizerType == fRectanizerType);
            fRectanizer.reset(new GrRectanizerSkyline(kWidth, kHeight));
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkRandom rand;
        SkIPoint16 loc;
        SkISize size;

        for (int i = 0; i < loops; ++i) {
            if (kRand_RectType == fRectType) {
                size = SkISize::Make(rand.nextRangeU(1, kWidth / 2),
                                     rand.nextRangeU(1, kHeight / 2));
            } else if (kRandPow2_RectType == fRectType) {
                size = SkISize::Make(GrNextPow2(rand.nextRangeU(1, kWidth / 2)),
                                     GrNextPow2(rand.nextRangeU(1, kHeight / 2)));
            } else {
                SkASSERT(kSmallPow2_RectType == fRectType);
                size = SkISize::Make(128, 128);
            }

            if (!fRectanizer->addRect(size.fWidth, size.fHeight, &loc)) {
                // insert failed so clear out the rectanizer and give the
                // current rect another try
                fRectanizer->reset();
                i--;
            }
        }

        fRectanizer->reset();
    }

private:
    SkString                    fName;
    RectanizerType              fRectanizerType;
    RectType                    fRectType;
    SkAutoTDelete<GrRectanizer> fRectanizer;

    typedef Benchmark INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_BENCH(return new RectanizerBench(RectanizerBench::kPow2_RectanizerType,
                                     RectanizerBench::kRand_RectType);)
DEF_BENCH(return new RectanizerBench(RectanizerBench::kPow2_RectanizerType,
                                     RectanizerBench::kRandPow2_RectType);)
DEF_BENCH(return new RectanizerBench(RectanizerBench::kPow2_RectanizerType,
                                     RectanizerBench::kSmallPow2_RectType);)
DEF_BENCH(return new RectanizerBench(RectanizerBench::kSkyline_RectanizerType,
                                     RectanizerBench::kRand_RectType);)
DEF_BENCH(return new RectanizerBench(RectanizerBench::kSkyline_RectanizerType,
                                     RectanizerBench::kRandPow2_RectType);)
DEF_BENCH(return new RectanizerBench(RectanizerBench::kSkyline_RectanizerType,
                                     RectanizerBench::kSmallPow2_RectType);)

#endif
