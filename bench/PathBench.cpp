
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkShader.h"
#include "SkString.h"
#include "SkTArray.h"


enum Flags {
    kStroke_Flag = 1 << 0,
    kBig_Flag    = 1 << 1
};

#define FLAGS00  Flags(0)
#define FLAGS01  Flags(kStroke_Flag)
#define FLAGS10  Flags(kBig_Flag)
#define FLAGS11  Flags(kStroke_Flag | kBig_Flag)

class PathBench : public SkBenchmark {
    SkPaint     fPaint;
    SkString    fName;
    Flags       fFlags;
    enum { N = SkBENCHLOOP(1000) };
public:
    PathBench(void* param, Flags flags) : INHERITED(param), fFlags(flags) {
        fPaint.setStyle(flags & kStroke_Flag ? SkPaint::kStroke_Style :
                        SkPaint::kFill_Style);
        fPaint.setStrokeWidth(SkIntToScalar(5));
        fPaint.setStrokeJoin(SkPaint::kBevel_Join);
    }

    virtual void appendName(SkString*) = 0;
    virtual void makePath(SkPath*) = 0;
    virtual int complexity() { return 0; }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        fName.printf("path_%s_%s_",
                     fFlags & kStroke_Flag ? "stroke" : "fill",
                     fFlags & kBig_Flag ? "big" : "small");
        this->appendName(&fName);
        return fName.c_str();
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkPaint paint(fPaint);
        this->setupPaint(&paint);

        SkPath path;
        this->makePath(&path);
        if (fFlags & kBig_Flag) {
            SkMatrix m;
            m.setScale(SkIntToScalar(10), SkIntToScalar(10));
            path.transform(m);
        }

        int count = N;
        if (fFlags & kBig_Flag) {
            count >>= 2;
        }
        count >>= (3 * complexity());

        for (int i = 0; i < count; i++) {
            canvas->drawPath(path, paint);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

class TrianglePathBench : public PathBench {
public:
    TrianglePathBench(void* param, Flags flags) : INHERITED(param, flags) {}

    virtual void appendName(SkString* name) SK_OVERRIDE {
        name->append("triangle");
    }
    virtual void makePath(SkPath* path) SK_OVERRIDE {
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
    RectPathBench(void* param, Flags flags) : INHERITED(param, flags) {}

    virtual void appendName(SkString* name) SK_OVERRIDE {
        name->append("rect");
    }
    virtual void makePath(SkPath* path) SK_OVERRIDE {
        SkRect r = { 10, 10, 20, 20 };
        path->addRect(r);
    }
private:
    typedef PathBench INHERITED;
};

class OvalPathBench : public PathBench {
public:
    OvalPathBench(void* param, Flags flags) : INHERITED(param, flags) {}

    virtual void appendName(SkString* name) SK_OVERRIDE {
        name->append("oval");
    }
    virtual void makePath(SkPath* path) SK_OVERRIDE {
        SkRect r = { 10, 10, 20, 20 };
        path->addOval(r);
    }
private:
    typedef PathBench INHERITED;
};

class CirclePathBench: public PathBench {
public:
    CirclePathBench(void* param, Flags flags) : INHERITED(param, flags) {}

    virtual void appendName(SkString* name) SK_OVERRIDE {
        name->append("circle");
    }
    virtual void makePath(SkPath* path) SK_OVERRIDE {
        path->addCircle(SkIntToScalar(20), SkIntToScalar(20),
                        SkIntToScalar(10));
    }
private:
    typedef PathBench INHERITED;
};

class SawToothPathBench : public PathBench {
public:
    SawToothPathBench(void* param, Flags flags) : INHERITED(param, flags) {}

    virtual void appendName(SkString* name) SK_OVERRIDE {
        name->append("sawtooth");
    }
    virtual void makePath(SkPath* path) {
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
    virtual int complexity() SK_OVERRIDE { return 1; }
private:
    typedef PathBench INHERITED;
};

class LongCurvedPathBench : public PathBench {
public:
    LongCurvedPathBench(void * param, Flags flags)
        : INHERITED(param, flags) {
    }

    virtual void appendName(SkString* name) SK_OVERRIDE {
        name->append("long_curved");
    }
    virtual void makePath(SkPath* path) SK_OVERRIDE {
        SkRandom rand (12);
        int i;
        for (i = 0; i < 100; i++) {
            path->quadTo(SkScalarMul(rand.nextUScalar1(), SkIntToScalar(640)),
                         SkScalarMul(rand.nextUScalar1(), SkIntToScalar(480)),
                         SkScalarMul(rand.nextUScalar1(), SkIntToScalar(640)),
                         SkScalarMul(rand.nextUScalar1(), SkIntToScalar(480)));
        }
        path->close();
    }
    virtual int complexity() SK_OVERRIDE { return 2; }
private:
    typedef PathBench INHERITED;
};

class LongLinePathBench : public PathBench {
public:
    LongLinePathBench(void * param, Flags flags)
        : INHERITED(param, flags) {
    }

    virtual void appendName(SkString* name) SK_OVERRIDE {
        name->append("long_line");
    }
    virtual void makePath(SkPath* path) SK_OVERRIDE {
        SkRandom rand;
        path->moveTo(rand.nextUScalar1() * 640, rand.nextUScalar1() * 480);
        for (size_t i = 1; i < 100; i++) {
            path->lineTo(rand.nextUScalar1() * 640, rand.nextUScalar1() * 480);
        }
    }
    virtual int complexity() SK_OVERRIDE { return 2; }
private:
    typedef PathBench INHERITED;
};

class RandomPathBench : public SkBenchmark {
public:
    RandomPathBench(void* param) : INHERITED(param) {
        fIsRendering = false;
    }

protected:
    void createData(int minVerbs,
                    int maxVerbs,
                    bool allowMoves = true,
                    SkRect* bounds = NULL) {
        SkRect tempBounds;
        if (NULL == bounds) {
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
    typedef SkBenchmark INHERITED;
};

class PathCreateBench : public RandomPathBench {
public:
    PathCreateBench(void* param) : INHERITED(param) {
    }

protected:
    enum { N = SkBENCHLOOP(5000) };

    virtual const char* onGetName() SK_OVERRIDE {
        return "path_create";
    }

    virtual void onPreDraw() SK_OVERRIDE {
        this->createData(10, 100);
        fPaths.reset(kPathCnt);
    }

    virtual void onDraw(SkCanvas*) SK_OVERRIDE {
        for (int i = 0; i < N; ++i) {
            this->makePath(&fPaths[i & (kPathCnt - 1)]);
        }
        this->restartMakingPaths();
    }

    virtual void onPostDraw() SK_OVERRIDE {
        this->finishedMakingPaths();
        fPaths.reset(0);
    }

private:
    enum {
        // must be a pow 2
        kPathCnt = 1 << 5,
    };
    SkAutoTArray<SkPath> fPaths;

    typedef RandomPathBench INHERITED;
};

class PathCopyBench : public RandomPathBench {
public:
    PathCopyBench(void* param) : INHERITED(param) {
    }

protected:
    enum { N = SkBENCHLOOP(30000) };

    virtual const char* onGetName() SK_OVERRIDE {
        return "path_copy";
    }
    virtual void onPreDraw() SK_OVERRIDE {
        this->createData(10, 100);
        fPaths.reset(kPathCnt);
        fCopies.reset(kPathCnt);
        for (int i = 0; i < kPathCnt; ++i) {
            this->makePath(&fPaths[i]);
        }
        this->finishedMakingPaths();
    }
    virtual void onDraw(SkCanvas*) SK_OVERRIDE {
        for (int i = 0; i < N; ++i) {
            int idx = i & (kPathCnt - 1);
            fCopies[idx] = fPaths[idx];
        }
    }
    virtual void onPostDraw() SK_OVERRIDE {
        fPaths.reset(0);
        fCopies.reset(0);
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
    PathTransformBench(bool inPlace, void* param)
        : INHERITED(param)
        , fInPlace(inPlace) {
    }

protected:
    enum { N = SkBENCHLOOP(30000) };

    virtual const char* onGetName() SK_OVERRIDE {
        return fInPlace ? "path_transform_in_place" : "path_transform_copy";
    }

    virtual void onPreDraw() SK_OVERRIDE {
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

    virtual void onDraw(SkCanvas*) SK_OVERRIDE {
        if (fInPlace) {
            for (int i = 0; i < N; ++i) {
                fPaths[i & (kPathCnt - 1)].transform(fMatrix);
            }
        } else {
            for (int i = 0; i < N; ++i) {
                int idx = i & (kPathCnt - 1);
                fPaths[idx].transform(fMatrix, &fTransformed[idx]);
            }
        }
    }

    virtual void onPostDraw() SK_OVERRIDE {
        fPaths.reset(0);
        fTransformed.reset(0);
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
    PathEqualityBench(void* param)
        : INHERITED(param) {
    }

protected:
    enum { N = SkBENCHLOOP(40000) };

    virtual const char* onGetName() SK_OVERRIDE {
        return "path_equality_50%";
    }

    virtual void onPreDraw() SK_OVERRIDE {
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

    virtual void onDraw(SkCanvas*) SK_OVERRIDE {
        for (int i = 0; i < N; ++i) {
            int idx = i & (kPathCnt - 1);
            fParity ^= (fPaths[idx] == fCopies[idx & ~0x1]);
        }
    }

    virtual void onPostDraw() SK_OVERRIDE {
        fPaths.reset(0);
        fCopies.reset(0);
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
        kPathTo_AddType,
        kReverseAdd_AddType,
        kReversePathTo_AddType,
    };

    SkBench_AddPathTest(AddType type, void* param)
        : INHERITED(param)
        , fType(type) {
        fMatrix.setRotate(60 * SK_Scalar1);
    }

protected:
    enum { N = SkBENCHLOOP(15000) };

    virtual const char* onGetName() SK_OVERRIDE {
        switch (fType) {
            case kAdd_AddType:
                return "path_add_path";
            case kAddTrans_AddType:
                return "path_add_path_trans";
            case kAddMatrix_AddType:
                return "path_add_path_matrix";
            case kPathTo_AddType:
                return "path_path_to";
            case kReverseAdd_AddType:
                return "path_reverse_add_path";
            case kReversePathTo_AddType:
                return "path_reverse_path_to";
            default:
                SkDEBUGFAIL("Bad add type");
                return "";
        }
    }

    virtual void onPreDraw() SK_OVERRIDE {
        // pathTo and reversePathTo assume a single contour path.
        bool allowMoves = kPathTo_AddType != fType &&
                          kReversePathTo_AddType != fType;
        this->createData(10, 100, allowMoves);
        fPaths0.reset(kPathCnt);
        fPaths1.reset(kPathCnt);
        for (int i = 0; i < kPathCnt; ++i) {
            this->makePath(&fPaths0[i]);
            this->makePath(&fPaths1[i]);
        }
        this->finishedMakingPaths();
    }

    virtual void onDraw(SkCanvas*) SK_OVERRIDE {
        switch (fType) {
            case kAdd_AddType:
                for (int i = 0; i < N; ++i) {
                    int idx = i & (kPathCnt - 1);
                    SkPath result = fPaths0[idx];
                    result.addPath(fPaths1[idx]);
                }
                break;
            case kAddTrans_AddType:
                for (int i = 0; i < N; ++i) {
                    int idx = i & (kPathCnt - 1);
                    SkPath result = fPaths0[idx];
                    result.addPath(fPaths1[idx], 2 * SK_Scalar1, 5 * SK_Scalar1);
                }
                break;
            case kAddMatrix_AddType:
                for (int i = 0; i < N; ++i) {
                    int idx = i & (kPathCnt - 1);
                    SkPath result = fPaths0[idx];
                    result.addPath(fPaths1[idx], fMatrix);
                }
                break;
            case kPathTo_AddType:
                for (int i = 0; i < N; ++i) {
                    int idx = i & (kPathCnt - 1);
                    SkPath result = fPaths0[idx];
                    result.pathTo(fPaths1[idx]);
                }
                break;
            case kReverseAdd_AddType:
                for (int i = 0; i < N; ++i) {
                    int idx = i & (kPathCnt - 1);
                    SkPath result = fPaths0[idx];
                    result.reverseAddPath(fPaths1[idx]);
                }
                break;
            case kReversePathTo_AddType:
                for (int i = 0; i < N; ++i) {
                    int idx = i & (kPathCnt - 1);
                    SkPath result = fPaths0[idx];
                    result.reversePathTo(fPaths1[idx]);
                }
                break;
        }
    }

    virtual void onPostDraw() SK_OVERRIDE {
        fPaths0.reset(0);
        fPaths1.reset(0);
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


class CirclesBench : public SkBenchmark {
protected:
    SkString            fName;

    enum {
        N = SkBENCHLOOP(100)
    };
public:
    CirclesBench(void* param) : INHERITED(param) {
        fName.printf("circles");
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkPaint paint;

        paint.setColor(SK_ColorBLACK);
        paint.setAntiAlias(true);

        SkRandom rand;

        SkRect r;

        for (int i = 0; i < 5000; ++i) {
            SkScalar radius = rand.nextUScalar1() * 3;
            r.fLeft = rand.nextUScalar1() * 300;
            r.fTop =  rand.nextUScalar1() * 300;
            r.fRight =  r.fLeft + 2 * radius;
            r.fBottom = r.fTop + 2 * radius;

            SkPath temp;

            // mimic how Chrome does circles
            temp.arcTo(r, 0, 0, false);
            temp.addOval(r, SkPath::kCCW_Direction);
            temp.arcTo(r, 360, 0, true);
            temp.close();

            canvas->drawPath(temp, paint);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

static SkBenchmark* FactT00(void* p) { return new TrianglePathBench(p, FLAGS00); }
static SkBenchmark* FactT01(void* p) { return new TrianglePathBench(p, FLAGS01); }
static SkBenchmark* FactT10(void* p) { return new TrianglePathBench(p, FLAGS10); }
static SkBenchmark* FactT11(void* p) { return new TrianglePathBench(p, FLAGS11); }

static SkBenchmark* FactR00(void* p) { return new RectPathBench(p, FLAGS00); }
static SkBenchmark* FactR01(void* p) { return new RectPathBench(p, FLAGS01); }
static SkBenchmark* FactR10(void* p) { return new RectPathBench(p, FLAGS10); }
static SkBenchmark* FactR11(void* p) { return new RectPathBench(p, FLAGS11); }

static SkBenchmark* FactO00(void* p) { return new OvalPathBench(p, FLAGS00); }
static SkBenchmark* FactO01(void* p) { return new OvalPathBench(p, FLAGS01); }
static SkBenchmark* FactO10(void* p) { return new OvalPathBench(p, FLAGS10); }
static SkBenchmark* FactO11(void* p) { return new OvalPathBench(p, FLAGS11); }

static SkBenchmark* FactC00(void* p) { return new CirclePathBench(p, FLAGS00); }
static SkBenchmark* FactC01(void* p) { return new CirclePathBench(p, FLAGS01); }
static SkBenchmark* FactC10(void* p) { return new CirclePathBench(p, FLAGS10); }
static SkBenchmark* FactC11(void* p) { return new CirclePathBench(p, FLAGS11); }

static SkBenchmark* FactS00(void* p) { return new SawToothPathBench(p, FLAGS00); }
static SkBenchmark* FactS01(void* p) { return new SawToothPathBench(p, FLAGS01); }

static SkBenchmark* FactLC00(void* p) {
    return new LongCurvedPathBench(p, FLAGS00);
}
static SkBenchmark* FactLC01(void* p) {
    return new LongCurvedPathBench(p, FLAGS01);
}

static SkBenchmark* FactLL00(void* p) {
    return new LongLinePathBench(p, FLAGS00);
}

static SkBenchmark* FactLL01(void* p) {
    return new LongLinePathBench(p, FLAGS01);
}

static BenchRegistry gRegT00(FactT00);
static BenchRegistry gRegT01(FactT01);
static BenchRegistry gRegT10(FactT10);
static BenchRegistry gRegT11(FactT11);

static BenchRegistry gRegR00(FactR00);
static BenchRegistry gRegR01(FactR01);
static BenchRegistry gRegR10(FactR10);
static BenchRegistry gRegR11(FactR11);

static BenchRegistry gRegO00(FactO00);
static BenchRegistry gRegO01(FactO01);
static BenchRegistry gRegO10(FactO10);
static BenchRegistry gRegO11(FactO11);

static BenchRegistry gRegC00(FactC00);
static BenchRegistry gRegC01(FactC01);
static BenchRegistry gRegC10(FactC10);
static BenchRegistry gRegC11(FactC11);

static BenchRegistry gRegS00(FactS00);
static BenchRegistry gRegS01(FactS01);

static BenchRegistry gRegLC00(FactLC00);
static BenchRegistry gRegLC01(FactLC01);

static BenchRegistry gRegLL00(FactLL00);
static BenchRegistry gRegLL01(FactLL01);

static SkBenchmark* FactCreate(void* p) { return new PathCreateBench(p); }
static BenchRegistry gRegCreate(FactCreate);

static SkBenchmark* FactCopy(void* p) { return new PathCopyBench(p); }
static BenchRegistry gRegCopy(FactCopy);

static SkBenchmark* FactPathTransformInPlace(void* p) { return new PathTransformBench(true, p); }
static BenchRegistry gRegPathTransformInPlace(FactPathTransformInPlace);

static SkBenchmark* FactPathTransformCopy(void* p) { return new PathTransformBench(false, p); }
static BenchRegistry gRegPathTransformCopy(FactPathTransformCopy);

static SkBenchmark* FactEquality(void* p) { return new PathEqualityBench(p); }
static BenchRegistry gRegEquality(FactEquality);

static SkBenchmark* FactAdd(void* p) { return new SkBench_AddPathTest(SkBench_AddPathTest::kAdd_AddType, p); }
static SkBenchmark* FactAddTrans(void* p) { return new SkBench_AddPathTest(SkBench_AddPathTest::kAddTrans_AddType, p); }
static SkBenchmark* FactAddMatrix(void* p) { return new SkBench_AddPathTest(SkBench_AddPathTest::kAddMatrix_AddType, p); }
static SkBenchmark* FactPathTo(void* p) { return new SkBench_AddPathTest(SkBench_AddPathTest::kPathTo_AddType, p); }
static SkBenchmark* FactReverseAdd(void* p) { return new SkBench_AddPathTest(SkBench_AddPathTest::kReverseAdd_AddType, p); }
static SkBenchmark* FactReverseTo(void* p) { return new SkBench_AddPathTest(SkBench_AddPathTest::kReversePathTo_AddType, p); }

static BenchRegistry gRegAdd(FactAdd);
static BenchRegistry gRegAddTrans(FactAddTrans);
static BenchRegistry gRegAddMatrix(FactAddMatrix);
static BenchRegistry gRegPathTo(FactPathTo);
static BenchRegistry gRegReverseAdd(FactReverseAdd);
static BenchRegistry gRegReverseTo(FactReverseTo);

static SkBenchmark* CirclesTest(void* p) { return new CirclesBench(p); }
static BenchRegistry gRegCirclesTest(CirclesTest);

