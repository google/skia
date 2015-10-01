/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkTDArray.h"

/**
 * This is a conversion of samplecode/SampleChart.cpp into a bench. It sure would be nice to be able
 * to write one subclass that can be a GM, bench, and/or Sample.
 */

// Generates y values for the chart plots.
static void gen_data(SkScalar yAvg, SkScalar ySpread, int count,
                     SkRandom* random, SkTDArray<SkScalar>* dataPts) {
    dataPts->setCount(count);
    for (int i = 0; i < count; ++i) {
        (*dataPts)[i] = random->nextRangeScalar(yAvg - SkScalarHalf(ySpread),
                                                yAvg + SkScalarHalf(ySpread));
    }
}

// Generates a path to stroke along the top of each plot and a fill path for the area below each
// plot. The fill path is bounded below by the bottomData plot points or a horizontal line at
// yBase if bottomData == nullptr.
// The plots are animated by rotating the data points by leftShift.
static void gen_paths(const SkTDArray<SkScalar>& topData,
                      const SkTDArray<SkScalar>* bottomData,
                      SkScalar yBase,
                      SkScalar xLeft, SkScalar xDelta,
                      int leftShift,
                      SkPath* plot, SkPath* fill) {
    plot->rewind();
    fill->rewind();
    plot->incReserve(topData.count());
    if (nullptr == bottomData) {
        fill->incReserve(topData.count() + 2);
    } else {
        fill->incReserve(2 * topData.count());
    }

    leftShift %= topData.count();
    SkScalar x = xLeft;

    // Account for the leftShift using two loops
    int shiftToEndCount = topData.count() - leftShift;
    plot->moveTo(x, topData[leftShift]);
    fill->moveTo(x, topData[leftShift]);

    for (int i = 1; i < shiftToEndCount; ++i) {
        plot->lineTo(x, topData[i + leftShift]);
        fill->lineTo(x, topData[i + leftShift]);
        x += xDelta;
    }

    for (int i = 0; i < leftShift; ++i) {
        plot->lineTo(x, topData[i]);
        fill->lineTo(x, topData[i]);
        x += xDelta;
    }

    if (bottomData) {
        SkASSERT(bottomData->count() == topData.count());
        // iterate backwards over the previous graph's data to generate the bottom of the filled
        // area (and account for leftShift).
        for (int i = 0; i < leftShift; ++i) {
            x -= xDelta;
            fill->lineTo(x, (*bottomData)[leftShift - 1 - i]);
        }
        for (int i = 0; i < shiftToEndCount; ++i) {
            x -= xDelta;
            fill->lineTo(x, (*bottomData)[bottomData->count() - 1 - i]);
        }
    } else {
        fill->lineTo(x - xDelta, yBase);
        fill->lineTo(xLeft, yBase);
    }
}

// A set of scrolling line plots with the area between each plot filled. Stresses out GPU path
// filling
class ChartBench : public Benchmark {
public:
    ChartBench(bool aa) {
        fShift = 0;
        fAA = aa;
        fSize.fWidth = -1;
        fSize.fHeight = -1;
    }

protected:
    const char* onGetName() override {
        if (fAA) {
            return "chart_aa";
        } else {
            return "chart_bw";
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        bool sizeChanged = false;
        if (canvas->getDeviceSize() != fSize) {
            fSize = canvas->getDeviceSize();
            sizeChanged = true;
        }

        SkScalar ySpread = SkIntToScalar(fSize.fHeight / 20);

        SkScalar height = SkIntToScalar(fSize.fHeight);
        if (sizeChanged) {
            int dataPointCount = SkMax32(fSize.fWidth / kPixelsPerTick + 1, 2);

            SkRandom random;
            for (int i = 0; i < kNumGraphs; ++i) {
                SkScalar y = (kNumGraphs - i) * (height - ySpread) / (kNumGraphs + 1);
                fData[i].reset();
                gen_data(y, ySpread, dataPointCount, &random, fData + i);
            }
        }

        SkRandom colorRand;
        SkColor colors[kNumGraphs];
        for (int i = 0; i < kNumGraphs; ++i) {
            colors[i] = colorRand.nextU() | 0xff000000;
        }

        for (int frame = 0; frame < loops; ++frame) {

            canvas->clear(0xFFE0F0E0);

            SkPath plotPath;
            SkPath fillPath;

            static const SkScalar kStrokeWidth = SkIntToScalar(2);
            SkPaint plotPaint;
            SkPaint fillPaint;
            plotPaint.setAntiAlias(fAA);
            plotPaint.setStyle(SkPaint::kStroke_Style);
            plotPaint.setStrokeWidth(kStrokeWidth);
            plotPaint.setStrokeCap(SkPaint::kRound_Cap);
            plotPaint.setStrokeJoin(SkPaint::kRound_Join);
            fillPaint.setAntiAlias(fAA);
            fillPaint.setStyle(SkPaint::kFill_Style);

            SkTDArray<SkScalar>* prevData = nullptr;
            for (int i = 0; i < kNumGraphs; ++i) {
                gen_paths(fData[i],
                          prevData,
                          height,
                          0,
                          SkIntToScalar(kPixelsPerTick),
                          fShift,
                          &plotPath,
                          &fillPath);

                // Make the fills partially transparent
                fillPaint.setColor((colors[i] & 0x00ffffff) | 0x80000000);
                canvas->drawPath(fillPath, fillPaint);

                plotPaint.setColor(colors[i]);
                canvas->drawPath(plotPath, plotPaint);

                prevData = fData + i;
            }

            fShift += kShiftPerFrame;
        }
    }

private:
    enum {
        kNumGraphs = 5,
        kPixelsPerTick = 3,
        kShiftPerFrame = 1,
    };
    int                 fShift;
    SkISize             fSize;
    SkTDArray<SkScalar> fData[kNumGraphs];
    bool                fAA;

    typedef Benchmark INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new ChartBench(true); )
DEF_BENCH( return new ChartBench(false); )
