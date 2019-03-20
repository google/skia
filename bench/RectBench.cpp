/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "CommandLineFlags.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkShader.h"
#include "SkString.h"

DEFINE_double(strokeWidth, -1.0, "If set, use this stroke width in RectBench.");

class RectBench : public Benchmark {
public:
    int fShift, fStroke;
    enum {
        W = 640,
        H = 480,
        N = 300,
    };
    SkRect  fRects[N];
    SkColor fColors[N];
    bool    fAA;
    bool    fPerspective;

    RectBench(int shift, int stroke = 0, bool aa = true, bool perspective = false)
        : fShift(shift)
        , fStroke(stroke)
        , fAA(aa)
        , fPerspective(perspective) {}

    const char* computeName(const char root[]) {
        fBaseName.printf("%s_%d", root, fShift);
        if (fStroke > 0) {
            fBaseName.appendf("_stroke_%d", fStroke);
        }
        if (fAA) {
            fBaseName.appendf("_aa");
        } else {
            fBaseName.appendf("_bw");
        }
        if (fPerspective) {
            fBaseName.appendf("_persp");
        }
        return fBaseName.c_str();
    }

protected:

    virtual void drawThisRect(SkCanvas* c, const SkRect& r, const SkPaint& p) {
        c->drawRect(r, p);
    }

    const char* onGetName() override { return computeName("rects"); }

    void onDelayedSetup() override {
        SkRandom rand;
        const SkScalar offset = SK_Scalar1/3;
        for (int i = 0; i < N; i++) {
            int x = rand.nextU() % W;
            int y = rand.nextU() % H;
            int w = rand.nextU() % W;
            int h = rand.nextU() % H;
            w >>= fShift;
            h >>= fShift;
            x -= w/2;
            y -= h/2;
            fRects[i].set(SkIntToScalar(x), SkIntToScalar(y),
                          SkIntToScalar(x+w), SkIntToScalar(y+h));
            fRects[i].offset(offset, offset);
            fColors[i] = rand.nextU() | 0xFF808080;
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;
        if (fStroke > 0) {
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(SkIntToScalar(fStroke));
        }
        if (fPerspective) {
            // Apply some fixed perspective to change how ops may draw the rects
            SkMatrix perspective;
            perspective.setIdentity();
            perspective.setPerspX(1e-4f);
            perspective.setPerspY(1e-3f);
            perspective.setSkewX(0.1f);
            canvas->concat(perspective);
        }
        for (int i = 0; i < loops; i++) {
            paint.setColor(fColors[i % N]);
            this->setupPaint(&paint);
            this->drawThisRect(canvas, fRects[i % N], paint);
        }
    }

    void setupPaint(SkPaint* paint) override {
        this->INHERITED::setupPaint(paint);
        paint->setAntiAlias(fAA);
    }

private:
    SkString fBaseName;
    typedef Benchmark INHERITED;
};

class SrcModeRectBench : public RectBench {
public:
    SrcModeRectBench() : INHERITED(1, 0) {
        fMode = SkBlendMode::kSrc;
    }

protected:
    void setupPaint(SkPaint* paint) override {
        this->INHERITED::setupPaint(paint);
        // srcmode is most interesting when we're not opaque
        paint->setAlpha(0x80);
        paint->setBlendMode(fMode);
    }

    const char* onGetName() override {
        fName.set(this->INHERITED::onGetName());
        fName.prepend("srcmode_");
        return fName.c_str();
    }

private:
    SkBlendMode fMode;
    SkString fName;

    typedef RectBench INHERITED;
};

class TransparentRectBench : public RectBench {
public:
    TransparentRectBench() : INHERITED(1, 0) {}

protected:
    void setupPaint(SkPaint* paint) override {
        this->INHERITED::setupPaint(paint);
        // draw non opaque rect
        paint->setAlpha(0x80);
    }

    const char* onGetName() override {
        fName.set(this->INHERITED::onGetName());
        fName.prepend("transparent_");
        return fName.c_str();
    }

private:
    SkString fName;
    typedef RectBench INHERITED;
};

// Adds a shader to the paint that requires local coordinates to be used
class LocalCoordsRectBench : public RectBench {
public:
    LocalCoordsRectBench(bool aa, bool perspective = false) : INHERITED(1, 0, aa, perspective) { }

protected:
    void onDelayedSetup() override {
        this->INHERITED::onDelayedSetup();
        // Create the shader once, so that isn't included in the timing
        SkPoint pts[2] = { {0.f, 0.f}, {50.f, 50.f} };
        SkColor colors[] = { SK_ColorWHITE, SK_ColorBLUE };
        fShader = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkShader::kClamp_TileMode);
    }

    void setupPaint(SkPaint* paint) override {
        this->INHERITED::setupPaint(paint);
        paint->setShader(fShader);
    }

    const char* onGetName() override {
        fName.set(this->INHERITED::onGetName());
        fName.append("_localcoords");
        return fName.c_str();
    }

private:
    SkString fName;
    sk_sp<SkShader> fShader;

    typedef RectBench INHERITED;
};


class OvalBench : public RectBench {
public:
    OvalBench(int shift, int stroke = 0) : RectBench(shift, stroke) {}
protected:
    void drawThisRect(SkCanvas* c, const SkRect& r, const SkPaint& p) override {
        c->drawOval(r, p);
    }
    const char* onGetName() override { return computeName("ovals"); }
};

class RRectBench : public RectBench {
public:
    RRectBench(int shift, int stroke = 0) : RectBench(shift, stroke) {}
protected:
    void drawThisRect(SkCanvas* c, const SkRect& r, const SkPaint& p) override {
        c->drawRoundRect(r, r.width() / 4, r.height() / 4, p);
    }
    const char* onGetName() override { return computeName("rrects"); }
};

class PointsBench : public RectBench {
public:
    SkCanvas::PointMode fMode;

    PointsBench(SkCanvas::PointMode mode, const char* name)
        : RectBench(2)
        , fMode(mode) {
        fName = name;
    }

protected:
    void onDraw(int loops, SkCanvas* canvas) override {
        SkScalar gSizes[] = {
            SkIntToScalar(7), 0
        };
        size_t sizes = SK_ARRAY_COUNT(gSizes);

        if (FLAGS_strokeWidth >= 0) {
            gSizes[0] = (SkScalar)FLAGS_strokeWidth;
            sizes = 1;
        }

        SkPaint paint;
        paint.setStrokeCap(SkPaint::kRound_Cap);

        for (int loop = 0; loop < loops; loop++) {
            for (size_t i = 0; i < sizes; i++) {
                paint.setStrokeWidth(gSizes[i]);
                this->setupPaint(&paint);
                canvas->drawPoints(fMode, N * 2, reinterpret_cast<SkPoint*>(fRects), paint);
                paint.setColor(fColors[i % N]);
            }
        }
    }
    const char* onGetName() override { return fName.c_str(); }

private:
    SkString fName;

};

/*******************************************************************************
 * to bench BlitMask [Opaque, Black, color, shader]
 *******************************************************************************/

class BlitMaskBench : public RectBench {
public:
    enum kMaskType {
        kMaskOpaque = 0,
        kMaskBlack,
        kMaskColor,
        KMaskShader
    };
    SkCanvas::PointMode fMode;

    BlitMaskBench(SkCanvas::PointMode mode,
                  BlitMaskBench::kMaskType type, const char* name) :
        RectBench(2), fMode(mode), _type(type) {
        fName = name;
    }

protected:
    void onDraw(int loops, SkCanvas* canvas) override {
        SkScalar gSizes[] = {
            SkIntToScalar(13), SkIntToScalar(24)
        };
        size_t sizes = SK_ARRAY_COUNT(gSizes);

        if (FLAGS_strokeWidth >= 0) {
            gSizes[0] = (SkScalar)FLAGS_strokeWidth;
            sizes = 1;
        }
        SkRandom rand;
        SkColor color = 0xFF000000;
        U8CPU alpha = 0xFF;
        SkPaint paint;
        paint.setStrokeCap(SkPaint::kRound_Cap);
        if (_type == KMaskShader) {
            SkBitmap srcBM;
            srcBM.allocN32Pixels(10, 1);
            srcBM.eraseColor(0xFF00FF00);

            paint.setShader(SkShader::MakeBitmapShader(srcBM, SkShader::kClamp_TileMode,
                                                       SkShader::kClamp_TileMode));
        }
        for (int loop = 0; loop < loops; loop++) {
            for (size_t i = 0; i < sizes; i++) {
                switch (_type) {
                    case kMaskOpaque:
                        color = fColors[i];
                        alpha = 0xFF;
                        break;
                    case kMaskBlack:
                        alpha = 0xFF;
                        color = 0xFF000000;
                        break;
                    case kMaskColor:
                        color = fColors[i];
                        alpha = rand.nextU() & 255;
                        break;
                    case KMaskShader:
                        break;
                }
                paint.setStrokeWidth(gSizes[i]);
                this->setupPaint(&paint);
                paint.setColor(color);
                paint.setAlpha(alpha);
                canvas->drawPoints(fMode, N * 2, reinterpret_cast<SkPoint*>(fRects), paint);
           }
        }
    }
    const char* onGetName() override { return fName.c_str(); }

private:
    typedef RectBench INHERITED;
    kMaskType _type;
    SkString fName;
};

// AA rects
DEF_BENCH(return new RectBench(1, 0, true);)
DEF_BENCH(return new RectBench(1, 4, true);)
DEF_BENCH(return new RectBench(3, 0, true);)
DEF_BENCH(return new RectBench(3, 4, true);)
// Non-AA rects
DEF_BENCH(return new RectBench(1, 0, false);)
DEF_BENCH(return new RectBench(1, 4, false);)
DEF_BENCH(return new RectBench(3, 0, false);)
DEF_BENCH(return new RectBench(3, 4, false);)

DEF_BENCH(return new OvalBench(1);)
DEF_BENCH(return new OvalBench(3);)
DEF_BENCH(return new OvalBench(1, 4);)
DEF_BENCH(return new OvalBench(3, 4);)
DEF_BENCH(return new RRectBench(1);)
DEF_BENCH(return new RRectBench(1, 4);)
DEF_BENCH(return new RRectBench(3);)
DEF_BENCH(return new RRectBench(3, 4);)
DEF_BENCH(return new PointsBench(SkCanvas::kPoints_PointMode, "points");)
DEF_BENCH(return new PointsBench(SkCanvas::kLines_PointMode, "lines");)
DEF_BENCH(return new PointsBench(SkCanvas::kPolygon_PointMode, "polygon");)

DEF_BENCH(return new SrcModeRectBench();)

DEF_BENCH(return new TransparentRectBench();)

DEF_BENCH(return new LocalCoordsRectBench(true);)
DEF_BENCH(return new LocalCoordsRectBench(false);)

// Perspective rects
DEF_BENCH(return new RectBench(1, 0, true, true);)
DEF_BENCH(return new RectBench(1, 0, false, true);)
DEF_BENCH(return new LocalCoordsRectBench(true, true);)
DEF_BENCH(return new LocalCoordsRectBench(false, true);)

/* init the blitmask bench
 */
DEF_BENCH(return new BlitMaskBench(SkCanvas::kPoints_PointMode,
                                   BlitMaskBench::kMaskOpaque,
                                   "maskopaque");)
DEF_BENCH(return new BlitMaskBench(SkCanvas::kPoints_PointMode,
                                   BlitMaskBench::kMaskBlack,
                                   "maskblack");)
DEF_BENCH(return new BlitMaskBench(SkCanvas::kPoints_PointMode,
                                   BlitMaskBench::kMaskColor,
                                   "maskcolor");)
DEF_BENCH(return new BlitMaskBench(SkCanvas::kPoints_PointMode,
                                   BlitMaskBench::KMaskShader,
                                   "maskshader");)
