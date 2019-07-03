/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/utils/SkRandom.h"
#include "samplecode/Sample.h"
#include "src/utils/SkUTF.h"
#if SK_SUPPORT_GPU
#include "src/gpu/GrRectanizer_pow2.h"
#include "src/gpu/GrRectanizer_skyline.h"

// This slide visualizes the various GrRectanizer-derived classes behavior
// for various input sets
//  'j' will cycle through the various rectanizers
//          Pow2 -> GrRectanizerPow2
//          Skyline -> GrRectanizerSkyline
//  'h' will cycle through the various rect sets
//          Rand -> random rects from 2-256
//          Pow2Rand -> random power of 2 sized rects from 2-256
//          SmallPow2 -> 128x128 rects
class RectanizerView : public Sample {
public:
    RectanizerView()
        : fCurRandRect(0)
        , fCurRectanizer(0) {
        for (int i = 0; i < 3; ++i) {
           fRects[i].setReserve(kNumRandRects);
        }
        fRectLocations.setReserve(kNumRandRects);

        SkRandom random;
        for (int i = 0; i < kNumRandRects; ++i) {
            *fRects[0].append() = SkISize::Make(random.nextRangeU(kMinRectSize, kMaxRectSize),
                                                random.nextRangeU(kMinRectSize, kMaxRectSize));
            *fRects[1].append() = SkISize::Make(
                        GrNextPow2(random.nextRangeU(kMinRectSize, kMaxRectSize)),
                        GrNextPow2(random.nextRangeU(kMinRectSize, kMaxRectSize)));
            *fRects[2].append() = SkISize::Make(128, 128);
            *fRectLocations.append() = SkIPoint16::Make(0, 0);
        }

        fCurRects = &fRects[0];

        fRectanizers.push_back(
            std::unique_ptr<GrRectanizer>(new GrRectanizerPow2(kWidth, kHeight)));
        fRectanizers.push_back(
            std::unique_ptr<GrRectanizer>(new GrRectanizerSkyline(kWidth, kHeight)));
    }

protected:
    SkString name() override { return SkString("Rectanizer"); }

    bool onQuery(Sample::Event* evt) override {
        SkUnichar uni;
        if (Sample::CharQ(*evt, &uni)) {
            char utf8[SkUTF::kMaxBytesInUTF8Sequence];
            size_t size = SkUTF::ToUTF8(uni, utf8);
            // Only consider events for single char keys
            if (1 == size) {
                switch (utf8[0]) {
                case kCycleRectanizerKey:
                    this->cycleRectanizer();
                    return true;
                case kCycleRectsKey:
                    this->cycleRects();
                    return true;
                default:
                    break;
                }
            }
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        if (fCurRandRect < kNumRandRects) {
            if (fRectanizers[fCurRectanizer]->addRect((*fCurRects)[fCurRandRect].fWidth,
                                                      (*fCurRects)[fCurRandRect].fHeight,
                                                      &fRectLocations[fCurRandRect])) {
                ++fCurRandRect;
            }
        }

        SkFont blackBigFont;
        blackBigFont.setSize(20);
        SkPaint blackStroke;
        blackStroke.setStyle(SkPaint::kStroke_Style);
        SkPaint redFill;
        redFill.setColor(SK_ColorRED);

        SkRect r = SkRect::MakeWH(SkIntToScalar(kWidth), SkIntToScalar(kHeight));

        canvas->clear(SK_ColorWHITE);
        canvas->drawRect(r, blackStroke);

        long totArea = 0;
        for (int i = 0; i < fCurRandRect; ++i) {
            r = SkRect::MakeXYWH(SkIntToScalar(fRectLocations[i].fX),
                                 SkIntToScalar(fRectLocations[i].fY),
                                 SkIntToScalar((*fCurRects)[i].fWidth),
                                 SkIntToScalar((*fCurRects)[i].fHeight));
            canvas->drawRect(r, redFill);
            canvas->drawRect(r, blackStroke);
            totArea += (*fCurRects)[i].fWidth * (*fCurRects)[i].fHeight;
        }

        SkString str;

        str.printf("%s-%s: tot Area: %ld %%full: %.2f (%.2f) numTextures: %d/%d",
                   this->getRectanizerName(),
                   this->getRectsName(),
                   totArea,
                   100.0f * fRectanizers[fCurRectanizer]->percentFull(),
                   100.0f * totArea / ((float)kWidth*kHeight),
                   fCurRandRect,
                   kNumRandRects);
        canvas->drawString(str, 50, kHeight + 50, blackBigFont, SkPaint());

        str.printf("Press \'j\' to toggle rectanizer");
        canvas->drawString(str, 50, kHeight + 100, blackBigFont, SkPaint());

        str.printf("Press \'h\' to toggle rects");
        canvas->drawString(str, 50, kHeight + 150, blackBigFont, SkPaint());
    }

private:
    static const int kWidth = 1024;
    static const int kHeight = 1024;
    static const int kNumRandRects = 200;
    static const char kCycleRectanizerKey = 'j';
    static const char kCycleRectsKey = 'h';
    static const int kMinRectSize = 2;
    static const int kMaxRectSize = 256;

    int                                     fCurRandRect;
    SkTDArray<SkISize>                      fRects[3];
    SkTDArray<SkISize>*                     fCurRects;
    SkTDArray<SkIPoint16>                   fRectLocations;
    SkTArray<std::unique_ptr<GrRectanizer>> fRectanizers;
    int                                     fCurRectanizer;

    const char* getRectanizerName() const {
        if (!fCurRectanizer) {
            return "Pow2";
        } else {
            return "Skyline";
        }
    }

    void cycleRectanizer() {
        fCurRectanizer = (fCurRectanizer + 1) % fRectanizers.count();

        fRectanizers[fCurRectanizer]->reset();
        fCurRandRect = 0;
    }

    const char* getRectsName() const {
        if (fCurRects == &fRects[0]) {
            return "Rand";
        } else if (fCurRects == &fRects[1]) {
            return "Pow2Rand";
        } else {
            return "SmallPow2";
        }
    }

    void cycleRects() {
        if (fCurRects == &fRects[0]) {
            fCurRects = &fRects[1];
        } else if (fCurRects == &fRects[1]) {
            fCurRects = &fRects[2];
        } else {
            fCurRects = &fRects[0];
        }

        fRectanizers[fCurRectanizer]->reset();
        fCurRandRect = 0;
    }

    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new RectanizerView(); )

#endif
