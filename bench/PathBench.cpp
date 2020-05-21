/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/private/SkTArray.h"
#include "include/utils/SkRandom.h"

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
    virtual void makePath(SkPath*) = 0;
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

        SkPath path;
        this->makePath(&path);
        if (fFlags & kBig_Flag) {
            const SkMatrix m = SkMatrix::MakeScale(SkIntToScalar(10), SkIntToScalar(10));
            path.transform(m);
        }

        for (int i = 0; i < loops; i++) {
            canvas->drawPath(path, paint);
        }
    }

private:
    typedef Benchmark INHERITED;
};

class TrianglePathBench : public PathBench {
public:
    TrianglePathBench(Flags flags) : INHERITED(flags) {}

    void appendName(SkString* name) override {
        name->append("triangle");
    }
    void makePath(SkPath* path) override {
        static const int gCoord[] = {
            10, 10, 15, 5, 20, 20
        };
        path->moveTo(SkIntToScalar(gCoord[0]), SkIntToScalar(gCoord[1]));
        path->lineTo(SkIntToScalar(gCoord[2]), SkIntToScalar(gCoord[3]));
        path->lineTo(SkIntToScalar(gCoord[4]), SkIntToScalar(gCoord[5]));
        path->close();
    }
private:
    typedef PathBench INHERITED;
};

class RectPathBench : public PathBench {
public:
    RectPathBench(Flags flags) : INHERITED(flags) {}

    void appendName(SkString* name) override {
        name->append("rect");
    }
    void makePath(SkPath* path) override {
        SkRect r = { 10, 10, 20, 20 };
        path->addRect(r);
    }
private:
    typedef PathBench INHERITED;
};

class RotatedRectBench : public PathBench {
public:
    RotatedRectBench(Flags flags, bool aa, int degrees) : INHERITED(flags) {
        fAA = aa;
        fDegrees = degrees;
    }

    void appendName(SkString* name) override {
        SkString suffix;
        suffix.printf("rotated_rect_%s_%d", fAA ? "aa" : "noaa", fDegrees);
        name->append(suffix);
    }

    void makePath(SkPath* path) override {
        SkRect r = { 10, 10, 20, 20 };
        path->addRect(r);
        SkMatrix rotateMatrix;
        rotateMatrix.setRotate((SkScalar)fDegrees);
        path->transform(rotateMatrix);
    }

    virtual void setupPaint(SkPaint* paint) override {
        PathBench::setupPaint(paint);
        paint->setAntiAlias(fAA);
    }
private:
    typedef PathBench INHERITED;
    int fDegrees;
    bool fAA;
};

class OvalPathBench : public PathBench {
public:
    OvalPathBench(Flags flags) : INHERITED(flags) {}

    void appendName(SkString* name) override {
        name->append("oval");
    }
    void makePath(SkPath* path) override {
        SkRect r = { 10, 10, 23, 20 };
        path->addOval(r);
    }
private:
    typedef PathBench INHERITED;
};

class CirclePathBench: public PathBench {
public:
    CirclePathBench(Flags flags) : INHERITED(flags) {}

    void appendName(SkString* name) override {
        name->append("circle");
    }
    void makePath(SkPath* path) override {
        path->addCircle(SkIntToScalar(20), SkIntToScalar(20),
                        SkIntToScalar(10));
    }
private:
    typedef PathBench INHERITED;
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
    typedef CirclePathBench INHERITED;
};

// Test max speedup of Analytic AA for concave paths
class AAAConcavePathBench : public PathBench {
public:
    AAAConcavePathBench(Flags flags) : INHERITED(flags) {}

    void appendName(SkString* name) override {
        name->append("concave_aaa");
    }

    void makePath(SkPath* path) override {
        path->moveTo(10, 10);
        path->lineTo(15, 10);
        path->lineTo(15, 5);
        path->lineTo(40, 40);
        path->close();
    }

private:
    typedef PathBench INHERITED;
};

// Test max speedup of Analytic AA for convex paths
class AAAConvexPathBench : public PathBench {
public:
    AAAConvexPathBench(Flags flags) : INHERITED(flags) {}

    void appendName(SkString* name) override {
        name->append("convex_aaa");
    }

    void makePath(SkPath* path) override {
        path->moveTo(10, 10);
        path->lineTo(15, 10);
        path->lineTo(40, 50);
        path->close();
    }

private:
    typedef PathBench INHERITED;
};

class SawToothPathBench : public PathBench {
public:
    SawToothPathBench(Flags flags) : INHERITED(flags) {}

    void appendName(SkString* name) override {
        name->append("sawtooth");
    }
    void makePath(SkPath* path) override {
        SkScalar x = SkIntToScalar(20);
        SkScalar y = SkIntToScalar(20);
        const SkScalar x0 = x;
        const SkScalar dx = SK_Scalar1 * 5;
        const SkScalar dy = SK_Scalar1 * 10;

        path->moveTo(x, y);
        for (int i = 0; i < 32; i++) {
            x += dx;
            path->lineTo(x, y - dy);
            x += dx;
            path->lineTo(x, y + dy);
        }
        path->lineTo(x, y + 2 * dy);
        path->lineTo(x0, y + 2 * dy);
        path->close();
    }
    int complexity() override { return 1; }
private:
    typedef PathBench INHERITED;
};

class LongCurvedPathBench : public PathBench {
public:
    LongCurvedPathBench(Flags flags) : INHERITED(flags) {}

    void appendName(SkString* name) override {
        name->append("long_curved");
    }
    void makePath(SkPath* path) override {
        SkRandom rand (12);
        int i;
        for (i = 0; i < 100; i++) {
            path->quadTo(rand.nextUScalar1() * 640, rand.nextUScalar1() * 480,
                         rand.nextUScalar1() * 640, rand.nextUScalar1() * 480);
        }
        path->close();
    }
    int complexity() override { return 2; }
private:
    typedef PathBench INHERITED;
};

class LongLinePathBench : public PathBench {
public:
    LongLinePathBench(Flags flags) : INHERITED(flags) {}

    void appendName(SkString* name) override {
        name->append("long_line");
    }
    void makePath(SkPath* path) override {
        SkRandom rand;
        path->moveTo(rand.nextUScalar1() * 640, rand.nextUScalar1() * 480);
        for (size_t i = 1; i < 100; i++) {
            path->lineTo(rand.nextUScalar1() * 640, rand.nextUScalar1() * 480);
        }
    }
    int complexity() override { return 2; }
private:
    typedef PathBench INHERITED;
};

class RandomPathBench : public Benchmark {
public:
    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
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
                fVerbs[i] = static_cast<SkPath::Verb>(fRandom.nextULessThan(SkPath::kDone_Verb));
            } while (!allowMoves && SkPath::kMove_Verb == fVerbs[i]);
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

    void makePath(SkPath* path) {
        int vCount = fVerbCnts[(fCurrPath++) & (kNumVerbCnts - 1)];
        for (int v = 0; v < vCount; ++v) {
            int verb = fVerbs[(fCurrVerb++) & (kNumVerbs - 1)];
            switch (verb) {
                case SkPath::kMove_Verb:
                    path->moveTo(fPoints[(fCurrPoint++) & (kNumPoints - 1)]);
                    break;
                case SkPath::kLine_Verb:
                    path->lineTo(fPoints[(fCurrPoint++) & (kNumPoints - 1)]);
                    break;
                case SkPath::kQuad_Verb:
                    path->quadTo(fPoints[(fCurrPoint + 0) & (kNumPoints - 1)],
                                 fPoints[(fCurrPoint + 1) & (kNumPoints - 1)]);
                    fCurrPoint += 2;
                    break;
                case SkPath::kConic_Verb:
                    path->conicTo(fPoints[(fCurrPoint + 0) & (kNumPoints - 1)],
                                  fPoints[(fCurrPoint + 1) & (kNumPoints - 1)],
                                  SK_ScalarHalf);
                    fCurrPoint += 2;
                    break;
                case SkPath::kCubic_Verb:
                    path->cubicTo(fPoints[(fCurrPoint + 0) & (kNumPoints - 1)],
                                  fPoints[(fCurrPoint + 1) & (kNumPoints - 1)],
                                  fPoints[(fCurrPoint + 2) & (kNumPoints - 1)]);
                    fCurrPoint += 3;
                    break;
                case SkPath::kClose_Verb:
                    path->close();
                    break;
                default:
                    SkDEBUGFAIL("Unexpected path verb");
                    break;
            }
        }
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
    SkAutoTArray<int>           fVerbCnts;
    SkAutoTArray<SkPath::Verb>  fVerbs;
    SkAutoTArray<SkPoint>       fPoints;
    int                         fCurrPath;
    int                         fCurrVerb;
    int                         fCurrPoint;
    SkRandom                    fRandom;
    typedef Benchmark INHERITED;
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
            if (i % 1000 == 0) {
                fPath.reset();  // PathRef memory can grow without bound otherwise.
            }
            this->makePath(&fPath);
        }
        this->restartMakingPaths();
    }

private:
    SkPath fPath;

    typedef RandomPathBench INHERITED;
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
            this->makePath(&fPaths[i]);
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
    SkAutoTArray<SkPath> fPaths;
    SkAutoTArray<SkPath> fCopies;

    typedef RandomPathBench INHERITED;
};

class PathTransformBench : public RandomPathBench {
public:
    PathTransformBench(bool inPlace) : fInPlace(inPlace) {}

protected:
    const char* onGetName() override {
        return fInPlace ? "path_transform_in_place" : "path_transform_copy";
    }

    void onDelayedSetup() override {
        fMatrix.setScale(5 * SK_Scalar1, 6 * SK_Scalar1);
        this->createData(10, 100);
        fPaths.reset(kPathCnt);
        for (int i = 0; i < kPathCnt; ++i) {
            this->makePath(&fPaths[i]);
        }
        this->finishedMakingPaths();
        if (!fInPlace) {
            fTransformed.reset(kPathCnt);
        }
    }

    void onDraw(int loops, SkCanvas*) override {
        if (fInPlace) {
            for (int i = 0; i < loops; ++i) {
                fPaths[i & (kPathCnt - 1)].transform(fMatrix);
            }
        } else {
            for (int i = 0; i < loops; ++i) {
                int idx = i & (kPathCnt - 1);
                fPaths[idx].transform(fMatrix, &fTransformed[idx]);
            }
        }
    }

private:
    enum {
        // must be a pow 2
        kPathCnt = 1 << 5,
    };
    SkAutoTArray<SkPath> fPaths;
    SkAutoTArray<SkPath> fTransformed;

    SkMatrix fMatrix;
    bool fInPlace;
    typedef RandomPathBench INHERITED;
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
            this->makePath(&fPaths[i]);
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
    SkAutoTArray<SkPath> fPaths;
    SkAutoTArray<SkPath> fCopies;
    typedef RandomPathBench INHERITED;
};

class SkBench_AddPathTest : public RandomPathBench {
public:
    enum AddType {
        kAdd_AddType,
        kAddTrans_AddType,
        kAddMatrix_AddType,
        kReverseAdd_AddType,
        kReversePathTo_AddType,
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
            case kReversePathTo_AddType:
                return "path_reverse_path_to";
            default:
                SkDEBUGFAIL("Bad add type");
                return "";
        }
    }

    void onDelayedSetup() override {
        // reversePathTo assumes a single contour path.
        bool allowMoves = kReversePathTo_AddType != fType;
        this->createData(10, 100, allowMoves);
        fPaths0.reset(kPathCnt);
        fPaths1.reset(kPathCnt);
        for (int i = 0; i < kPathCnt; ++i) {
            this->makePath(&fPaths0[i]);
            this->makePath(&fPaths1[i]);
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
            case kReversePathTo_AddType:
                for (int i = 0; i < loops; ++i) {
                    int idx = i & (kPathCnt - 1);
                    SkPath result = fPaths0[idx];
                    result.reversePathTo(fPaths1[idx]);
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
    SkAutoTArray<SkPath> fPaths0;
    SkAutoTArray<SkPath> fPaths1;
    SkMatrix         fMatrix;
    typedef RandomPathBench INHERITED;
};


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

            SkPath temp;

            // mimic how Chrome does circles
            temp.arcTo(r, 0, 0, false);
            temp.addOval(r, SkPathDirection::kCCW);
            temp.arcTo(r, 360, 0, true);
            temp.close();

            canvas->drawPath(temp, paint);
        }
    }

private:
    typedef Benchmark INHERITED;
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

    static void add_corner_arc(SkPath* path, const SkRect& rect,
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

        path->arcTo(arcRect, SkIntToScalar(startAngle), SkIntToScalar(90), false);
    }

    static void make_arb_round_rect(SkPath* path, const SkRect& r,
                                    SkScalar xCorner, SkScalar yCorner) {
        // we are lazy here and use the same x & y for each corner
        add_corner_arc(path, r, xCorner, yCorner, 270);
        add_corner_arc(path, r, xCorner, yCorner, 0);
        add_corner_arc(path, r, xCorner, yCorner, 90);
        add_corner_arc(path, r, xCorner, yCorner, 180);
        path->close();

        SkASSERT(path->isConvex());
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
                make_arb_round_rect(&temp, r, 0, 0);

                SkASSERT(temp.isRect(nullptr));
            } else {
                make_arb_round_rect(&temp, r, r.width() / 10, r.height() / 15);
            }

            canvas->drawPath(temp, paint);
        }
    }

private:
    bool fZeroRad;      // should 0 radius rounds rects be tested?

    typedef Benchmark INHERITED;
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
                fPath.addRect(kBaseRect);
                break;
            case kRoundRect_Type:
                fName.append("round_rect");
                fPath.addRoundRect(kBaseRect, kRRRadii[0], kRRRadii[1]);
                break;
            case kOval_Type:
                fName.append("oval");
                fPath.addOval(kBaseRect);
                break;
        }
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
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
        fQueryRects.setCount(kQueryRectCnt);

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

    typedef Benchmark INHERITED;
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
        return backend == kNonRendering_Backend;
    }

private:
    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; ++i) {
            fRQ.chop(fDst);
        }
    }

    typedef Benchmark INHERITED;
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
        return backend == kNonRendering_Backend;
    }

protected:
    enum {
        CONICS = 100
    };
    SkConic fConics[CONICS];

private:
    typedef Benchmark INHERITED;
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
    typedef ConicBench INHERITED;
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
    typedef ConicBench INHERITED;
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
    typedef ConicBench INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class TightBoundsBench : public Benchmark {
    SkPath      fPath;
    SkString    fName;
    SkRect      (*fProc)(const SkPath&);

public:
    TightBoundsBench(SkRect (*proc)(const SkPath&), const char suffix[]) : fProc(proc) {
        fName.printf("tight_bounds_%s", suffix);

        const int N = 100;
        SkRandom rand;
        for (int i = 0; i < N; ++i) {
            fPath.moveTo(rand.nextF()*100, rand.nextF()*100);
            fPath.lineTo(rand.nextF()*100, rand.nextF()*100);
            fPath.quadTo(rand.nextF()*100, rand.nextF()*100, rand.nextF()*100, rand.nextF()*100);
            fPath.conicTo(rand.nextF()*100, rand.nextF()*100, rand.nextF()*100, rand.nextF()*100,
                          rand.nextF()*10);
            fPath.cubicTo(rand.nextF()*100, rand.nextF()*100, rand.nextF()*100, rand.nextF()*100,
                          rand.nextF()*100, rand.nextF()*100);
        }
    }

protected:
    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops*100; ++i) {
            fProc(fPath);
        }
    }

private:
    typedef Benchmark INHERITED;
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

DEF_BENCH( return new RotatedRectBench(FLAGS00, false, 45));
DEF_BENCH( return new RotatedRectBench(FLAGS10, false, 45));
DEF_BENCH( return new RotatedRectBench(FLAGS00, true, 45));
DEF_BENCH( return new RotatedRectBench(FLAGS10, true, 45));

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
DEF_BENCH( return new PathTransformBench(true); )
DEF_BENCH( return new PathTransformBench(false); )
DEF_BENCH( return new PathEqualityBench(); )

DEF_BENCH( return new SkBench_AddPathTest(SkBench_AddPathTest::kAdd_AddType); )
DEF_BENCH( return new SkBench_AddPathTest(SkBench_AddPathTest::kAddTrans_AddType); )
DEF_BENCH( return new SkBench_AddPathTest(SkBench_AddPathTest::kAddMatrix_AddType); )
DEF_BENCH( return new SkBench_AddPathTest(SkBench_AddPathTest::kReverseAdd_AddType); )
DEF_BENCH( return new SkBench_AddPathTest(SkBench_AddPathTest::kReversePathTo_AddType); )

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
DEF_BENCH( return new TightBoundsBench([](const SkPath& path) {
        SkRect bounds; TightBounds(path, &bounds); return bounds;
    }, "pathops"); )

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
        fPath.addRRect(SkRRect::MakeRectXY(r, w/8.0f, h/8.0f));

        if (forceConcave) {
            fPath.setConvexityType(SkPathConvexityType::kConcave);
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
    typedef Benchmark INHERITED;
};

DEF_BENCH( return new CommonConvexBench( 16, 16, false, false); )
DEF_BENCH( return new CommonConvexBench( 16, 16, true,  false); )
DEF_BENCH( return new CommonConvexBench( 16, 16, false, true); )
DEF_BENCH( return new CommonConvexBench( 16, 16, true,  true); )

DEF_BENCH( return new CommonConvexBench(200, 16, false, false); )
DEF_BENCH( return new CommonConvexBench(200, 16, true,  false); )
DEF_BENCH( return new CommonConvexBench(200, 16, false, true); )
DEF_BENCH( return new CommonConvexBench(200, 16, true,  true); )
