/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// for std::max
#include <algorithm>

#include "Benchmark.h"
#include "SkCurveMeasure.h"
#include "SkPath.h"
#include "SkPathMeasure.h"
#include "SkString.h"

#define NORMALIZE_LOOPS

class MeasureBench : public Benchmark {
   protected:
    SkString fName;

    SkPath fPath;

    bool fUsePathMeasure;
    float fSize;
    size_t fPieces;

    SkPoint fPts[3];

   public:
    MeasureBench(bool usePathMeasure, float size, size_t pieces)
        : fUsePathMeasure(usePathMeasure),
          fSize(size),
          fPieces(pieces) {
        fName.printf("measure_%s_%.1f_" SK_SIZE_T_SPECIFIER,
                     fUsePathMeasure ? "pathMeasure" : "curveMeasure", fSize,
                     fPieces);

        auto p1 = SkPoint::Make(0, 0);
        auto p2 = SkPoint::Make(30*fSize, 0);
        auto p3 = SkPoint::Make(15*fSize, 15*fSize);

        fPts[0] = p1;
        fPts[1] = p2;
        fPts[2] = p3;

        this->setPath();
    }

   protected:
    const char* onGetName() override { return fName.c_str(); }

    void setPath() {
        fPath.moveTo(fPts[0]);
        fPath.quadTo(fPts[1], fPts[2]);
    }

    int numLoops() {
#ifdef NORMALIZE_LOOPS
        // arbitrary heuristic
        return std::max(2, 10000 / ((int)fSize*(int)fPieces));
#else
        return 1000;
#endif // NORMALIZE_LOOPS
    }

    //// measurement code

    void do_pathMeasure(SkCanvas* canvas) {
        SkPathMeasure meas(fPath, false);

        SkScalar totalLength = meas.getLength();
        SkScalar pieceLength = totalLength / fPieces;

        SkPoint point;
        for (size_t i = 0; i <= fPieces; i++) {
            if (meas.getPosTan(i * pieceLength, &point, nullptr)) {
            };
        }
    }

    void do_curveMeasure(SkCanvas* canvas) {
        SkCurveMeasure meas(fPts, kQuad_SegType);

        SkScalar totalLength = meas.getLength();
        SkScalar pieceLength = totalLength / fPieces;

        SkPoint point;
        for (size_t i = 0; i <= fPieces; i++) {
            meas.getPosTanTime(i*pieceLength, &point, nullptr, nullptr);
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        int inner_loops = numLoops();
        for (int i = 0; i < loops; i++) {
            for (int j = 0; j < inner_loops; j++) {
                if (fUsePathMeasure) {
                    do_pathMeasure(canvas);
                }
                else {
                    do_curveMeasure(canvas);
                }
            }
        }
    }

   private:
    typedef Benchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH(return new MeasureBench(true, 1, 2);)
DEF_BENCH(return new MeasureBench(true, 2, 2);)
DEF_BENCH(return new MeasureBench(true, 10, 2);)
DEF_BENCH(return new MeasureBench(true, 100, 2);)
DEF_BENCH(return new MeasureBench(true, 1000, 2);)

DEF_BENCH(return new MeasureBench(true, 1, 1);)
DEF_BENCH(return new MeasureBench(true, 1, 2);)
DEF_BENCH(return new MeasureBench(true, 1, 3);)
DEF_BENCH(return new MeasureBench(true, 1, 4);)
DEF_BENCH(return new MeasureBench(true, 1, 5);)
DEF_BENCH(return new MeasureBench(true, 2, 1);)
DEF_BENCH(return new MeasureBench(true, 2, 2);)
DEF_BENCH(return new MeasureBench(true, 2, 3);)
DEF_BENCH(return new MeasureBench(true, 2, 4);)
DEF_BENCH(return new MeasureBench(true, 2, 5);)
DEF_BENCH(return new MeasureBench(true, 10, 10);)
DEF_BENCH(return new MeasureBench(true, 10, 20);)
DEF_BENCH(return new MeasureBench(true, 10, 30);)
DEF_BENCH(return new MeasureBench(true, 10, 40);)
DEF_BENCH(return new MeasureBench(true, 10, 50);)
DEF_BENCH(return new MeasureBench(true, 100, 100);)
DEF_BENCH(return new MeasureBench(true, 100, 200);)
DEF_BENCH(return new MeasureBench(true, 100, 300);)
DEF_BENCH(return new MeasureBench(true, 100, 400);)
DEF_BENCH(return new MeasureBench(true, 100, 500);)
DEF_BENCH(return new MeasureBench(true, 1000, 1000);)
DEF_BENCH(return new MeasureBench(true, 1000, 2000);)
DEF_BENCH(return new MeasureBench(true, 1000, 3000);)
DEF_BENCH(return new MeasureBench(true, 1000, 4000);)
DEF_BENCH(return new MeasureBench(true, 1000, 5000);)

DEF_BENCH(return new MeasureBench(false, 1, 2);)
DEF_BENCH(return new MeasureBench(false, 2, 2);)
DEF_BENCH(return new MeasureBench(false, 10, 2);)
DEF_BENCH(return new MeasureBench(false, 100, 2);)
DEF_BENCH(return new MeasureBench(false, 1000, 2);)

DEF_BENCH(return new MeasureBench(false, 1, 1);)
DEF_BENCH(return new MeasureBench(false, 1, 2);)
DEF_BENCH(return new MeasureBench(false, 1, 3);)
DEF_BENCH(return new MeasureBench(false, 1, 4);)
DEF_BENCH(return new MeasureBench(false, 1, 5);)
DEF_BENCH(return new MeasureBench(false, 2, 1);)
DEF_BENCH(return new MeasureBench(false, 2, 2);)
DEF_BENCH(return new MeasureBench(false, 2, 3);)
DEF_BENCH(return new MeasureBench(false, 2, 4);)
DEF_BENCH(return new MeasureBench(false, 2, 5);)
DEF_BENCH(return new MeasureBench(false, 10, 10);)
DEF_BENCH(return new MeasureBench(false, 10, 20);)
DEF_BENCH(return new MeasureBench(false, 10, 30);)
DEF_BENCH(return new MeasureBench(false, 10, 40);)
DEF_BENCH(return new MeasureBench(false, 10, 50);)
DEF_BENCH(return new MeasureBench(false, 100, 100);)
DEF_BENCH(return new MeasureBench(false, 100, 200);)
DEF_BENCH(return new MeasureBench(false, 100, 300);)
DEF_BENCH(return new MeasureBench(false, 100, 400);)
DEF_BENCH(return new MeasureBench(false, 100, 500);)
DEF_BENCH(return new MeasureBench(false, 1000, 1000);)
DEF_BENCH(return new MeasureBench(false, 1000, 2000);)
DEF_BENCH(return new MeasureBench(false, 1000, 3000);)
DEF_BENCH(return new MeasureBench(false, 1000, 4000);)
DEF_BENCH(return new MeasureBench(false, 1000, 5000);)
