/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/ganesh/GrCanvas.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/effects/GrPorterDuffXferProcessor.h"
#include "src/gpu/ganesh/effects/GrRRectEffect.h"
#include "src/gpu/ganesh/ops/FillRectOp.h"
#include "src/gpu/ganesh/ops/GrDrawOp.h"

#include <memory>
#include <utility>

namespace skiagm {

///////////////////////////////////////////////////////////////////////////////

class RRectGM : public GM {
public:
    enum Type {
        kBW_Draw_Type,
        kAA_Draw_Type,
        kBW_Clip_Type,
        kAA_Clip_Type,
        kEffect_Type,
    };
    RRectGM(Type type) : fType(type) { }

protected:

    void onOnceBeforeDraw() override {
        this->setBGColor(0xFFDDDDDD);
        this->setUpRRects();
    }

    SkString getName() const override {
        SkString name("rrect");
        switch (fType) {
            case kBW_Draw_Type:
                name.append("_draw_bw");
                break;
            case kAA_Draw_Type:
                name.append("_draw_aa");
                break;
            case kBW_Clip_Type:
                name.append("_clip_bw");
                break;
            case kAA_Clip_Type:
                name.append("_clip_aa");
                break;
            case kEffect_Type:
                name.append("_effect");
                break;
        }
        return name;
    }

    SkISize getISize() override { return SkISize::Make(kImageWidth, kImageHeight); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        auto sdc = skgpu::ganesh::TopDeviceSurfaceDrawContext(canvas);

        auto rContext = canvas->recordingContext();
        if (kEffect_Type == fType && (!sdc || !rContext)) {
            *errorMsg = kErrorMsg_DrawSkippedGpuOnly;
            return DrawResult::kSkip;
        }

        SkPaint paint;
        if (kAA_Draw_Type == fType) {
            paint.setAntiAlias(true);
        }

        if (fType == kBW_Clip_Type || fType == kAA_Clip_Type) {
            // Add a gradient to the paint to ensure local coords are respected.
            SkPoint pts[3] = {{0, 0}, {1.5f, 1}};
            SkColor colors[3] = {SK_ColorBLACK, SK_ColorYELLOW};
            paint.setShader(SkGradientShader::MakeLinear(pts, colors, nullptr, 2,
                                                         SkTileMode::kClamp));
        }

#ifdef SK_DEBUG
        const SkRect kMaxImageBound = SkRect::MakeWH(SkIntToScalar(kImageWidth),
                                                     SkIntToScalar(kImageHeight));
#endif

        int lastEdgeType = (kEffect_Type == fType) ? (int) GrClipEdgeType::kLast: 0;

        int y = 1;
        for (int et = 0; et <= lastEdgeType; ++et) {
            int x = 1;
            for (int curRRect = 0; curRRect < kNumRRects; ++curRRect) {
                bool drew = true;
#ifdef SK_DEBUG
                if (curRRect != kNumRRects - 1) { // skip last rrect, which is large but clipped
                    SkRect imageSpaceBounds = fRRects[curRRect].getBounds();
                    imageSpaceBounds.offset(SkIntToScalar(x), SkIntToScalar(y));
                    SkASSERT(kMaxImageBound.contains(imageSpaceBounds));
                }
#endif
                canvas->save();
                    canvas->translate(SkIntToScalar(x), SkIntToScalar(y));

                    SkRRect rrect = fRRects[curRRect];
                    if (curRRect == kNumRRects - 1) {
                        canvas->clipRect({0, 0, kTileX - 2, kTileY - 2});
                        canvas->translate(-0.14f * rrect.rect().width(),
                                          -0.14f * rrect.rect().height());
                    }
                    if (kEffect_Type == fType) {
                        fRRects[curRRect].transform(canvas->getLocalToDeviceAs3x3(), &rrect);

                        GrClipEdgeType edgeType = (GrClipEdgeType) et;
                        const auto& caps = *rContext->priv().caps()->shaderCaps();
                        auto [success, fp] = GrRRectEffect::Make(/*inputFP=*/nullptr,
                                                                 edgeType, rrect, caps);
                        if (success) {
                            GrPaint grPaint;
                            grPaint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));
                            grPaint.setCoverageFragmentProcessor(std::move(fp));
                            grPaint.setColor4f({ 0, 0, 0, 1.f });

                            SkRect bounds = rrect.getBounds();
                            bounds.intersect(SkRect::MakeXYWH(x, y, kTileX - 2, kTileY - 2));
                            if (et >= (int) GrClipEdgeType::kInverseFillBW) {
                                bounds.outset(2.f, 2.f);
                            }

                            sdc->addDrawOp(skgpu::ganesh::FillRectOp::MakeNonAARect(
                                    rContext, std::move(grPaint), SkMatrix::I(), bounds));
                        } else {
                            drew = false;
                        }
                    } else if (fType == kBW_Clip_Type || fType == kAA_Clip_Type) {
                        bool aaClip = (kAA_Clip_Type == fType);
                        canvas->clipRRect(rrect, aaClip);
                        canvas->setMatrix(SkMatrix::Scale(kImageWidth, kImageHeight));
                        canvas->drawRect(SkRect::MakeWH(1, 1), paint);
                    } else {
                        canvas->drawRRect(rrect, paint);
                    }

                canvas->restore();
                if (drew) {
                    x = x + kTileX;
                    if (x > kImageWidth) {
                        x = 1;
                        y += kTileY;
                    }
                }
            }
            if (x != 1) {
                y += kTileY;
            }
        }
        return DrawResult::kOk;
    }

    void setUpRRects() {
        // each RRect must fit in a 0x0 -> (kTileX-2)x(kTileY-2) block. These will be tiled across
        // the screen in kTileX x kTileY tiles. The extra empty pixels on each side are for AA.

        // simple cases
        fRRects[0].setRect(SkRect::MakeWH(kTileX-2, kTileY-2));
        fRRects[1].setOval(SkRect::MakeWH(kTileX-2, kTileY-2));
        fRRects[2].setRectXY(SkRect::MakeWH(kTileX-2, kTileY-2), 10, 10);
        fRRects[3].setRectXY(SkRect::MakeWH(kTileX-2, kTileY-2), 10, 5);
        // small circular corners are an interesting test case for gpu clipping
        fRRects[4].setRectXY(SkRect::MakeWH(kTileX-2, kTileY-2), 1, 1);
        fRRects[5].setRectXY(SkRect::MakeWH(kTileX-2, kTileY-2), 0.5f, 0.5f);
        fRRects[6].setRectXY(SkRect::MakeWH(kTileX-2, kTileY-2), 0.2f, 0.2f);

        // The first complex case needs special handling since it is a square
        fRRects[kNumSimpleCases].setRectRadii(SkRect::MakeWH(kTileY-2, kTileY-2), gRadii[0]);
        for (size_t i = 1; i < std::size(gRadii); ++i) {
            fRRects[kNumSimpleCases+i].setRectRadii(SkRect::MakeWH(kTileX-2, kTileY-2), gRadii[i]);
        }
        // The last case is larger than kTileX-2 x kTileY-2 but will be drawn at an offset
        // into a clip rect that respects the tile size and highlights the rrect's corner curve.
        fRRects[kNumRRects - 1].setRectXY({9.f, 9.f, 1699.f, 1699.f}, 843.749f, 843.75f);
    }

private:
    Type fType;

    inline static constexpr int kImageWidth = 640;
    inline static constexpr int kImageHeight = 480;

    inline static constexpr int kTileX = 80;
    inline static constexpr int kTileY = 40;

    inline static constexpr int kNumSimpleCases = 7;
    inline static constexpr int kNumComplexCases = 35;

    static const SkVector gRadii[kNumComplexCases][4];

    inline static constexpr int kNumRRects = kNumSimpleCases + kNumComplexCases + 1 /* extra big */;
    SkRRect fRRects[kNumRRects];

    using INHERITED = GM;
};

// Radii for the various test cases. Order is UL, UR, LR, LL
const SkVector RRectGM::gRadii[kNumComplexCases][4] = {
    // a circle
    { { kTileY, kTileY }, { kTileY, kTileY }, { kTileY, kTileY }, { kTileY, kTileY } },

    // odd ball cases
    { { 8, 8 }, { 32, 32 }, { 8, 8 }, { 32, 32 } },
    { { 16, 8 }, { 8, 16 }, { 16, 8 }, { 8, 16 } },
    { { 0, 0 }, { 16, 16 }, { 8, 8 }, { 32, 32 } },

    // UL
    { { 30, 30 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
    { { 30, 15 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
    { { 15, 30 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },

    // UR
    { { 0, 0 }, { 30, 30 }, { 0, 0 }, { 0, 0 } },
    { { 0, 0 }, { 30, 15 }, { 0, 0 }, { 0, 0 } },
    { { 0, 0 }, { 15, 30 }, { 0, 0 }, { 0, 0 } },

    // LR
    { { 0, 0 }, { 0, 0 }, { 30, 30 }, { 0, 0 } },
    { { 0, 0 }, { 0, 0 }, { 30, 15 }, { 0, 0 } },
    { { 0, 0 }, { 0, 0 }, { 15, 30 }, { 0, 0 } },

    // LL
    { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 30, 30 } },
    { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 30, 15 } },
    { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 15, 30 } },

    // over-sized radii
    { { 0, 0 }, { 100, 400 }, { 0, 0 }, { 0, 0 } },
    { { 0, 0 }, { 400, 400 }, { 0, 0 }, { 0, 0 } },
    { { 400, 400 }, { 400, 400 }, { 400, 400 }, { 400, 400 } },

    // circular corner tabs
    { { 0, 0 }, { 20, 20 }, { 20, 20 }, { 0, 0 } },
    { { 20, 20 }, { 20, 20 }, { 0, 0 }, { 0, 0 } },
    { { 0, 0 }, { 0, 0 }, { 20, 20 }, { 20, 20 } },
    { { 20, 20 }, { 0, 0 }, { 0, 0 }, { 20, 20 } },

    // small radius circular corner tabs
    { { 0, 0 }, { 0.2f, 0.2f }, { 0.2f, 0.2f }, { 0, 0 } },
    { { 0.3f, 0.3f }, { 0.3f, .3f }, { 0, 0 }, { 0, 0 } },

    // single circular corner cases
    { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 15, 15 } },
    { { 0, 0 }, { 0, 0 }, { 15, 15 }, { 0, 0 } },
    { { 0, 0 }, { 15, 15 }, { 0, 0 }, { 0, 0 } },
    { { 15, 15 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },

    // nine patch elliptical
    { { 5, 7 }, { 8, 7 }, { 8, 12 }, { 5, 12 } },
    { { 0, 7 }, { 8, 7 }, { 8, 12 }, { 0, 12 } },

    // nine patch elliptical, small radii
    { { 0.4f, 7 }, { 8, 7 }, { 8, 12 }, { 0.4f, 12 } },
    { { 0.4f, 0.4f }, { 8, 0.4f }, { 8, 12 }, { 0.4f, 12 } },
    { { 20, 0.4f }, { 18, 0.4f }, { 18, 0.4f }, { 20, 0.4f } },
    { { 0.3f, 0.4f }, { 0.3f, 0.4f }, { 0.3f, 0.4f }, { 0.3f, 0.4f } },

};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new RRectGM(RRectGM::kAA_Draw_Type); )
DEF_GM( return new RRectGM(RRectGM::kBW_Draw_Type); )
DEF_GM( return new RRectGM(RRectGM::kAA_Clip_Type); )
DEF_GM( return new RRectGM(RRectGM::kBW_Clip_Type); )
DEF_GM( return new RRectGM(RRectGM::kEffect_Type); )

// This GM is designed to test a variety of fill and stroked rectangles and round rectangles, with
// different stroke width and join type scenarios. The geometry parameters are chosen so that
// Graphite should be able to use its AnalyticRoundRectRenderStep and batch into a single draw.
DEF_SIMPLE_GM(stroke_rect_rrects, canvas, 1350, 700) {
    canvas->scale(0.5f, 0.5f);
    canvas->translate(50.f, 50.f);

    auto draw = [&](int cx, int cy, bool rrect, float width, SkPaint::Join join) {
        SkPaint p;
        p.setAntiAlias(true);
        p.setStrokeWidth(width);
        p.setStyle(width >= 0.f ? SkPaint::kStroke_Style : SkPaint::kFill_Style);
        p.setStrokeJoin(join);

        canvas->save();
        canvas->translate(cx * 110.f, cy * 110.f);
        float dx = cx % 2 ? 0.5f : 0.f;
        float dy = cy % 2 ? 0.5f : 0.f;
        SkRect rect = SkRect::MakeWH(50.f, 40.f);
        rect.offset(dx, dy);

        if (width < 0.0) {
            rect.outset(25.f, 25.f); // make it the same size as the largest stroke
        }

        // Filled rounded rects can have arbitrary corners
        float cornerScale = std::min(rect.width(), rect.height());
        SkVector outerRadii[4] = { { 0.25f * cornerScale, 0.75f * cornerScale },
                                   { 0.f, 0.f},
                                   { 0.50f * cornerScale, 0.50f * cornerScale },
                                   { 0.75f * cornerScale, 0.25f * cornerScale } };
        // Stroked rounded rects will only have circular corners so that they remain compatible with
        // Graphite's AnalyticRoundRectRenderStep's requirements.
        SkVector strokeRadii[4] = { { 0.25f * cornerScale, 0.25f * cornerScale },
                                    { 0.f, 0.f }, // this corner matches join type
                                    { 0.50f * cornerScale, 0.50f * cornerScale },
                                    { 0.75f * cornerScale, 0.75f * cornerScale } };

        if (rrect) {
            SkRRect r;
            if (width >= 0.0) {
                r.setRectRadii(rect, strokeRadii);
            } else {
                r.setRectRadii(rect, outerRadii);
            }
            canvas->drawRRect(r, p);
        } else {
            canvas->drawRect(rect, p);
        }
        canvas->restore();
    };

    // The stroke widths are chosen to test when the inner stroke edges have completely crossed
    // over (50); when the inner corner circles intersect each other (30); a typical "nice"
    // stroke (10); a skinny stroke (1); and a hairline (0).
    int i = 0;
    for (float width : {-1.f, 50.f, 30.f, 10.f, 1.f, 0.f}) {
        int j = 0;
        for (SkPaint::Join join : { SkPaint::kMiter_Join,
                                    SkPaint::kBevel_Join,
                                    SkPaint::kRound_Join }) {
            if (width < 0 && join != SkPaint::kMiter_Join) {
                continue; // Don't repeat fills, since join type is ignored
            }
            draw(2*i, 2*j, false, width, join);
            draw(2*i+1, 2*j, false, width, join);
            draw(2*i, 2*j+1, false, width, join);
            draw(2*i+1, 2*j+1, false, width, join);
            j++;
        }
        i++;
    }

    canvas->translate(0.f, 50.f);

    i = 0;
    for (float width : {-1.f, 50.f, 30.f, 10.f, 1.f, 0.f}) {
        int j = 3;
        for (SkPaint::Join join : { SkPaint::kMiter_Join,
                                    SkPaint::kBevel_Join,
                                    SkPaint::kRound_Join }) {
            if (width < 0 && join != SkPaint::kMiter_Join) {
                continue;
            }
            draw(2*i, 2*j, true, width, join);
            draw(2*i+1, 2*j, true, width, join);
            draw(2*i, 2*j+1, true, width, join);
            draw(2*i+1, 2*j+1, true, width, join);
            j++;
        }
        i++;
    }

    // Rotated "footballs"
    auto drawComplex = [&](int cx, int cy, float width, float stretch) {
        SkPaint p;
        p.setAntiAlias(true);
        p.setStrokeWidth(width);
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeJoin(SkPaint::kBevel_Join);

        canvas->save();
        canvas->translate(cx * 110.f, cy * 110.f);

        SkRect rect = SkRect::MakeWH(cx % 2 ? 50.f : (40.f + stretch),
                                     cx % 2 ? (40.f + stretch) : 50.f);
        const SkVector kBigCorner{30.f, 30.f};
        const SkVector kRectCorner{0.f, 0.f};

        SkVector strokeRadii[4] = { cy % 2 ? kRectCorner : kBigCorner,
                                    cy % 2 ? kBigCorner : kRectCorner,
                                    cy % 2 ? kRectCorner : kBigCorner,
                                    cy % 2 ? kBigCorner : kRectCorner };

        SkRRect r;
        r.setRectRadii(rect, strokeRadii);
        canvas->drawRRect(r, p);

        canvas->restore();
    };

    canvas->translate(0.f, -50.f);
    i = 6;
    for (float width : {50.f, 30.f, 20.f, 10.f, 1.f, 0.f}) {
        int j = 0;
        for (float stretch: {0.f, 5.f, 10.f}) {
            drawComplex(2*i, 2*j, width, stretch);
            drawComplex(2*i+1, 2*j, width, stretch);
            drawComplex(2*i, 2*j+1, width, stretch);
            drawComplex(2*i+1, 2*j+1, width, stretch);
            j++;
        }
        i++;
    }

    // Rotated "D"s
    auto drawComplex2 = [&](int cx, int cy, float width, float stretch) {
        SkPaint p;
        p.setAntiAlias(true);
        p.setStrokeWidth(width);
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeJoin(SkPaint::kMiter_Join);

        canvas->save();
        canvas->translate(cx * 110.f, cy * 110.f);

        SkRect rect = SkRect::MakeWH(cx % 2 ? 50.f : (40.f + stretch),
                                     cx % 2 ? (40.f + stretch) : 50.f);
        const SkVector kBigCorner{30.f, 30.f};
        const SkVector kRectCorner{0.f, 0.f};

        SkVector strokeRadii[4] = { cx % 2 ? kRectCorner : kBigCorner,
                                    (cx % 2) ^ (cy % 2) ? kBigCorner : kRectCorner,
                                    cx % 2 ? kBigCorner : kRectCorner,
                                    (cx % 2) ^ (cy % 2) ? kRectCorner : kBigCorner };

        SkRRect r;
        r.setRectRadii(rect, strokeRadii);
        canvas->drawRRect(r, p);

        canvas->restore();
    };

    canvas->translate(0.f, 50.f);
    i = 6;
    for (float width : {50.f, 30.f, 20.f, 10.f, 1.f, 0.f}) {
        int j = 3;
        for (float stretch: {0.f, 5.f, 10.f}) {
            drawComplex2(2*i, 2*j, width, stretch);
            drawComplex2(2*i+1, 2*j, width, stretch);
            drawComplex2(2*i, 2*j+1, width, stretch);
            drawComplex2(2*i+1, 2*j+1, width, stretch);
            j++;
        }
        i++;
    }
}

}  // namespace skiagm
