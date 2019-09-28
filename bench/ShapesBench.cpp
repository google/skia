/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkString.h"
#include "include/utils/SkRandom.h"
#include "tools/flags/CommandLineFlags.h"

#include <stdio.h>
#include <stdlib.h>
#include <functional>

#define ENABLE_COMMAND_LINE_SHAPES_BENCH 0

#if ENABLE_COMMAND_LINE_SHAPES_BENCH
static DEFINE_string(shapesType, "mixed",
                     "Type of shape to use in ShapesBench. Must be one of: "
                     "rect, oval, rrect, mixed.");
static DEFINE_string(innerShapesType, "none",
                     "Type of inner shape to use in ShapesBench. Must be one of: "
                     "none, rect, oval, rrect, mixed.");
static DEFINE_int(numShapes, 10000, "Number of shapes to draw in ShapesBench.");
static DEFINE_string(shapesSize, "32x32", "Size of shapes to draw in ShapesBench.");
static DEFINE_bool(shapesPersp, false, "Use slight perspective tilt in ShapesBench?");
#endif

/*
 * This class is used for several benchmarks that draw different primitive Skia shapes at various
 * sizes. It is used to test both CPU-bound and GPU-bound rendering situations. It draws large
 * amounts of shapes internally (rather than relying on nanobench selecting lots of loops) in order
 * to take advantage of instanced rendering approaches.
 */
class ShapesBench : public Benchmark {
public:
    enum ShapesType {
        kNone_ShapesType,
        kRect_ShapesType,
        kOval_ShapesType,
        kRRect_ShapesType,
        kMixed_ShapesType
    };

    ShapesBench(ShapesType shapesType, ShapesType innerShapesType,
                int numShapes, const SkISize& shapesSize, bool perspective)
        : fShapesType(shapesType)
        , fInnerShapesType(innerShapesType)
        , fNumShapes(numShapes)
        , fShapesSize(shapesSize)
        , fPerspective(perspective) {
        clampShapeSize();
    }

#if ENABLE_COMMAND_LINE_SHAPES_BENCH
    ShapesBench() {
        if (!strcmp(FLAGS_shapesType[0], "rect")) {
            fShapesType = kRect_ShapesType;
        } else if (!strcmp(FLAGS_shapesType[0], "oval")) {
            fShapesType = kOval_ShapesType;
        } else if (!strcmp(FLAGS_shapesType[0], "rrect")) {
            fShapesType = kRRect_ShapesType;
        } else if (!strcmp(FLAGS_shapesType[0], "mixed")) {
            fShapesType = kMixed_ShapesType;
        } else {
            SkDebugf("Invalid shapesType \"%s\". Must be one of: rect, oval, rrect, mixed.",
                     FLAGS_shapesType[0]);
            exit(-1);
        }
        if (!strcmp(FLAGS_innerShapesType[0], "none")) {
            fInnerShapesType = kNone_ShapesType;
        } else if (!strcmp(FLAGS_innerShapesType[0], "rect")) {
            fInnerShapesType = kRect_ShapesType;
        } else if (!strcmp(FLAGS_innerShapesType[0], "oval")) {
            fInnerShapesType = kOval_ShapesType;
        } else if (!strcmp(FLAGS_innerShapesType[0], "rrect")) {
            fInnerShapesType = kRRect_ShapesType;
        } else if (!strcmp(FLAGS_innerShapesType[0], "mixed")) {
            fInnerShapesType = kMixed_ShapesType;
        } else {
            SkDebugf("Invalid innerShapesType \"%s\". Must be one of: "
                     "none, rect, oval, rrect, mixed.", FLAGS_innerShapesType[0]);
            exit(-1);
        }
        if (2 != sscanf(FLAGS_shapesSize[0], "%ix%i", &fShapesSize.fWidth, &fShapesSize.fHeight)) {
            SkDebugf("Could not parse shapesSize from \"%s\". Expected \"%%ix%%i\"\n",
                     FLAGS_shapesSize[0]);
            exit(-1);
        }

        fNumShapes = FLAGS_numShapes;
        fPerspective = FLAGS_shapesPersp;

        clampShapeSize();
    }
#endif

private:
    void clampShapeSize() {
        float maxDiagonal = static_cast<float>(SkTMin(kBenchWidth, kBenchHeight));
        float diagonal = sqrtf(static_cast<float>(fShapesSize.width() * fShapesSize.width()) +
                               static_cast<float>(fShapesSize.height() * fShapesSize.height()));
        if (diagonal > maxDiagonal) {
            fShapesSize.fWidth = static_cast<int>(fShapesSize.width() * maxDiagonal / diagonal);
            fShapesSize.fHeight = static_cast<int>(fShapesSize.height() * maxDiagonal / diagonal);
        }
    }

    const char* onGetName() override {
        const char* shapeTypeNames[] = {
            "none", "rect", "oval", "rrect", "mixed"
        };

        fName.printf("shapes_%s", shapeTypeNames[fShapesType]);

        if (kNone_ShapesType != fInnerShapesType) {
            fName.appendf("_inner_%s", shapeTypeNames[fInnerShapesType]);
        }

        fName.appendf("_%i_%ix%i", fNumShapes, fShapesSize.width(), fShapesSize.height());

        if (fPerspective) {
            fName.append("_persp");
        }

        return fName.c_str();
    }
    SkIPoint onGetSize() override { return SkIPoint::Make(kBenchWidth, kBenchHeight); }

    void onDelayedSetup() override {
        SkScalar w = SkIntToScalar(fShapesSize.width());
        SkScalar h = SkIntToScalar(fShapesSize.height());

        fRect.setRect(SkRect::MakeXYWH(-w / 2, -h / 2, w, h));
        fOval.setOval(fRect.rect());
        fRRect.setNinePatch(fRect.rect(), w / 8, h / 13, w / 11, h / 7);

        if (kNone_ShapesType != fInnerShapesType) {
            fRect.inset(w / 7, h / 11, &fInnerRect);
            fInnerRect.offset(w / 28, h / 44);
            fInnerOval.setOval(fInnerRect.rect());
            fInnerRRect.setRectXY(fInnerRect.rect(), w / 13, w / 7);
        }

        SkRandom rand;
        fShapes.push_back_n(fNumShapes);
        for (int i = 0; i < fNumShapes; i++) {
            float pad = sqrtf(static_cast<float>(fShapesSize.width() * fShapesSize.width()) +
                              static_cast<float>(fShapesSize.height() * fShapesSize.height()));
            fShapes[i].fMatrix.setTranslate(0.5f * pad + rand.nextF() * (kBenchWidth - pad),
                                            0.5f * pad + rand.nextF() * (kBenchHeight - pad));
            fShapes[i].fMatrix.preRotate(rand.nextF() * 360.0f);
            if (fPerspective) {
                fShapes[i].fMatrix.setPerspX(0.00015f);
                fShapes[i].fMatrix.setPerspY(-0.00015f);
            }
            fShapes[i].fColor = rand.nextU() | 0xff808080;
        }
        for (int i = 0; i < fNumShapes; i++) {
            // Do this in a separate loop so mixed shapes get the same random numbers during
            // placement as non-mixed do.
            int shapeType = fShapesType;
            if (kMixed_ShapesType == shapeType) {
                shapeType = rand.nextRangeU(kRect_ShapesType, kRRect_ShapesType);
            }
            int innerShapeType = fInnerShapesType;
            if (kMixed_ShapesType == innerShapeType) {
                innerShapeType = rand.nextRangeU(kRect_ShapesType, kRRect_ShapesType);
            }
            if (kNone_ShapesType == innerShapeType) {
                switch (shapeType) {
                    using namespace std;
                    using namespace std::placeholders;
                    case kRect_ShapesType:
                        fShapes[i].fDraw = bind(&SkCanvas::drawRect, _1, cref(fRect.rect()), _2);
                        break;
                    case kOval_ShapesType:
                        fShapes[i].fDraw = bind(&SkCanvas::drawOval, _1, cref(fOval.rect()), _2);
                        break;
                    case kRRect_ShapesType:
                        fShapes[i].fDraw = bind(&SkCanvas::drawRRect, _1, cref(fRRect), _2);
                        break;
                }
            } else {
                const SkRRect* outer = nullptr;
                switch (shapeType) {
                    case kRect_ShapesType: outer = &fRect; break;
                    case kOval_ShapesType: outer = &fOval; break;
                    case kRRect_ShapesType: outer = &fRRect; break;
                }
                const SkRRect* inner = nullptr;
                switch (innerShapeType) {
                    case kRect_ShapesType: inner = &fInnerRect; break;
                    case kOval_ShapesType: inner = &fInnerOval; break;
                    case kRRect_ShapesType: inner = &fInnerRRect; break;
                }
                fShapes[i].fDraw = std::bind(&SkCanvas::drawDRRect, std::placeholders::_1,
                                             std::cref(*outer), std::cref(*inner),
                                             std::placeholders::_2);
            }
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;
        this->setupPaint(&paint);
        for (int j = 0; j < loops; j++) {
            for (int i = 0; i < fNumShapes; i++) {
                canvas->save();
                canvas->setMatrix(fShapes[i].fMatrix);
                paint.setColor(fShapes[i].fColor);
                fShapes[i].fDraw(canvas, paint);
                canvas->restore();
            }
        }
    }

    enum {
        kBenchWidth = 1000,
        kBenchHeight = 1000
    };

    struct ShapeInfo {
        SkMatrix   fMatrix;
        SkColor    fColor;
        std::function<void(SkCanvas*, const SkPaint&)> fDraw;
    };

    ShapesType            fShapesType;
    ShapesType            fInnerShapesType;
    int                   fNumShapes;
    SkISize               fShapesSize;
    bool                  fPerspective;
    SkString              fName;
    SkRRect               fRect;
    SkRRect               fOval;
    SkRRect               fRRect;
    SkRRect               fInnerRect;
    SkRRect               fInnerOval;
    SkRRect               fInnerRRect;
    SkTArray<ShapeInfo>   fShapes;


    typedef Benchmark INHERITED;
};

#if ENABLE_COMMAND_LINE_SHAPES_BENCH
DEF_BENCH(return new ShapesBench;)
#else
// Small primitives (CPU bound, in theory):
DEF_BENCH(return new ShapesBench(ShapesBench::kRect_ShapesType, ShapesBench::kNone_ShapesType,
                                 10000, SkISize::Make(32, 32), false);)
DEF_BENCH(return new ShapesBench(ShapesBench::kOval_ShapesType, ShapesBench::kNone_ShapesType,
                                 10000, SkISize::Make(32, 32), false);)
DEF_BENCH(return new ShapesBench(ShapesBench::kOval_ShapesType, ShapesBench::kNone_ShapesType,
                                 10000, SkISize::Make(32, 33), false);)
DEF_BENCH(return new ShapesBench(ShapesBench::kRRect_ShapesType, ShapesBench::kNone_ShapesType,
                                 10000, SkISize::Make(32, 32), false);)
DEF_BENCH(return new ShapesBench(ShapesBench::kMixed_ShapesType, ShapesBench::kNone_ShapesType,
                                 10000, SkISize::Make(32, 33), false);)

// Large primitives (GPU bound, in theory):
DEF_BENCH(return new ShapesBench(ShapesBench::kRect_ShapesType, ShapesBench::kNone_ShapesType,
                                 100, SkISize::Make(500, 500), false);)
DEF_BENCH(return new ShapesBench(ShapesBench::kOval_ShapesType, ShapesBench::kNone_ShapesType,
                                 100, SkISize::Make(500, 500), false);)
DEF_BENCH(return new ShapesBench(ShapesBench::kOval_ShapesType, ShapesBench::kNone_ShapesType,
                                 100, SkISize::Make(500, 501), false);)
DEF_BENCH(return new ShapesBench(ShapesBench::kRRect_ShapesType, ShapesBench::kNone_ShapesType,
                                 100, SkISize::Make(500, 500), false);)
DEF_BENCH(return new ShapesBench(ShapesBench::kMixed_ShapesType, ShapesBench::kNone_ShapesType,
                                 100, SkISize::Make(500, 501), false);)

// Donuts (small and large). These fall-back to path rendering due to non-orthogonal rotation
// making them quite slow. Thus, reduce the counts substantially:
DEF_BENCH(return new ShapesBench(ShapesBench::kRect_ShapesType, ShapesBench::kRect_ShapesType,
                                 500, SkISize::Make(32, 32), false);)
DEF_BENCH(return new ShapesBench(ShapesBench::kRRect_ShapesType, ShapesBench::kRRect_ShapesType,
                                 500, SkISize::Make(32, 32), false);)
DEF_BENCH(return new ShapesBench(ShapesBench::kRect_ShapesType, ShapesBench::kRect_ShapesType,
                                 50, SkISize::Make(500, 500), false);)
DEF_BENCH(return new ShapesBench(ShapesBench::kRRect_ShapesType, ShapesBench::kRRect_ShapesType,
                                 50, SkISize::Make(500, 500), false);)
#endif
