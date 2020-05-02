/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/core/SkString.h"
#include "include/pathops/SkPathOps.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkClipOpPriv.h"
#include "src/core/SkPathPriv.h"
#include "src/gpu/GrClipStack.h"


namespace skiagm {

//this test exercise SkCanvas::setDeviceClipRestriction behavior
class ComplexClip4GM : public GM {
public:
  ComplexClip4GM(bool aaclip)
    : fDoAAClip(aaclip) {
        this->setBGColor(0xFFDEDFDE);
    }

protected:


    SkString onShortName() {
        SkString str;
        str.printf("complexclip4_%s",
                   fDoAAClip ? "aa" : "bw");
        return str;
    }

    SkISize onISize() { return SkISize::Make(970, 780); }

    virtual void onDraw(SkCanvas* canvas) {
        SkPaint p;
        p.setAntiAlias(fDoAAClip);
        p.setColor(SK_ColorYELLOW);

        canvas->save();
            // draw a yellow rect through a rect clip
            canvas->save();
                canvas->androidFramework_setDeviceClipRestriction(SkIRect::MakeLTRB(100, 100, 300, 300));
                canvas->drawColor(SK_ColorGREEN);
                canvas->clipRect(SkRect::MakeLTRB(100, 200, 400, 500),
                                 kReplace_SkClipOp, fDoAAClip);
                canvas->drawRect(SkRect::MakeLTRB(100, 200, 400, 500), p);
            canvas->restore();

            // draw a yellow rect through a diamond clip
            canvas->save();
                canvas->androidFramework_setDeviceClipRestriction(SkIRect::MakeLTRB(500, 100, 800, 300));
                canvas->drawColor(SK_ColorGREEN);

                SkPath pathClip;
                pathClip.moveTo(SkIntToScalar(650),  SkIntToScalar(200));
                pathClip.lineTo(SkIntToScalar(900), SkIntToScalar(300));
                pathClip.lineTo(SkIntToScalar(650), SkIntToScalar(400));
                pathClip.lineTo(SkIntToScalar(650), SkIntToScalar(300));
                pathClip.close();
                canvas->clipPath(pathClip, kReplace_SkClipOp, fDoAAClip);
                canvas->drawRect(SkRect::MakeLTRB(500, 200, 900, 500), p);
            canvas->restore();

            // draw a yellow rect through a round rect clip
            canvas->save();
                canvas->androidFramework_setDeviceClipRestriction(SkIRect::MakeLTRB(500, 500, 800, 700));
                canvas->drawColor(SK_ColorGREEN);

                canvas->clipRRect(SkRRect::MakeOval(SkRect::MakeLTRB(500, 600, 900, 750)),
                                  kReplace_SkClipOp, fDoAAClip);
                canvas->drawRect(SkRect::MakeLTRB(500, 600, 900, 750), p);
            canvas->restore();

            // fill the clip with yellow color showing that androidFramework_setDeviceClipRestriction
            // intersects with the current clip
            canvas->save();
                canvas->clipRect(SkRect::MakeLTRB(100, 400, 300, 750),
                                 kIntersect_SkClipOp, fDoAAClip);
                canvas->drawColor(SK_ColorGREEN);
                canvas->androidFramework_setDeviceClipRestriction(SkIRect::MakeLTRB(150, 450, 250, 700));
                canvas->drawColor(SK_ColorYELLOW);
            canvas->restore();

        canvas->restore();
    }
private:

    bool fDoAAClip;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ComplexClip4GM(false);)
DEF_GM(return new ComplexClip4GM(true);)

//////////////////////////////////////////////////////////////////////////////

class ClipStackStrategyGM : public GM {
public:
    enum class Mode {
        // Just draw, no clipping
        kDraw,
        // Semi-reference: actually clip and then fill
        kClipAndFill,
        // Use path ops to generate perfect path representation before rasterizing once
        kPathOps,
        // Rasterize each clip op into a single mask
        kRasterize,
        // Use SW rasterization and then upload to GPU (only valid for GPU config)
        kSWRasterize,
        // Invoke regular clip operations, but don't execute a draw
        // (benching only covers the clip tracking, minus application and prep).
        kClipOnly,
        // Invoke new clip operations, but don't execute the draw
        kNewClipOnly,
        // Baseline that only generates the paths and that's it
        kPrepOnly,
    };

    ClipStackStrategyGM(Mode mode, int index = -1)
            : fMode(mode)
            , fCountIndex(index)
            , fName("clip_stack_strategy") {
        switch(mode) {
        case Mode::kDraw:
            fName.append("_draw");
            break;
        case Mode::kClipAndFill:
            fName.append("_clipandfill");
            break;
        case Mode::kPathOps:
            fName.append("_pathops");
            break;
        case Mode::kRasterize:
            fName.append("_rasterize");
            break;
        case Mode::kSWRasterize:
            fName.append("_rasterize_sw");
            break;
        case Mode::kClipOnly:
            fName.append("_cliponly");
            break;
        case Mode::kNewClipOnly:
            fName.append("_cliponly_new");
            break;
        case Mode::kPrepOnly:
            fName.append("_cliponly_prep");
            break;
        }

        if (index >= 0) {
            fName.appendf("_%d", kCounts[index]);
        }
    }

    bool runAsBench() const override { return fCountIndex >= 0; }

protected:
    static constexpr int kWidth = 400;
    static constexpr int kHeight = 400;

    static constexpr int kCounts[] = {1, 2, 3, 4,
                                      5, 6, 7, 8,
                                      9, 10, 11, 12, // @ 12, clip becomes empty
                                      20, 50, 100, 500};

    SkString onShortName() override {
        return fName;
    }

    SkISize onISize() override { return SkISize::Make(1024, 1024); }

    void onDraw(SkCanvas* canvas) override {
        // Special case for the clip-only varieties, minimize canvas interactions.
        if (fMode == Mode::kClipOnly || fMode == Mode::kNewClipOnly || fMode == Mode::kPrepOnly) {
            if (this->getMode() != kBench_Mode || fCountIndex < 0) {
                return;
            }

            if (fMode == Mode::kClipOnly) {
                this->clipAndFill(canvas, kCounts[fCountIndex], false);
            } else if (fMode == Mode::kNewClipOnly) {
                GrClipStack stack{SkIRect::MakeWH(kWidth, kHeight)};
                this->fillClipStack(canvas, &stack, kCounts[fCountIndex]);
            } else {
                this->prepClip(canvas, kCounts[fCountIndex]);
            }
            return;
        }

        if (this->getMode() != kBench_Mode)
            canvas->clear(SK_ColorBLACK);

        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 4; ++x) {
                int index = x + y * 4;
                if (fCountIndex >= 0 && fCountIndex != index)
                    continue;

                canvas->save();
                if (fCountIndex < 0)
                    canvas->translate((kWidth + 10.f) * x + 10.f, (kHeight + 10.f) * y + 10.f);
                canvas->clipRect(SkRect::MakeIWH(kWidth, kHeight));

                if (this->getMode() != kBench_Mode)
                    canvas->clear(SK_ColorDKGRAY);

                int count = kCounts[index];
                switch(fMode) {
                case Mode::kDraw:
                    this->drawClip(canvas, count);
                    break;
                case Mode::kClipAndFill:
                    this->clipAndFill(canvas, count);
                    break;
                case Mode::kPathOps:
                    this->pathOpsClip(canvas, count);
                    break;
                case Mode::kRasterize:
                    this->rasterClip(canvas, count);
                    break;
                case Mode::kSWRasterize:
                    this->swRasterClip(canvas, count);
                    break;
                default:
                    break;
                }

                canvas->restore();
            }
        }
    }

    void prepClip(SkCanvas* canvas, int count) const {
        SkRandom random;
        int vCount = 0;
        for (int i = 0; i < count; ++i) {
            SkPath path = MakeClipPath(&random, kWidth, kHeight);
            bool aa = random.nextBool();
            bool difference = random.nextBool();

            // Don't just optimize out the loop, but pretty trivial operations
            int pathVCount = path.getPoints(nullptr, 0);
            if (aa ^ difference) {
                vCount -= pathVCount;
            } else {
                vCount += pathVCount;
            }
        }
    }

    void fillClipStack(SkCanvas* canvas, GrClipStack* stack, int count, bool debug = false) const {
        SkRandom random;
        stack->save();
        for (int i = 0; i < count; ++i) {
            SkPath path = MakeClipPath(&random, kWidth, kHeight);
            bool aa = random.nextBool();
            SkClipOp op = random.nextBool() ? SkClipOp::kDifference : SkClipOp::kIntersect;

            stack->clipPath(SkMatrix::I(), path, aa, op);

            if (debug) {
                // Draw all the elements with red strokes
                SkPaint debugPaint;
                debugPaint.setStyle(SkPaint::kStroke_Style);
                debugPaint.setStrokeWidth(0.f);
                debugPaint.setAntiAlias(aa);
                if (op == SkClipOp::kDifference) {
                    debugPaint.setColor4f({1.f, 0.f, 0.f, 1.f});
                } else {
                    debugPaint.setColor4f({0.f, 1.f, 0.f, 1.f});
                }
                canvas->drawPath(path, debugPaint);
            }
        }
    }

    void rasterClip(SkCanvas* canvas, int count) const {
        GrClipStack stack(SkIRect::MakeWH(kWidth, kHeight));
        this->fillClipStack(canvas, &stack, count);

        // SkTArray<GrClipStack::DebugElement> elements;
        // stack.apply(SkRect::MakeIWH(kWidth, kHeight), &elements);
        // count = (int) elements.size();

        // canvas->save();
        // canvas->clipRect(stack.bounds(), false);

        if (this->getMode() != kBench_Mode) {
            canvas->clear(SK_ColorWHITE);
        }

        for (int i = 0; i < count; ++i) {
            // const auto& e = elements[i];

            SkPaint paint;
            // paint.setAntiAlias(e.aa);

            // SkPath path = e.path;
            // if (e.op == SkClipOp::kIntersect) {
            //     // Set every pixel outside the path to 0
            //     paint.setColor(SK_ColorTRANSPARENT);
            //     paint.setBlendMode(SkBlendMode::kModulate);
            //     path.toggleInverseFillType();
            // } else {
            //     // Set every pixel inside the path to 0
            //     paint.setColor(SK_ColorWHITE);
            //     paint.setBlendMode(SkBlendMode::kDstOut);
            // }

            // canvas->drawPath(path, paint);
        }
        canvas->restore();
    }

    void swRasterClip(SkCanvas* canvas, int count) const {
        sk_sp<SkSurface> swSurface = SkSurface::MakeRaster(
            canvas->imageInfo().makeWH(kWidth, kHeight).makeColorType(kAlpha_8_SkColorType),
            0, nullptr);
        this->rasterClip(swSurface->getCanvas(), count);
        canvas->drawImage(swSurface->makeImageSnapshot(), 0, 0, nullptr);
    }

    void drawClip(SkCanvas* canvas, int count) const {
        SkPaint p;
        p.setColor4f({1.f, 1.f, 1.f, 0.2f});

        GrClipStack stack(SkIRect::MakeWH(kWidth, kHeight));
        this->fillClipStack(canvas, &stack, count, true);

        // SkTArray<GrClipStack::DebugElement> elements;
        // auto analysis = stack.apply(SkRect::MakeIWH(kWidth, kHeight), &elements);


        // SkDebugf("Draw clip @ %d, returned elements = %d\n", count, elements.size());
        // analysis.dump();

        // count = (int) elements.size();

        for (int i = 0; i < count; ++i) {
            // const auto& e = elements[i];
            // SkPathDirection dir;
            // unsigned start;
            // SkRect dummy;
            // SkRRect dummyR;
            // const char* typeStr;
            // if (SkPathPriv::IsSimpleClosedRect(e.path, &dummy, &dir, &start)) {
            //     typeStr = "rect";
            // } else if (SkPathPriv::IsOval(e.path, &dummy, &dir, &start)) {
            //     typeStr = "oval";
            // } else if (SkPathPriv::IsRRect(e.path, &dummyR, &dir, &start)) {
            //     typeStr = "rrect";
            // } else if (e.path.getSegmentMasks() == SkPath::kLine_SegmentMask && e.path.isConvex()) {
            //     typeStr = "convex";
            // } else {
            //     typeStr = "path";
            // }
            // SkDebugf(" %d - op %d, aa %d, type: %s\n",
            //     i, (int) e.op, e.aa, typeStr);

            // p.setAntiAlias(e.aa);
            // SkPath path = e.path;
            // if (e.op == SkClipOp::kDifference) {
            //     path.toggleInverseFillType();
            // }
            // canvas->drawPath(path, p);
        }

        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(5.f);
        p.setAntiAlias(false);

        // Magenta for inner bounds
        p.setColor4f({1.f, 0.f, 1.f, .5f});
        // canvas->drawRect(stack.innerBounds(), p);
        // Cyan for outer bounds
        p.setColor4f({0.f, 1.f, 1.f, .5f});
        // canvas->drawRect(stack.outerBounds(), p);

        // Yellow for final bounds (if you don't see it, it's empty or wide)
        p.setColor4f({1.f, 1.f, 0.f, 1.f});
        p.setStrokeWidth(0.f);
        // canvas->drawRect(stack.bounds(), p);
    }

    void clipAndFill(SkCanvas* canvas, int count, bool finalClear = true) const {
        SkPaint p;

        // This approach does not use FakeClipStack since the actual clip stack should be
        // handling these optimizations
        SkRandom random;
        for (int i = 0; i < count; ++i) {
            SkPath path = MakeClipPath(&random, kWidth, kHeight);
            bool aa = random.nextBool();
            bool difference = random.nextBool();
            SkClipOp op = difference ? SkClipOp::kDifference : SkClipOp::kIntersect;
            canvas->clipPath(path, op, aa);
        }

        if (finalClear)
            canvas->clear(SK_ColorWHITE);
    }

    void pathOpsClip(SkCanvas* canvas, int count) const {
        GrClipStack stack(SkIRect::MakeWH(kWidth, kHeight));
        this->fillClipStack(canvas, &stack, count);

        // SkTArray<GrClipStack::DebugElement> elements;
        // stack.apply(SkRect::MakeIWH(kWidth, kHeight), &elements);
        // count = (int) elements.size();

        SkPath aaPath;
        SkPath bwPath;

        // count operations, to differentiate between no ops of a aa/bw flavor or
        // the shape actually became empty as the result of the ops.
        int aaCount = 0;
        int bwCount = 0;
        for (int i = 0; i < count; ++i) {
            // const auto& e = elements[i];
            // int* count = e.aa ? &aaCount : &bwCount;
            // SkPath* result = e.aa ? &aaPath : &bwPath;
            // if (*count == 0) {
            //     // First start from the device rectangle
            //     result->addRect(SkRect::MakeIWH(kWidth, kHeight));
            // }
            // (*count)++;

            // SkPathOp op = e.op == SkClipOp::kDifference ? kDifference_SkPathOp
            //                                             : kIntersect_SkPathOp;
            // Op(*result, e.path, op, result);
        }

        // simulate the fact we could make a smaller clip image.
        // canvas->clipRect(stack.bounds(), false);

        if (this->getMode() != kBench_Mode) {
            canvas->saveLayer(nullptr, nullptr);
            canvas->clear(SK_ColorWHITE); // full coverage
        }

        SkPaint p;
        p.setColor(SK_ColorWHITE);

        SkPaint layerBlend;
        layerBlend.setBlendMode(SkBlendMode::kSrcIn); // intersect between bw and aa
        if (bwCount > 0) {
            if (this->getMode() != kBench_Mode)
                canvas->saveLayer(nullptr, &layerBlend);

            canvas->drawPath(bwPath, p);

            if (this->getMode() != kBench_Mode)
                canvas->restore();
        }

        if (aaCount > 0) {
            p.setAntiAlias(true);
            if (this->getMode() != kBench_Mode)
                canvas->saveLayer(nullptr, &layerBlend);
            // May be empty in which case this layer remains transparent and properly zeroes out
            // any BW coverage from the earlier draw.
            canvas->drawPath(aaPath, p);

            if (this->getMode() != kBench_Mode)
                canvas->restore();
        }

        if (this->getMode() != kBench_Mode)
            canvas->restore();
    }

    static void MakeStar(SkPath* path, SkScalar cx, SkScalar cy, SkScalar r, int n) {
        SkScalar rad = -SK_ScalarPI / 2;
        const SkScalar drad = (n >> 1) * SK_ScalarPI * 2 / n;

        path->moveTo(cx, cy - r);
        for (int i = 1; i < n; i++) {
            rad += drad;
            path->quadTo(cx, cy, cx + SkScalarCos(rad) * r, cy + SkScalarSin(rad) * r);
        }
        path->close();
    }

    static SkPath MakeClipPath(SkRandom* random, int width, int height) {
        uint32_t pathType = random->nextULessThan(3);
        SkPath path;

        // Generate random centers of clips near the middle, to increase chance that
        // they overlap and make a meanginful intersection instead of quickly resulting in
        // an empty clip.

        float cx = random->nextRangeF(0.3f * width, 0.6f * width);
        float cy = random->nextRangeF(0.3f * width, 0.6f * height);
        float maxRadius = (width + height) / 2.f;
        float minRadius = maxRadius / 10.f;

        if (pathType == 0) {
            // Rect
            float w = random->nextRangeF(minRadius, maxRadius);
            float h = random->nextRangeF(minRadius, maxRadius);
            SkRect rect = SkRect::MakeXYWH(cx - w/2.f, cy - h/2.f, w, h);
            path.addRect(rect);
        } else if (pathType == 1) {
            // Round rect
            float w = random->nextRangeF(minRadius, maxRadius);
            float h = random->nextRangeF(minRadius, maxRadius);
            SkRect rect = SkRect::MakeXYWH(cx - w/2.f, cy - h/2.f, w, h);

            float xRadius = random->nextRangeF(0.f, rect.width() / 2.f);
            float yRadius = random->nextRangeF(0.f, rect.height() / 2.f);
            SkRRect rrect = SkRRect::MakeRectXY(rect, xRadius, yRadius);
            path.addRRect(rrect);
        } else if (pathType == 2) {
            // Circle
            float radius = random->nextRangeF(minRadius, maxRadius);
            path.addCircle(cx, cy, radius);
        } else {
            // Complicated transformed path
            SkASSERT(pathType == 3);
            float radius = random->nextRangeF(minRadius, maxRadius);
            int n = (int) random->nextRangeU(3, 8);
            if (n % 2 == 0) {
                n = n + 1;
            }
            MakeStar(&path, cx, cy, radius, n);
        }

        return path;
    }

    Mode     fMode;
    int      fCountIndex;
    SkString fName;

    typedef GM INHERITED;
};

DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kDraw);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kClipAndFill);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kPathOps);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kRasterize);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kSWRasterize);)

DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kClipAndFill, 0);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kPathOps, 0);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kRasterize, 0);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kSWRasterize, 0);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kClipOnly, 0);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kNewClipOnly, 0);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kPrepOnly, 0);)

DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kClipAndFill, 4);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kPathOps, 4);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kRasterize, 4);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kSWRasterize, 4);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kClipOnly, 4);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kNewClipOnly, 4);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kPrepOnly, 4);)

DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kClipAndFill, 9);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kPathOps, 9);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kRasterize, 9);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kSWRasterize, 9);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kClipOnly, 9);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kNewClipOnly, 9);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kPrepOnly, 9);)

DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kClipAndFill, 12);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kPathOps, 12);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kRasterize, 12);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kSWRasterize, 12);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kClipOnly, 12);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kNewClipOnly, 12);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kPrepOnly, 12);)

DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kClipAndFill, 13);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kPathOps, 13);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kRasterize, 13);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kSWRasterize, 13);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kClipOnly, 13);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kNewClipOnly, 13);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kPrepOnly, 13);)

DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kClipAndFill, 14);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kPathOps, 14);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kRasterize, 14);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kSWRasterize, 14);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kClipOnly, 14);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kNewClipOnly, 14);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kPrepOnly, 14);)

DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kClipAndFill, 15);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kPathOps, 15);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kRasterize, 15);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kSWRasterize, 15);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kClipOnly, 15);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kNewClipOnly, 15);)
DEF_GM( return new ClipStackStrategyGM(ClipStackStrategyGM::Mode::kPrepOnly, 15);)

}
