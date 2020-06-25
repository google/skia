/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/effects/GrRRectEffect.h"
#include "src/gpu/ops/GrFillRectOp.h"
#include "src/gpu/ops/GrFillRRectOp.h"
#include "src/gpu/ops/GrOvalOpFactory.h"
#include "tools/ToolUtils.h"

typedef void (*InsetProc)(const SkRRect&, SkScalar dx, SkScalar dy, SkRRect*);

static void inset0(const SkRRect& src, SkScalar dx, SkScalar dy, SkRRect* dst) {
    SkRect r = src.rect();

    r.inset(dx, dy);
    if (r.isEmpty()) {
        dst->setEmpty();
        return;
    }

    SkVector radii[4];
    for (int i = 0; i < 4; ++i) {
        radii[i] = src.radii((SkRRect::Corner)i);
    }
    for (int i = 0; i < 4; ++i) {
        radii[i].fX -= dx;
        radii[i].fY -= dy;
    }
    dst->setRectRadii(r, radii);
}

static void inset1(const SkRRect& src, SkScalar dx, SkScalar dy, SkRRect* dst) {
    SkRect r = src.rect();

    r.inset(dx, dy);
    if (r.isEmpty()) {
        dst->setEmpty();
        return;
    }

    SkVector radii[4];
    for (int i = 0; i < 4; ++i) {
        radii[i] = src.radii((SkRRect::Corner)i);
    }
    dst->setRectRadii(r, radii);
}

static void inset2(const SkRRect& src, SkScalar dx, SkScalar dy, SkRRect* dst) {
    SkRect r = src.rect();

    r.inset(dx, dy);
    if (r.isEmpty()) {
        dst->setEmpty();
        return;
    }

    SkVector radii[4];
    for (int i = 0; i < 4; ++i) {
        radii[i] = src.radii((SkRRect::Corner)i);
    }
    for (int i = 0; i < 4; ++i) {
        if (radii[i].fX) {
            radii[i].fX -= dx;
        }
        if (radii[i].fY) {
            radii[i].fY -= dy;
        }
    }
    dst->setRectRadii(r, radii);
}

static SkScalar prop(SkScalar radius, SkScalar newSize, SkScalar oldSize) {
    return newSize * radius / oldSize;
}

static void inset3(const SkRRect& src, SkScalar dx, SkScalar dy, SkRRect* dst) {
    SkRect r = src.rect();

    r.inset(dx, dy);
    if (r.isEmpty()) {
        dst->setEmpty();
        return;
    }

    SkVector radii[4];
    for (int i = 0; i < 4; ++i) {
        radii[i] = src.radii((SkRRect::Corner)i);
    }
    for (int i = 0; i < 4; ++i) {
        radii[i].fX = prop(radii[i].fX, r.width(), src.rect().width());
        radii[i].fY = prop(radii[i].fY, r.height(), src.rect().height());
    }
    dst->setRectRadii(r, radii);
}

static void draw_rrect_color(SkCanvas* canvas, const SkRRect& rrect) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);

    if (rrect.isRect()) {
        paint.setColor(SK_ColorRED);
    } else if (rrect.isOval()) {
        paint.setColor(ToolUtils::color_to_565(0xFF008800));
    } else if (rrect.isSimple()) {
        paint.setColor(SK_ColorBLUE);
    } else {
        paint.setColor(SK_ColorBLACK);
    }
    canvas->drawRRect(rrect, paint);
}

static void drawrr(SkCanvas* canvas, const SkRRect& rrect, InsetProc proc) {
    SkRRect rr;
    for (SkScalar d = -30; d <= 30; d += 5) {
        proc(rrect, d, d, &rr);
        draw_rrect_color(canvas, rr);
    }
}

class RRectGM : public skiagm::GM {
public:
    RRectGM() {}

protected:

    SkString onShortName() override {
        return SkString("rrect");
    }

    SkISize onISize() override {
        return SkISize::Make(820, 710);
    }

    void onDraw(SkCanvas* canvas) override {
        constexpr InsetProc insetProcs[] = {
            inset0, inset1, inset2, inset3
        };

        SkRRect rrect[4];
        SkRect r = { 0, 0, 120, 100 };
        SkVector radii[4] = {
            { 0, 0 }, { 30, 1 }, { 10, 40 }, { 40, 40 }
        };

        rrect[0].setRect(r);
        rrect[1].setOval(r);
        rrect[2].setRectXY(r, 20, 20);
        rrect[3].setRectRadii(r, radii);

        canvas->translate(50.5f, 50.5f);
        for (size_t j = 0; j < SK_ARRAY_COUNT(insetProcs); ++j) {
            canvas->save();
            for (size_t i = 0; i < SK_ARRAY_COUNT(rrect); ++i) {
                drawrr(canvas, rrect[i], insetProcs[j]);
                canvas->translate(200, 0);
            }
            canvas->restore();
            canvas->translate(0, 170);
        }
    }

private:
    typedef GM INHERITED;
};

DEF_GM( return new RRectGM; )


DEF_SIMPLE_GM(giant_rrect_clip_fill, canvas, 400, 400) {
    SkRRect giant = SkRRect::MakeOval(SkRect::MakeIWH(1500, 1500));
    canvas->clipRRect(giant, true);
    canvas->clear(SK_ColorBLACK);
}

DEF_SIMPLE_GM(giant_rrect_clip_draw, canvas, 400, 400) {
    SkRRect giant = SkRRect::MakeOval(SkRect::MakeIWH(1500, 1500));
    canvas->clipRRect(giant, true);

    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setAntiAlias(true);
    canvas->drawRRect(giant, paint);
}

DEF_SIMPLE_GM(giant_rrect_draw, canvas, 400, 400) {
    SkRRect giant = SkRRect::MakeOval(SkRect::MakeIWH(1500, 1500));

    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setAntiAlias(true);
    canvas->drawRRect(giant, paint);
}

DEF_SIMPLE_GM(giant_rrect_scale_clip_fill, canvas, 400, 400) {
    SkRRect notGiant = SkRRect::MakeOval(SkRect::MakeIWH(300, 300));
    canvas->scale(5.0f, 5.0f);
    canvas->clipRRect(notGiant, true);
    canvas->clear(SK_ColorBLACK);
}

DEF_SIMPLE_GPU_GM_BG(
        giant_rrect_gpu, context, rtc, canvas, 400, 400, SK_ColorBLACK) {
    // 3 ways to draw a giant rrect, draw additively in R, G, B so should appear as a white rrect
    // and any deviation in AA in the corners shows up as coloration.

    SkRRect giant = SkRRect::MakeOval(SkRect::MakeIWH(1500, 1500));

    // 1. GrOvalOpFactory
    GrPaint p1;
    p1.setColor4f({1.f, 0.f, 0.f, 0.5f});
    p1.setPorterDuffXPFactory(SkBlendMode::kPlus);
    std::unique_ptr<GrDrawOp> op1 = GrOvalOpFactory::MakeRRectOp(context, std::move(p1), SkMatrix::I(), giant, SkStrokeRec(SkStrokeRec::kFill_InitStyle), context->priv().caps()->shaderCaps());
    if (op1) {
        rtc->priv().testingOnly_addDrawOp(std::move(op1));
    }

    // 2. GrFillRRectOp
    GrPaint p2;
    p2.setColor4f({0.f, 1.f, 0.f, 0.5f});
    p2.setPorterDuffXPFactory(SkBlendMode::kPlus);
    std::unique_ptr<GrDrawOp> op2 = GrFillRRectOp::Make(context, std::move(p2), SkMatrix::I(), giant, GrAAType::kCoverage);
    if (op2) {
        rtc->priv().testingOnly_addDrawOp(std::move(op2));
    }
    // 3. GrRRectEffect
    GrPaint p3;
    p3.setColor4f({0.f, 0.f, 1.f, 0.5f});
    p3.setPorterDuffXPFactory(SkBlendMode::kPlus);
    auto [success, fp] = GrRRectEffect::Make(nullptr, GrClipEdgeType::kFillAA, giant, *context->priv().caps()->shaderCaps());
    if (success) {
        p3.addCoverageFragmentProcessor(std::move(fp));
        std::unique_ptr<GrDrawOp> op3 = GrFillRectOp::MakeNonAARect(context, std::move(p3), SkMatrix::I(), giant.getBounds());
        if (op3) {
            rtc->priv().testingOnly_addDrawOp(std::move(op3));
        }
    }
}
