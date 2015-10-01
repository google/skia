/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkAAClip.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkString.h"

////////////////////////////////////////////////////////////////////////////////
// This bench tests out AA/BW clipping via canvas' clipPath and clipRect calls
class AAClipBench : public Benchmark {
    SkString fName;
    SkPath   fClipPath;
    SkRect   fClipRect;
    SkRect   fDrawRect;
    bool     fDoPath;
    bool     fDoAA;

public:
    AAClipBench(bool doPath, bool doAA)
        : fDoPath(doPath)
        , fDoAA(doAA) {

        fName.printf("aaclip_%s_%s",
                     doPath ? "path" : "rect",
                     doAA ? "AA" : "BW");

        fClipRect.set(10.5f, 10.5f,
                      50.5f, 50.5f);
        fClipPath.addRoundRect(fClipRect, SkIntToScalar(10), SkIntToScalar(10));
        fDrawRect.set(SkIntToScalar(0), SkIntToScalar(0),
                      SkIntToScalar(100), SkIntToScalar(100));

        SkASSERT(fClipPath.isConvex());
    }

protected:
    virtual const char* onGetName() { return fName.c_str(); }
    virtual void onDraw(int loops, SkCanvas* canvas) {

        SkPaint paint;
        this->setupPaint(&paint);

        for (int i = 0; i < loops; ++i) {
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
    typedef Benchmark INHERITED;
};

////////////////////////////////////////////////////////////////////////////////
// This bench tests out nested clip stacks. It is intended to simulate
// how WebKit nests clips.
class NestedAAClipBench : public Benchmark {
    SkString fName;
    bool     fDoAA;
    SkRect   fDrawRect;
    SkRandom fRandom;

    static const int kNestingDepth = 3;
    static const int kImageSize = 400;

    SkPoint fSizes[kNestingDepth+1];

public:
    NestedAAClipBench(bool doAA) : fDoAA(doAA) {
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

    virtual void onDraw(int loops, SkCanvas* canvas) {

        for (int i = 0; i < loops; ++i) {
            SkPoint offset = SkPoint::Make(0, 0);
            this->recurse(canvas, 0, offset);
        }
    }

private:
    typedef Benchmark INHERITED;
};

////////////////////////////////////////////////////////////////////////////////
class AAClipBuilderBench : public Benchmark {
    SkString fName;
    SkPath   fPath;
    SkRect   fRect;
    SkRegion fRegion;
    bool     fDoPath;
    bool     fDoAA;

public:
    AAClipBuilderBench(bool doPath, bool doAA)  {
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
    virtual void onDraw(int loops, SkCanvas*) {
        SkPaint paint;
        this->setupPaint(&paint);

        for (int i = 0; i < loops; ++i) {
            SkAAClip clip;
            if (fDoPath) {
                clip.setPath(fPath, &fRegion, fDoAA);
            } else {
                clip.setRect(fRect, fDoAA);
            }
        }
    }
private:
    typedef Benchmark INHERITED;
};

////////////////////////////////////////////////////////////////////////////////
class AAClipRegionBench : public Benchmark {
public:
    AAClipRegionBench()  {
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
    virtual void onDraw(int loops, SkCanvas*) {
        for (int i = 0; i < loops; ++i) {
            SkAAClip clip;
            clip.setRegion(fRegion);
        }
    }

private:
    SkRegion fRegion;
    typedef Benchmark INHERITED;
};

////////////////////////////////////////////////////////////////////////////////

DEF_BENCH(return new AAClipBuilderBench(false, false);)
DEF_BENCH(return new AAClipBuilderBench(false, true);)
DEF_BENCH(return new AAClipBuilderBench(true, false);)
DEF_BENCH(return new AAClipBuilderBench(true, true);)
DEF_BENCH(return new AAClipRegionBench();)
DEF_BENCH(return new AAClipBench(false, false);)
DEF_BENCH(return new AAClipBench(false, true);)
DEF_BENCH(return new AAClipBench(true, false);)
DEF_BENCH(return new AAClipBench(true, true);)
DEF_BENCH(return new NestedAAClipBench(false);)
DEF_BENCH(return new NestedAAClipBench(true);)
