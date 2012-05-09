/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBenchmark.h"
#include "SkAAClip.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkString.h"
#include "SkCanvas.h"
#include "SkRandom.h"

////////////////////////////////////////////////////////////////////////////////
// This bench tests out AA/BW clipping via canvas' clipPath and clipRect calls
class AAClipBench : public SkBenchmark {
    SkString fName;
    SkPath   fClipPath;
    SkRect   fClipRect;
    SkRect   fDrawRect;
    bool     fDoPath;
    bool     fDoAA;

    enum {
        N = SkBENCHLOOP(200),
    };

public:
    AAClipBench(void* param, bool doPath, bool doAA) 
        : INHERITED(param)
        , fDoPath(doPath)
        , fDoAA(doAA) {

        fName.printf("aaclip_%s_%s",
                     doPath ? "path" : "rect",
                     doAA ? "AA" : "BW");

        fClipRect.set(SkFloatToScalar(10.5), SkFloatToScalar(10.5), 
                      SkFloatToScalar(50.5), SkFloatToScalar(50.5));
        fClipPath.addRoundRect(fClipRect, SkIntToScalar(10), SkIntToScalar(10));
        fDrawRect.set(SkIntToScalar(0), SkIntToScalar(0),
                      SkIntToScalar(100), SkIntToScalar(100));

        SkASSERT(fClipPath.isConvex());
    }

protected:
    virtual const char* onGetName() { return fName.c_str(); }
    virtual void onDraw(SkCanvas* canvas) {

        SkPaint paint;
        this->setupPaint(&paint);

        for (int i = 0; i < N; ++i) {
            // jostle the clip regions each time to prevent caching
            fClipRect.offset((i % 2) == 0 ? SkIntToScalar(10) : SkIntToScalar(-10), 0);
            fClipPath.reset();
            fClipPath.addRoundRect(fClipRect, 
                                   SkIntToScalar(5), SkIntToScalar(5));
            SkASSERT(fClipPath.isConvex());

            canvas->save();
#if 1
            if (fDoPath) {
                canvas->clipPath(fClipPath, SkRegion::kReplace_Op, fDoAA);
            } else {
                canvas->clipRect(fClipRect, SkRegion::kReplace_Op, fDoAA);
            }

            canvas->drawRect(fDrawRect, paint);
#else
            // this path tests out directly draw the clip primitive
            // use it to comparing just drawing the clip vs. drawing using
            // the clip
            if (fDoPath) {
                canvas->drawPath(fClipPath, paint);
            } else {
                canvas->drawRect(fClipRect, paint);
            }
#endif
            canvas->restore();
        }
    }
private:
    typedef SkBenchmark INHERITED;
};

////////////////////////////////////////////////////////////////////////////////
// This bench tests out nested clip stacks. It is intended to simulate
// how WebKit nests clips.
class NestedAAClipBench : public SkBenchmark {
    SkString fName;
    bool     fDoAA;
    SkRect   fDrawRect;
    SkRandom fRandom;

    static const int kNumDraws = SkBENCHLOOP(200);
    static const int kNestingDepth = 4;
    static const int kImageSize = 400;

    SkPoint fSizes[kNestingDepth+1];

public:
    NestedAAClipBench(void* param, bool doAA) 
        : INHERITED(param)
        , fDoAA(doAA) {

        fName.printf("nested_aaclip_%s", doAA ? "AA" : "BW");

        fDrawRect = SkRect::MakeLTRB(0, 0, 
                                     SkIntToScalar(kImageSize), 
                                     SkIntToScalar(kImageSize));

        fSizes[0].set(SkIntToScalar(kImageSize), SkIntToScalar(kImageSize));

        for (int i = 1; i < kNestingDepth+1; ++i) {
            fSizes[i].set(fSizes[i-1].fX/2, fSizes[i-1].fY/2);
        }
    }

protected:
    virtual const char* onGetName() { return fName.c_str(); }


    void recurse(SkCanvas* canvas, 
                 int depth,
                 const SkPoint& offset) {

            canvas->save();

            SkRect temp = SkRect::MakeLTRB(0, 0, 
                                           fSizes[depth].fX, fSizes[depth].fY);
            temp.offset(offset);

            SkPath path;
            path.addRoundRect(temp, SkIntToScalar(3), SkIntToScalar(3));
            SkASSERT(path.isConvex());

            canvas->clipPath(path, 
                             0 == depth ? SkRegion::kReplace_Op : 
                                          SkRegion::kIntersect_Op,
                             fDoAA);

            if (kNestingDepth == depth) {
                // we only draw the draw rect at the lowest nesting level
                SkPaint paint;
                paint.setColor(0xff000000 | fRandom.nextU());
                canvas->drawRect(fDrawRect, paint);
            } else {
                SkPoint childOffset = offset;
                this->recurse(canvas, depth+1, childOffset);

                childOffset += fSizes[depth+1];
                this->recurse(canvas, depth+1, childOffset);

                childOffset.fX = offset.fX + fSizes[depth+1].fX;
                childOffset.fY = offset.fY;
                this->recurse(canvas, depth+1, childOffset);

                childOffset.fX = offset.fX;
                childOffset.fY = offset.fY + fSizes[depth+1].fY;
                this->recurse(canvas, depth+1, childOffset);
            }

            canvas->restore();
    }

    virtual void onDraw(SkCanvas* canvas) {

        for (int i = 0; i < kNumDraws; ++i) {
            SkPoint offset = SkPoint::Make(0, 0);
            this->recurse(canvas, 0, offset);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

////////////////////////////////////////////////////////////////////////////////
class AAClipBuilderBench : public SkBenchmark {
    SkString fName;
    SkPath   fPath;
    SkRect   fRect;
    SkRegion fRegion;
    bool     fDoPath;
    bool     fDoAA;

    enum {
        N = SkBENCHLOOP(200),
    };

public:
    AAClipBuilderBench(void* param, bool doPath, bool doAA) : INHERITED(param) {
        fDoPath = doPath;
        fDoAA = doAA;

        fName.printf("aaclip_build_%s_%s", doPath ? "path" : "rect",
                     doAA ? "AA" : "BW");

        fRegion.setRect(0, 0, 640, 480);
        fRect.set(fRegion.getBounds());
        fRect.inset(SK_Scalar1/4, SK_Scalar1/4);
        fPath.addRoundRect(fRect, SkIntToScalar(20), SkIntToScalar(20));
    }

protected:
    virtual const char* onGetName() { return fName.c_str(); }
    virtual void onDraw(SkCanvas* canvas) {
        SkPaint paint;
        this->setupPaint(&paint);

        for (int i = 0; i < N; ++i) {
            SkAAClip clip;
            if (fDoPath) {
                clip.setPath(fPath, &fRegion, fDoAA);
            } else {
                clip.setRect(fRect, fDoAA);
            }
        }
    }
private:
    typedef SkBenchmark INHERITED;
};

////////////////////////////////////////////////////////////////////////////////
class AAClipRegionBench : public SkBenchmark {
public:
    AAClipRegionBench(void* param) : INHERITED(param) {
        SkPath path;
        // test conversion of a complex clip to a aaclip
        path.addCircle(0, 0, SkIntToScalar(200));
        path.addCircle(0, 0, SkIntToScalar(180));
        // evenodd means we've constructed basically a stroked circle
        path.setFillType(SkPath::kEvenOdd_FillType);

        SkIRect bounds;
        path.getBounds().roundOut(&bounds);
        fRegion.setPath(path, SkRegion(bounds));
    }

protected:
    virtual const char* onGetName() { return "aaclip_setregion"; }
    virtual void onDraw(SkCanvas* canvas) {
        for (int i = 0; i < N; ++i) {
            SkAAClip clip;
            clip.setRegion(fRegion);
        }
    }

private:
    enum {
        N = SkBENCHLOOP(400),
    };
    SkRegion fRegion;
    typedef SkBenchmark INHERITED;
};

////////////////////////////////////////////////////////////////////////////////

static SkBenchmark* Fact0(void* p) { return SkNEW_ARGS(AAClipBuilderBench, (p, false, false)); }
static SkBenchmark* Fact1(void* p) { return SkNEW_ARGS(AAClipBuilderBench, (p, false, true)); }
static SkBenchmark* Fact2(void* p) { return SkNEW_ARGS(AAClipBuilderBench, (p, true, false)); }
static SkBenchmark* Fact3(void* p) { return SkNEW_ARGS(AAClipBuilderBench, (p, true, true)); }

static BenchRegistry gReg0(Fact0);
static BenchRegistry gReg1(Fact1);
static BenchRegistry gReg2(Fact2);
static BenchRegistry gReg3(Fact3);

static SkBenchmark* Fact01(void* p) { return SkNEW_ARGS(AAClipRegionBench, (p)); }
static BenchRegistry gReg01(Fact01);

static SkBenchmark* Fact000(void* p) { return SkNEW_ARGS(AAClipBench, (p, false, false)); }
static SkBenchmark* Fact001(void* p) { return SkNEW_ARGS(AAClipBench, (p, false, true)); }
static SkBenchmark* Fact002(void* p) { return SkNEW_ARGS(AAClipBench, (p, true, false)); }
static SkBenchmark* Fact003(void* p) { return SkNEW_ARGS(AAClipBench, (p, true, true)); }

static BenchRegistry gReg000(Fact000);
static BenchRegistry gReg001(Fact001);
static BenchRegistry gReg002(Fact002);
static BenchRegistry gReg003(Fact003);

static SkBenchmark* Fact004(void* p) { return SkNEW_ARGS(NestedAAClipBench, (p, false)); }
static SkBenchmark* Fact005(void* p) { return SkNEW_ARGS(NestedAAClipBench, (p, true)); }

static BenchRegistry gReg004(Fact004);
static BenchRegistry gReg005(Fact005);
