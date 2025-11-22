/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathUtils.h"
#include "include/core/SkRRect.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTDArray.h"
#include "src/base/SkRandom.h"
#include "src/core/SkColorPriv.h"

#include "src/core/SkDraw.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkPathData.h"

using namespace skia_private;

enum Flags {
    kStroke_Flag = 1 << 0,
    kBig_Flag    = 1 << 1
};

#define FLAGS00  Flags(0)
#define FLAGS01  Flags(kStroke_Flag)
#define FLAGS10  Flags(kBig_Flag)
#define FLAGS11  Flags(kStroke_Flag | kBig_Flag)

class PathBench : public Benchmark {
    SkPaint     fPaint;
    SkString    fName;
    Flags       fFlags;
public:
    PathBench(Flags flags) : fFlags(flags) {
        fPaint.setStyle(flags & kStroke_Flag ? SkPaint::kStroke_Style :
                        SkPaint::kFill_Style);
        fPaint.setStrokeWidth(SkIntToScalar(5));
        fPaint.setStrokeJoin(SkPaint::kBevel_Join);
    }

    virtual void appendName(SkString*) = 0;
    virtual SkPath makePath() = 0;
    virtual int complexity() { return 0; }

protected:
    const char* onGetName() override {
        fName.printf("path_%s_%s_",
                     fFlags & kStroke_Flag ? "stroke" : "fill",
                     fFlags & kBig_Flag ? "big" : "small");
        this->appendName(&fName);
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint(fPaint);
        this->setupPaint(&paint);

        SkPath path = this->makePath();
        if (fFlags & kBig_Flag) {
            path = path.makeTransform(SkMatrix::Scale(10, 10));
        }

        for (int i = 0; i < loops; i++) {
            canvas->drawPath(path, paint);
        }
    }

private:
    using INHERITED = Benchmark;
};

class TrianglePathBench : public PathBench {
public:
    TrianglePathBench(Flags flags) : INHERITED(flags) {}

    void appendName(SkString* name) override {
        name->append("triangle");
    }
    SkPath makePath() override {
        static const SkPoint pts[] = {
            {10, 10}, {15, 5}, {20, 20}
        };
        return SkPath::Polygon(pts, true);
    }
private:
    using INHERITED = PathBench;
};

class RectPathBench : public PathBench {
public:
    RectPathBench(Flags flags) : INHERITED(flags) {}

    void appendName(SkString* name) override {
        name->append("rect");
    }
    SkPath makePath() override {
        return SkPath::Rect({10, 10, 20, 20});
    }
private:
    using INHERITED = PathBench;
};

class RotatedRectBench : public PathBench {
public:
    RotatedRectBench(Flags flags, bool aa, float degrees) : INHERITED(flags) {
        fAA = aa;
        fDegrees = degrees;
    }

    void appendName(SkString* name) override {
        SkString suffix;
        suffix.printf("rotated_rect_%s_%g", fAA ? "aa" : "noaa", fDegrees);
        name->append(suffix);
    }

    SkPath makePath() override {
        SkPath path = SkPath::Rect({10, 10, 20, 20});
        return path.makeTransform(SkMatrix::RotateDeg(fDegrees));
    }

    void setupPaint(SkPaint* paint) override {
        PathBench::setupPaint(paint);
        paint->setAntiAlias(fAA);
    }
private:
    using INHERITED = PathBench;
    float fDegrees;
    bool fAA;
};

class OvalPathBench : public PathBench {
public:
    OvalPathBench(Flags flags) : INHERITED(flags) {}

    void appendName(SkString* name) override {
        name->append("oval");
    }
    SkPath makePath() override {
        return SkPath::Oval({ 10, 10, 23, 20 });
    }
private:
    using INHERITED = PathBench;
};

class CirclePathBench: public PathBench {
public:
    CirclePathBench(Flags flags) : INHERITED(flags) {}

    void appendName(SkString* name) override {
        name->append("circle");
    }
    SkPath makePath() override {
        return SkPath::Circle(20, 20, 10);
    }
private:
    using INHERITED = PathBench;
};

class NonAACirclePathBench: public CirclePathBench {
public:
    NonAACirclePathBench(Flags flags) : INHERITED(flags) {}

    void appendName(SkString* name) override {
        name->append("nonaacircle");
    }

    void setupPaint(SkPaint* paint) override {
        CirclePathBench::setupPaint(paint);
        paint->setAntiAlias(false);
    }

private:
    using INHERITED = CirclePathBench;
};

// Test max speedup of Analytic AA for concave paths
class AAAConcavePathBench : public PathBench {
public:
    AAAConcavePathBench(Flags flags) : INHERITED(flags) {}

    void appendName(SkString* name) override {
        name->append("concave_aaa");
    }

    SkPath makePath() override {
        const SkPoint pts[] = {
            {10, 10}, {15, 10}, {15, 5}, {40, 40},
        };
        return SkPath::Polygon(pts, true);
    }

private:
    using INHERITED = PathBench;
};

// Test max speedup of Analytic AA for convex paths
class AAAConvexPathBench : public PathBench {
public:
    AAAConvexPathBench(Flags flags) : INHERITED(flags) {}

    void appendName(SkString* name) override {
        name->append("convex_aaa");
    }

    SkPath makePath() override {
        const SkPoint pts[] = {
            {10, 10}, {15, 10}, {40, 50},
        };
        return SkPath::Polygon(pts, true);
    }

private:
    using INHERITED = PathBench;
};

class SawToothPathBench : public PathBench {
public:
    SawToothPathBench(Flags flags) : INHERITED(flags) {}

    void appendName(SkString* name) override {
        name->append("sawtooth");
    }
    SkPath makePath() override {
        SkScalar x = SkIntToScalar(20);
        SkScalar y = SkIntToScalar(20);
        const SkScalar x0 = x;
        const SkScalar dx = SK_Scalar1 * 5;
        const SkScalar dy = SK_Scalar1 * 10;

        SkPathBuilder builder;
        builder.moveTo(x, y);
        for (int i = 0; i < 32; i++) {
            x += dx;
            builder.lineTo(x, y - dy);
            x += dx;
            builder.lineTo(x, y + dy);
        }
        builder.lineTo(x, y + 2 * dy);
        builder.lineTo(x0, y + 2 * dy);
        builder.close();
        return builder.detach();
    }
    int complexity() override { return 1; }
private:
    using INHERITED = PathBench;
};

class LongCurvedPathBench : public PathBench {
public:
    LongCurvedPathBench(Flags flags) : INHERITED(flags) {}

    void appendName(SkString* name) override {
        name->append("long_curved");
    }
    SkPath makePath() override {
        SkRandom rand (12);
        SkPathBuilder builder;
        for (int i = 0; i < 100; i++) {
            builder.quadTo(rand.nextUScalar1() * 640, rand.nextUScalar1() * 480,
                           rand.nextUScalar1() * 640, rand.nextUScalar1() * 480);
        }
        builder.close();
        return builder.detach();
    }
    int complexity() override { return 2; }
private:
    using INHERITED = PathBench;
};

class LongLinePathBench : public PathBench {
public:
    LongLinePathBench(Flags flags) : INHERITED(flags) {}

    void appendName(SkString* name) override {
        name->append("long_line");
    }
    SkPath makePath() override {
        SkRandom rand;
        SkPathBuilder builder;
        builder.moveTo(rand.nextUScalar1() * 640, rand.nextUScalar1() * 480);
        for (size_t i = 1; i < 100; i++) {
            builder.lineTo(rand.nextUScalar1() * 640, rand.nextUScalar1() * 480);
        }
        return builder.detach();
    }
    int complexity() override { return 2; }
private:
    using INHERITED = PathBench;
};

class RandomPathBench : public Benchmark {
public:
    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

protected:
    void createData(int minVerbs,
                    int maxVerbs,
                    bool allowMoves = true,
                    SkRect* bounds = nullptr) {
        SkRect tempBounds;
        if (nullptr == bounds) {
            tempBounds.setXYWH(0, 0, SK_Scalar1, SK_Scalar1);
            bounds = &tempBounds;
        }
        fVerbCnts.reset(kNumVerbCnts);
        for (int i = 0; i < kNumVerbCnts; ++i) {
            fVerbCnts[i] = fRandom.nextRangeU(minVerbs, maxVerbs + 1);
        }
        fVerbs.reset(kNumVerbs);
        for (int i = 0; i < kNumVerbs; ++i) {
            do {
                fVerbs[i] = static_cast<SkPathVerb>(fRandom.nextULessThan((int)SkPathVerb::kClose));
            } while (!allowMoves && SkPathVerb::kMove == fVerbs[i]);
        }
        fPoints.reset(kNumPoints);
        for (int i = 0; i < kNumPoints; ++i) {
            fPoints[i].set(fRandom.nextRangeScalar(bounds->fLeft, bounds->fRight),
                           fRandom.nextRangeScalar(bounds->fTop, bounds->fBottom));
        }
        this->restartMakingPaths();
    }

    void restartMakingPaths() {
        fCurrPath = 0;
        fCurrVerb = 0;
        fCurrPoint = 0;
    }

    SkPath makePath() {
        SkPathBuilder builder;
        int vCount = fVerbCnts[(fCurrPath++) & (kNumVerbCnts - 1)];
        for (int v = 0; v < vCount; ++v) {
            SkPathVerb verb = fVerbs[(fCurrVerb++) & (kNumVerbs - 1)];
            switch (verb) {
                case SkPathVerb::kMove:
                    builder.moveTo(fPoints[(fCurrPoint++) & (kNumPoints - 1)]);
                    break;
                case SkPathVerb::kLine:
                    builder.lineTo(fPoints[(fCurrPoint++) & (kNumPoints - 1)]);
                    break;
                case SkPathVerb::kQuad:
                    builder.quadTo(fPoints[(fCurrPoint + 0) & (kNumPoints - 1)],
                                   fPoints[(fCurrPoint + 1) & (kNumPoints - 1)]);
                    fCurrPoint += 2;
                    break;
                case SkPathVerb::kConic:
                    builder.conicTo(fPoints[(fCurrPoint + 0) & (kNumPoints - 1)],
                                    fPoints[(fCurrPoint + 1) & (kNumPoints - 1)],
                                    SK_ScalarHalf);
                    fCurrPoint += 2;
                    break;
                case SkPathVerb::kCubic:
                    builder.cubicTo(fPoints[(fCurrPoint + 0) & (kNumPoints - 1)],
                                    fPoints[(fCurrPoint + 1) & (kNumPoints - 1)],
                                    fPoints[(fCurrPoint + 2) & (kNumPoints - 1)]);
                    fCurrPoint += 3;
                    break;
                case SkPathVerb::kClose:
                    builder.close();
                    break;
                default:
                    SkDEBUGFAIL("Unexpected path verb");
                    break;
            }
        }
        return builder.detach();
    }

    void finishedMakingPaths() {
        fVerbCnts.reset(0);
        fVerbs.reset(0);
        fPoints.reset(0);
    }

private:
    enum {
        // these should all be pow 2
        kNumVerbCnts = 1 << 5,
        kNumVerbs    = 1 << 5,
        kNumPoints   = 1 << 5,
    };
    AutoTArray<int>         fVerbCnts;
    AutoTArray<SkPathVerb>  fVerbs;
    AutoTArray<SkPoint>     fPoints;
    int                     fCurrPath;
    int                     fCurrVerb;
    int                     fCurrPoint;
    SkRandom                fRandom;
    using INHERITED = Benchmark;
};

class PathCreateBench : public RandomPathBench {
public:
    PathCreateBench()  {
    }

protected:
    const char* onGetName() override {
        return "path_create";
    }

    void onDelayedSetup() override {
        this->createData(10, 100);
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; ++i) {
            (void)this->makePath();
        }
        this->restartMakingPaths();
    }

private:
    using INHERITED = RandomPathBench;
};

class PathCopyBench : public RandomPathBench {
public:
    PathCopyBench()  {
    }

protected:
    const char* onGetName() override {
        return "path_copy";
    }
    void onDelayedSetup() override {
        this->createData(10, 100);
        fPaths.reset(kPathCnt);
        fCopies.reset(kPathCnt);
        for (int i = 0; i < kPathCnt; ++i) {
            fPaths[i] = this->makePath();
        }
        this->finishedMakingPaths();
    }
    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; ++i) {
            int idx = i & (kPathCnt - 1);
            fCopies[idx] = fPaths[idx];
        }
    }

private:
    enum {
        // must be a pow 2
        kPathCnt = 1 << 5,
    };
    AutoTArray<SkPath> fPaths;
    AutoTArray<SkPath> fCopies;

    using INHERITED = RandomPathBench;
};

enum class BenchPathType {
    kPath,
    kBuilder,
    kData,
};
const char* gBenchPathTypeNames[] = { "path", "builder", "data" };

class PathTransformBench : public Benchmark {
public:
    PathTransformBench(BenchPathType t, bool p) : fType(t), fPerspective(p) {
        const char* mx = fPerspective ? "persp" : "affine";
        fName.printf("path_transform_%s_%s", mx, gBenchPathTypeNames[(int)fType]);
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

    void onDelayedSetup() override {
        const SkRect r = {0, 0, 100, 100};
        fBuilderSrc.addOval(r);
        fBuilderSrc.addOval(r.makeInset(10, 10));
        fPathSrc = fBuilderSrc.snapshot();
        fPData = fBuilderSrc.snapshotData();

        if (fPerspective) {
            fMatrix[6] = 0.0000001f;
        } else {
            fMatrix[0] = 1.000001f;
        }
    }

    void onDraw(int loops, SkCanvas*) override {
        // we ask for bounds each time, to ensure we're playing fair, as some
        // techniques compute bounds up front after a transform, and some defer
        // it until it is first requested.
        for (int i = 0; i < loops; ++i) {
            switch (fType) {
                case BenchPathType::kPath:
                    fPathSrc.makeTransform(fMatrix).getBounds();
                    break;
                case BenchPathType::kBuilder:
                    fBuilderSrc.transform(fMatrix);
                    fBuilderSrc.snapshot().getBounds();
                    break;
                case BenchPathType::kData:
                    fPData->makeTransform(fMatrix)->bounds();
                    break;
                }
        }
    }

private:
    SkPath          fPathSrc;
    SkPathBuilder   fBuilderSrc;
    sk_sp<SkPathData> fPData;

    SkMatrix      fMatrix;
    BenchPathType fType;
    bool          fPerspective;
    SkString      fName;
};

static void builder_from_rect(const SkRRect& r) {
    SkPathBuilder bu; bu.addRect(r.rect()); (void)bu.detach();
}
static void builder_from_oval(const SkRRect& r) {
    SkPathBuilder bu; bu.addOval(r.rect()); (void)bu.detach();
}
static void builder_from_rrect(const SkRRect& r) {
    SkPathBuilder bu; bu.addRRect(r); (void)bu.detach();
}

static void path_from_rect(const SkRRect& r) {
    (void)SkPath::Rect(r.rect());
}
static void path_from_oval(const SkRRect& r) {
    (void)SkPath::Oval(r.rect());
}
static void path_from_rrect(const SkRRect& r) {
    (void)SkPath::RRect(r);
}

static void pdata_from_rect(const SkRRect& r) {
    (void)SkPathData::Rect(r.rect());
}
static void pdata_from_oval(const SkRRect& r) {
    (void)SkPathData::Oval(r.rect());
}
static void pdata_from_rrect(const SkRRect& r) {
    (void)SkPathData::RRect(r);
}

class PathMakeFromBench : public Benchmark {
public:
    using MakeFrom = void(const SkRRect&);

    PathMakeFromBench(const char name[], MakeFrom* proc) : fMaker(proc) {
        fName.printf("pathmaker_%s", name);
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

    void onDraw(int loops, SkCanvas*) override {
        const SkRRect rr = SkRRect::MakeRectXY({10, 20, 30, 40}, 2, 3);
        for (int i = 0; i < loops; ++i) {
            fMaker(rr);
        }
    }

private:
    MakeFrom*   fMaker;
    SkString    fName;
};

class PathEqualityBench : public RandomPathBench {
public:
    PathEqualityBench() { }

protected:
    const char* onGetName() override {
        return "path_equality_50%";
    }

    void onDelayedSetup() override {
        fParity = 0;
        this->createData(10, 100);
        fPaths.reset(kPathCnt);
        fCopies.reset(kPathCnt);
        for (int i = 0; i < kPathCnt; ++i) {
            fPaths[i] = this->makePath();
            fCopies[i] = fPaths[i];
        }
        this->finishedMakingPaths();
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; ++i) {
            int idx = i & (kPathCnt - 1);
            fParity ^= (fPaths[idx] == fCopies[idx & ~0x1]);
        }
    }

private:
    bool fParity; // attempt to keep compiler from optimizing out the ==
    enum {
        // must be a pow 2
        kPathCnt = 1 << 5,
    };
    AutoTArray<SkPath> fPaths;
    AutoTArray<SkPath> fCopies;
    using INHERITED = RandomPathBench;
};

#ifndef SK_HIDE_PATH_EDIT_METHODS
class SkBench_AddPathTest : public RandomPathBench {
public:
    enum AddType {
        kAdd_AddType,
        kAddTrans_AddType,
        kAddMatrix_AddType,
        kReverseAdd_AddType,
    };

    SkBench_AddPathTest(AddType type) : fType(type) {
        fMatrix.setRotate(60 * SK_Scalar1);
    }

protected:
    const char* onGetName() override {
        switch (fType) {
            case kAdd_AddType:
                return "path_add_path";
            case kAddTrans_AddType:
                return "path_add_path_trans";
            case kAddMatrix_AddType:
                return "path_add_path_matrix";
            case kReverseAdd_AddType:
                return "path_reverse_add_path";
            default:
                SkDEBUGFAIL("Bad add type");
                return "";
        }
    }

    void onDelayedSetup() override {
        this->createData(10, 100, true);
        fPaths0.reset(kPathCnt);
        fPaths1.reset(kPathCnt);
        for (int i = 0; i < kPathCnt; ++i) {
            fPaths0[i] = this->makePath();
            fPaths1[i] = this->makePath();
        }
        this->finishedMakingPaths();
    }

    void onDraw(int loops, SkCanvas*) override {
        switch (fType) {
            case kAdd_AddType:
                for (int i = 0; i < loops; ++i) {
                    int idx = i & (kPathCnt - 1);
                    SkPath result = fPaths0[idx];
                    result.addPath(fPaths1[idx]);
                }
                break;
            case kAddTrans_AddType:
                for (int i = 0; i < loops; ++i) {
                    int idx = i & (kPathCnt - 1);
                    SkPath result = fPaths0[idx];
                    result.addPath(fPaths1[idx], 2 * SK_Scalar1, 5 * SK_Scalar1);
                }
                break;
            case kAddMatrix_AddType:
                for (int i = 0; i < loops; ++i) {
                    int idx = i & (kPathCnt - 1);
                    SkPath result = fPaths0[idx];
                    result.addPath(fPaths1[idx], fMatrix);
                }
                break;
            case kReverseAdd_AddType:
                for (int i = 0; i < loops; ++i) {
                    int idx = i & (kPathCnt - 1);
                    SkPath result = fPaths0[idx];
                    result.reverseAddPath(fPaths1[idx]);
                }
                break;
        }
    }

private:
    AddType fType; // or reverseAddPath
    enum {
        // must be a pow 2
        kPathCnt = 1 << 5,
    };
    AutoTArray<SkPath> fPaths0;
    AutoTArray<SkPath> fPaths1;
    SkMatrix         fMatrix;
    using INHERITED = RandomPathBench;
};
#endif

class CirclesBench : public Benchmark {
protected:
    SkString            fName;
    Flags               fFlags;

public:
    CirclesBench(Flags flags) : fFlags(flags) {
        fName.printf("circles_%s", fFlags & kStroke_Flag ? "stroke" : "fill");
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;

        paint.setColor(SK_ColorBLACK);
        paint.setAntiAlias(true);
        if (fFlags & kStroke_Flag) {
            paint.setStyle(SkPaint::kStroke_Style);
        }

        SkRandom rand;

        SkRect r;

        for (int i = 0; i < loops; ++i) {
            SkScalar radius = rand.nextUScalar1() * 3;
            r.fLeft = rand.nextUScalar1() * 300;
            r.fTop =  rand.nextUScalar1() * 300;
            r.fRight =  r.fLeft + 2 * radius;
            r.fBottom = r.fTop + 2 * radius;

            if (fFlags & kStroke_Flag) {
                paint.setStrokeWidth(rand.nextUScalar1() * 5.0f);
            }

            // mimic how Chrome does circles
            SkPath temp = SkPathBuilder()
                          .arcTo(r, 0, 0, false)
                          .addOval(r, SkPathDirection::kCCW)
                          .arcTo(r, 360, 0, true)
                          .close()
                          .detach();

            canvas->drawPath(temp, paint);
        }
    }

private:
    using INHERITED = Benchmark;
};


// Chrome creates its own round rects with each corner possibly being different.
// In its "zero radius" incarnation it creates degenerate round rects.
// Note: PathTest::test_arb_round_rect_is_convex and
// test_arb_zero_rad_round_rect_is_rect perform almost exactly
// the same test (but with no drawing)
class ArbRoundRectBench : public Benchmark {
protected:
    SkString            fName;

public:
    ArbRoundRectBench(bool zeroRad) : fZeroRad(zeroRad) {
        if (zeroRad) {
            fName.printf("zeroradroundrect");
        } else {
            fName.printf("arbroundrect");
        }
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    static void add_corner_arc(SkPathBuilder* builder, const SkRect& rect,
                               SkScalar xIn, SkScalar yIn,
                               int startAngle)
    {

        SkScalar rx = std::min(rect.width(), xIn);
        SkScalar ry = std::min(rect.height(), yIn);

        SkRect arcRect;
        arcRect.setLTRB(-rx, -ry, rx, ry);
        switch (startAngle) {
        case 0:
            arcRect.offset(rect.fRight - arcRect.fRight, rect.fBottom - arcRect.fBottom);
            break;
        case 90:
            arcRect.offset(rect.fLeft - arcRect.fLeft, rect.fBottom - arcRect.fBottom);
            break;
        case 180:
            arcRect.offset(rect.fLeft - arcRect.fLeft, rect.fTop - arcRect.fTop);
            break;
        case 270:
            arcRect.offset(rect.fRight - arcRect.fRight, rect.fTop - arcRect.fTop);
            break;
        default:
            break;
        }

        builder->arcTo(arcRect, SkIntToScalar(startAngle), SkIntToScalar(90), false);
    }

    static SkPath make_arb_round_rect(const SkRect& r, SkScalar xCorner, SkScalar yCorner) {
        SkPathBuilder builder;
        // we are lazy here and use the same x & y for each corner
        add_corner_arc(&builder, r, xCorner, yCorner, 270);
        add_corner_arc(&builder, r, xCorner, yCorner, 0);
        add_corner_arc(&builder, r, xCorner, yCorner, 90);
        add_corner_arc(&builder, r, xCorner, yCorner, 180);
        builder.close();

        SkPath path = builder.detach();
        SkASSERT(path.isConvex());
        return path;
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkRandom rand;
        SkRect r;

        for (int i = 0; i < loops; ++i) {
            SkPaint paint;
            paint.setColor(0xff000000 | rand.nextU());
            paint.setAntiAlias(true);

            SkScalar size = rand.nextUScalar1() * 30;
            if (size < SK_Scalar1) {
                continue;
            }
            r.fLeft = rand.nextUScalar1() * 300;
            r.fTop =  rand.nextUScalar1() * 300;
            r.fRight =  r.fLeft + 2 * size;
            r.fBottom = r.fTop + 2 * size;

            SkPath temp;

            if (fZeroRad) {
                temp = make_arb_round_rect(r, 0, 0);

                SkASSERT(temp.isRect(nullptr));
            } else {
                temp = make_arb_round_rect(r, r.width() / 10, r.height() / 15);
            }

            canvas->drawPath(temp, paint);
        }
    }

private:
    bool fZeroRad;      // should 0 radius rounds rects be tested?

    using INHERITED = Benchmark;
};

class ConservativelyContainsBench : public Benchmark {
public:
    enum Type {
        kRect_Type,
        kRoundRect_Type,
        kOval_Type,
    };

    ConservativelyContainsBench(Type type)  {
        fParity = false;
        fName = "conservatively_contains_";
        switch (type) {
            case kRect_Type:
                fName.append("rect");
                fPath = SkPath::Rect(kBaseRect);
                break;
            case kRoundRect_Type:
                fName.append("round_rect");
                fPath = SkPath::RRect(kBaseRect, kRRRadii[0], kRRRadii[1]);
                break;
            case kOval_Type:
                fName.append("oval");
                fPath = SkPath::Oval(kBaseRect);
                break;
        }
    }

    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

private:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; ++i) {
            const SkRect& rect = fQueryRects[i % kQueryRectCnt];
            fParity = fParity != fPath.conservativelyContainsRect(rect);
        }
    }

    void onDelayedSetup() override {
        fQueryRects.resize(kQueryRectCnt);

        SkRandom rand;
        for (int i = 0; i < kQueryRectCnt; ++i) {
            SkSize size;
            SkPoint xy;
            size.fWidth = rand.nextRangeScalar(kQueryMin.fWidth,  kQueryMax.fWidth);
            size.fHeight = rand.nextRangeScalar(kQueryMin.fHeight, kQueryMax.fHeight);
            xy.fX = rand.nextRangeScalar(kBounds.fLeft, kBounds.fRight - size.fWidth);
            xy.fY = rand.nextRangeScalar(kBounds.fTop, kBounds.fBottom - size.fHeight);

            fQueryRects[i] = SkRect::MakeXYWH(xy.fX, xy.fY, size.fWidth, size.fHeight);
        }
    }

    enum {
        kQueryRectCnt = 400,
    };
    static const SkRect kBounds;   // bounds for all random query rects
    static const SkSize kQueryMin; // minimum query rect size, should be <= kQueryMax
    static const SkSize kQueryMax; // max query rect size, should < kBounds
    static const SkRect kBaseRect; // rect that is used to construct the path
    static const SkScalar kRRRadii[2]; // x and y radii for round rect

    SkString            fName;
    SkPath              fPath;
    bool                fParity;
    SkTDArray<SkRect>   fQueryRects;

    using INHERITED = Benchmark;
};

///////////////////////////////////////////////////////////////////////////////

#include "src/core/SkGeometry.h"

class ConicBench_Chop : public Benchmark {
protected:
    SkConic fRQ, fDst[2];
    SkString fName;
public:
    ConicBench_Chop() : fName("conic-chop") {
        fRQ.fPts[0].set(0, 0);
        fRQ.fPts[1].set(100, 0);
        fRQ.fPts[2].set(100, 100);
        fRQ.fW = SkScalarCos(SK_ScalarPI/4);
    }

    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

private:
    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; ++i) {
            fRQ.chop(fDst);
        }
    }

    using INHERITED = Benchmark;
};
DEF_BENCH( return new ConicBench_Chop; )

class ConicBench_EvalPos : public ConicBench_Chop {
    const bool fUseV2;
public:
    ConicBench_EvalPos(bool useV2) : fUseV2(useV2) {
        fName.printf("conic-eval-pos%d", useV2);
    }
    void onDraw(int loops, SkCanvas*) override {
        if (fUseV2) {
            for (int i = 0; i < loops; ++i) {
                for (int j = 0; j < 1000; ++j) {
                    fDst[0].fPts[0] = fRQ.evalAt(0.4f);
                }
            }
        } else {
            for (int i = 0; i < loops; ++i) {
                for (int j = 0; j < 1000; ++j) {
                    fRQ.evalAt(0.4f, &fDst[0].fPts[0], nullptr);
                }
            }
        }
    }
};
DEF_BENCH( return new ConicBench_EvalPos(false); )
DEF_BENCH( return new ConicBench_EvalPos(true); )

class ConicBench_EvalTan : public ConicBench_Chop {
    const bool fUseV2;
public:
    ConicBench_EvalTan(bool useV2) : fUseV2(useV2) {
        fName.printf("conic-eval-tan%d", useV2);
    }
    void onDraw(int loops, SkCanvas*) override {
        if (fUseV2) {
            for (int i = 0; i < loops; ++i) {
                for (int j = 0; j < 1000; ++j) {
                    fDst[0].fPts[0] = fRQ.evalTangentAt(0.4f);
                }
            }
        } else {
            for (int i = 0; i < loops; ++i) {
                for (int j = 0; j < 1000; ++j) {
                    fRQ.evalAt(0.4f, nullptr, &fDst[0].fPts[0]);
                }
            }
        }
    }
};
DEF_BENCH( return new ConicBench_EvalTan(false); )
DEF_BENCH( return new ConicBench_EvalTan(true); )

class ConicBench_TinyError : public Benchmark {
protected:
    SkString fName;

public:
    ConicBench_TinyError() : fName("conic-tinyerror") {}

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas*) override {
        SkPaint paint;
        paint.setColor(SK_ColorBLACK);
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(2);

        SkPath path = SkPathBuilder()
                      .moveTo(-100, 1)
                      .cubicTo(-101, 1, -118, -47, -138, -44)
                      .detach();

        // The large y scale factor produces a tiny error threshold.
        const SkMatrix mtx = SkMatrix::MakeAll(3.07294035f, 0.833333373f, 361.111115f, 0.0f,
                                               6222222.5f, 28333.334f, 0.0f, 0.0f, 1.0f);
        const SkScalar scale = SkMatrixPriv::ComputeResScaleForStroking(mtx);
        const SkMatrix mx = SkMatrix::Scale(scale, scale);

        for (int i = 0; i < loops; ++i) {
            SkPathBuilder dst;
            skpathutils::FillPathWithPaint(path, paint, &dst, nullptr, mx);
        }
    }

private:
    using INHERITED = Benchmark;
};
DEF_BENCH( return new ConicBench_TinyError; )

///////////////////////////////////////////////////////////////////////////////

static void rand_conic(SkConic* conic, SkRandom& rand) {
    for (int i = 0; i < 3; ++i) {
        conic->fPts[i].set(rand.nextUScalar1() * 100, rand.nextUScalar1() * 100);
    }
    if (rand.nextUScalar1() > 0.5f) {
        conic->fW = rand.nextUScalar1();
    } else {
        conic->fW = 1 + rand.nextUScalar1() * 4;
    }
}

class ConicBench : public Benchmark {
public:
    ConicBench()  {
        SkRandom rand;
        for (int i = 0; i < CONICS; ++i) {
            rand_conic(&fConics[i], rand);
        }
    }

    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

protected:
    enum {
        CONICS = 100
    };
    SkConic fConics[CONICS];

private:
    using INHERITED = Benchmark;
};

class ConicBench_ComputeError : public ConicBench {
public:
    ConicBench_ComputeError()  {}

protected:
    const char* onGetName() override {
        return "conic-compute-error";
    }

    void onDraw(int loops, SkCanvas*) override {
        SkVector err;
        for (int i = 0; i < loops; ++i) {
            for (int j = 0; j < CONICS; ++j) {
                fConics[j].computeAsQuadError(&err);
            }
        }
    }

private:
    using INHERITED = ConicBench;
};

class ConicBench_asQuadTol : public ConicBench {
public:
    ConicBench_asQuadTol()  {}

protected:
    const char* onGetName() override {
        return "conic-asQuadTol";
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; ++i) {
            for (int j = 0; j < CONICS; ++j) {
                fConics[j].asQuadTol(SK_ScalarHalf);
            }
        }
    }

private:
    using INHERITED = ConicBench;
};

class ConicBench_quadPow2 : public ConicBench {
public:
    ConicBench_quadPow2()  {}

protected:
    const char* onGetName() override {
        return "conic-quadPow2";
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; ++i) {
            for (int j = 0; j < CONICS; ++j) {
                fConics[j].computeQuadPOW2(SK_ScalarHalf);
            }
        }
    }

private:
    using INHERITED = ConicBench;
};

///////////////////////////////////////////////////////////////////////////////

class TightBoundsBench : public Benchmark {
    SkPath      fPath;
    SkString    fName;
    SkRect      (*fProc)(const SkPath&);

public:
    TightBoundsBench(SkRect (*proc)(const SkPath&), const char suffix[]) : fProc(proc) {
        fName.printf("tight_bounds_%s", suffix);

        SkPathBuilder builder;
        const int N = 100;
        SkRandom rand;
        for (int i = 0; i < N; ++i) {
            builder.moveTo(rand.nextF()*100, rand.nextF()*100);
            builder.lineTo(rand.nextF()*100, rand.nextF()*100);
            builder.quadTo(rand.nextF()*100, rand.nextF()*100, rand.nextF()*100, rand.nextF()*100);
            builder.conicTo(rand.nextF()*100, rand.nextF()*100, rand.nextF()*100, rand.nextF()*100,
                            rand.nextF()*10);
            builder.cubicTo(rand.nextF()*100, rand.nextF()*100, rand.nextF()*100, rand.nextF()*100,
                            rand.nextF()*100, rand.nextF()*100);
        }
        fPath = builder.detach();
    }

protected:
    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops*100; ++i) {
            fProc(fPath);
        }
    }

private:
    using INHERITED = Benchmark;
};


const SkRect ConservativelyContainsBench::kBounds = SkRect::MakeWH(SkIntToScalar(100), SkIntToScalar(100));
const SkSize ConservativelyContainsBench::kQueryMin = {SkIntToScalar(1), SkIntToScalar(1)};
const SkSize ConservativelyContainsBench::kQueryMax = {SkIntToScalar(40), SkIntToScalar(40)};
const SkRect ConservativelyContainsBench::kBaseRect = SkRect::MakeXYWH(SkIntToScalar(25), SkIntToScalar(25), SkIntToScalar(50), SkIntToScalar(50));
const SkScalar ConservativelyContainsBench::kRRRadii[2] = {SkIntToScalar(5), SkIntToScalar(10)};

DEF_BENCH( return new TrianglePathBench(FLAGS00); )
DEF_BENCH( return new TrianglePathBench(FLAGS01); )
DEF_BENCH( return new TrianglePathBench(FLAGS10); )
DEF_BENCH( return new TrianglePathBench(FLAGS11); )

DEF_BENCH( return new RectPathBench(FLAGS00); )
DEF_BENCH( return new RectPathBench(FLAGS01); )
DEF_BENCH( return new RectPathBench(FLAGS10); )
DEF_BENCH( return new RectPathBench(FLAGS11); )

DEF_BENCH( return new RotatedRectBench(FLAGS00, false, 45))
DEF_BENCH( return new RotatedRectBench(FLAGS10, false, 45))
DEF_BENCH( return new RotatedRectBench(FLAGS00, true, 45))
DEF_BENCH( return new RotatedRectBench(FLAGS10, true, 45))

DEF_BENCH( return new OvalPathBench(FLAGS00); )
DEF_BENCH( return new OvalPathBench(FLAGS01); )
DEF_BENCH( return new OvalPathBench(FLAGS10); )
DEF_BENCH( return new OvalPathBench(FLAGS11); )

DEF_BENCH( return new CirclePathBench(FLAGS00); )
DEF_BENCH( return new CirclePathBench(FLAGS01); )
DEF_BENCH( return new CirclePathBench(FLAGS10); )
DEF_BENCH( return new CirclePathBench(FLAGS11); )

DEF_BENCH( return new NonAACirclePathBench(FLAGS00); )
DEF_BENCH( return new NonAACirclePathBench(FLAGS10); )

DEF_BENCH( return new AAAConcavePathBench(FLAGS00); )
DEF_BENCH( return new AAAConcavePathBench(FLAGS10); )
DEF_BENCH( return new AAAConvexPathBench(FLAGS00); )
DEF_BENCH( return new AAAConvexPathBench(FLAGS10); )

DEF_BENCH( return new SawToothPathBench(FLAGS00); )
DEF_BENCH( return new SawToothPathBench(FLAGS01); )

DEF_BENCH( return new LongCurvedPathBench(FLAGS00); )
DEF_BENCH( return new LongCurvedPathBench(FLAGS01); )
DEF_BENCH( return new LongLinePathBench(FLAGS00); )
DEF_BENCH( return new LongLinePathBench(FLAGS01); )

DEF_BENCH( return new PathCreateBench(); )
DEF_BENCH( return new PathCopyBench(); )
DEF_BENCH( return new PathEqualityBench(); )

DEF_BENCH( return new PathTransformBench(BenchPathType::kData,    true); )
DEF_BENCH( return new PathTransformBench(BenchPathType::kBuilder, true); )
DEF_BENCH( return new PathTransformBench(BenchPathType::kPath,    true); )

DEF_BENCH( return new PathTransformBench(BenchPathType::kData,    false); )
DEF_BENCH( return new PathTransformBench(BenchPathType::kBuilder, false); )
DEF_BENCH( return new PathTransformBench(BenchPathType::kPath,    false); )

#define MAKEFROM(name)  PathMakeFromBench(#name, name);

DEF_BENCH( return new MAKEFROM(pdata_from_rrect) )
DEF_BENCH( return new MAKEFROM(builder_from_rrect) )
DEF_BENCH( return new MAKEFROM(path_from_rrect) )

DEF_BENCH( return new MAKEFROM(pdata_from_oval) )
DEF_BENCH( return new MAKEFROM(builder_from_oval) )
DEF_BENCH( return new MAKEFROM(path_from_oval) )

DEF_BENCH( return new MAKEFROM(pdata_from_rect) )
DEF_BENCH( return new MAKEFROM(builder_from_rect) )
DEF_BENCH( return new MAKEFROM(path_from_rect) )

#undef MAKEFROM

#ifndef SK_HIDE_PATH_EDIT_METHODS
DEF_BENCH( return new SkBench_AddPathTest(SkBench_AddPathTest::kAdd_AddType); )
DEF_BENCH( return new SkBench_AddPathTest(SkBench_AddPathTest::kAddTrans_AddType); )
DEF_BENCH( return new SkBench_AddPathTest(SkBench_AddPathTest::kAddMatrix_AddType); )
DEF_BENCH( return new SkBench_AddPathTest(SkBench_AddPathTest::kReverseAdd_AddType); )
#endif

DEF_BENCH( return new CirclesBench(FLAGS00); )
DEF_BENCH( return new CirclesBench(FLAGS01); )
DEF_BENCH( return new ArbRoundRectBench(false); )
DEF_BENCH( return new ArbRoundRectBench(true); )
DEF_BENCH( return new ConservativelyContainsBench(ConservativelyContainsBench::kRect_Type); )
DEF_BENCH( return new ConservativelyContainsBench(ConservativelyContainsBench::kRoundRect_Type); )
DEF_BENCH( return new ConservativelyContainsBench(ConservativelyContainsBench::kOval_Type); )

#include "include/pathops/SkPathOps.h"
#include "src/core/SkPathPriv.h"

DEF_BENCH( return new TightBoundsBench([](const SkPath& path){ return path.computeTightBounds();},
                                       "priv"); )

// These seem to be optimized away, which is troublesome for timing.
/*
DEF_BENCH( return new ConicBench_Chop5() )
DEF_BENCH( return new ConicBench_ComputeError() )
DEF_BENCH( return new ConicBench_asQuadTol() )
DEF_BENCH( return new ConicBench_quadPow2() )
*/

class CommonConvexBench : public Benchmark {
protected:
    SkString    fName;
    SkPath      fPath;
    const bool  fAA;

public:
    CommonConvexBench(int w, int h, bool forceConcave, bool aa) : fAA(aa) {
        fName.printf("convex_path_%d_%d_%d_%d", w, h, forceConcave, aa);

        SkRect r = SkRect::MakeXYWH(10, 10, w*1.0f, h*1.0f);
        fPath = SkPath::RRect(SkRRect::MakeRectXY(r, w/8.0f, h/8.0f));

        if (forceConcave) {
            SkPathPriv::SetConvexity(fPath, SkPathConvexity::kConcave);
            SkASSERT(!fPath.isConvex());
        } else {
            SkASSERT(fPath.isConvex());
        }
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(fAA);

        for (int i = 0; i < loops; ++i) {
            for (int inner = 0; inner < 100; ++inner) {
                canvas->drawPath(fPath, paint);
            }
        }
    }

private:
    using INHERITED = Benchmark;
};

DEF_BENCH( return new CommonConvexBench( 16, 16, false, false); )
DEF_BENCH( return new CommonConvexBench( 16, 16, true,  false); )
DEF_BENCH( return new CommonConvexBench( 16, 16, false, true); )
DEF_BENCH( return new CommonConvexBench( 16, 16, true,  true); )

DEF_BENCH( return new CommonConvexBench(200, 16, false, false); )
DEF_BENCH( return new CommonConvexBench(200, 16, true,  false); )
DEF_BENCH( return new CommonConvexBench(200, 16, false, true); )
DEF_BENCH( return new CommonConvexBench(200, 16, true,  true); )

class PathBuildBench : public Benchmark {
public:
    using Builder = SkPath(const SkRect&);

    Builder* fBuilder;
    SkString fName;

    PathBuildBench(const char name[], Builder* builder) : fBuilder(builder) {
        fName.printf("path_buider_%s", name);
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        const SkRect r = {1, 2, 3, 4};
        for (int i = 0; i < loops; ++i) {
            SkPath path = fBuilder(r);
            std::ignore = path.getBounds();
        }
    }

    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

private:
    using INHERITED = Benchmark;
};

DEF_BENCH( return new PathBuildBench("addRect", [](const SkRect& r) {
    return SkPath::Rect(r);
}))
DEF_BENCH( return new PathBuildBench("addOval", [](const SkRect& r) {
    return SkPath::Oval(r);
}))
DEF_BENCH( return new PathBuildBench("addRRect", [](const SkRect& r) {
    return SkPath::RRect(SkRRect::MakeRectXY(r, 0.1f, 0.1f));
}))

class PathIsRectBench final : public Benchmark {
public:
    PathIsRectBench(const char* name, SkPath p)
        : fName(SkStringPrintf("path_isrect_%s", name))
        , fPath(std::move(p))
    {
        SkASSERT_RELEASE(fName.endsWith("norect") == !fPath.isRect(nullptr));
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

    void onDraw(int loops, SkCanvas*) override {
        SkRect rect;
        bool closed;
        SkPathDirection dir;
        for (int i = 0; i < loops; ++i) {
            std::ignore = fPath.isRect(&rect, &closed, &dir);
        }
    }

private:
    const SkString fName;
    const SkPath   fPath;
};

DEF_BENCH( return new PathIsRectBench("trivial", SkPath::Rect({10, 10, 100, 50})); )
DEF_BENCH( return new PathIsRectBench("complex", SkPathBuilder()
                                                    .moveTo( 10, 10)
                                                    .lineTo( 50, 10)
                                                    .lineTo(100, 10)
                                                    .lineTo(100, 25)
                                                    .lineTo(100, 50)
                                                    .lineTo( 50, 50)
                                                    .lineTo( 10, 50)
                                                    .lineTo( 10, 25)
                                                    .lineTo( 10, 10)
                                                    .close()
                                                    .detach()); )
DEF_BENCH( return new PathIsRectBench("empty_norect", SkPath()); )
DEF_BENCH( return new PathIsRectBench("complex_norect", SkPathBuilder()
                                                    .moveTo( 10, 10)
                                                    .lineTo( 50, 10)
                                                    .lineTo(100, 10)
                                                    .lineTo(100, 25)
                                                    .lineTo(100, 50)
                                                    .lineTo( 50, 50)
                                                    .lineTo( 10, 50)
                                                    .lineTo( 10, 25)
                                                    .lineTo( 10, 10)
                                                    .conicTo(10, 20, 20, 20, .7f)
                                                    .close()
                                                    .detach()); )
