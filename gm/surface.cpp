/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/utils/SkTextUtils.h"
#include "tools/ToolUtils.h"

class GrContext;

#define W 200
#define H 100

static sk_sp<SkShader> make_shader() {
    int a = 0x99;
    int b = 0xBB;
    SkPoint pts[] = { { 0, 0 }, { W, H } };
    SkColor colors[] = { SkColorSetRGB(a, a, a), SkColorSetRGB(b, b, b) };
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);
}

static sk_sp<SkSurface> make_surface(GrContext* ctx, const SkImageInfo& info, SkPixelGeometry geo) {
    SkSurfaceProps props(0, geo);
    if (ctx) {
        return SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info, 0, &props);
    } else {
        return SkSurface::MakeRaster(info, &props);
    }
}

static void test_draw(SkCanvas* canvas, const char label[]) {
    SkPaint paint;

    paint.setAntiAlias(true);
    paint.setDither(true);

    paint.setShader(make_shader());
    canvas->drawRect(SkRect::MakeWH(W, H), paint);
    paint.setShader(nullptr);

    paint.setColor(SK_ColorWHITE);
    SkFont font(ToolUtils::create_portable_typeface(), 32);
    font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
    SkTextUtils::DrawString(canvas, label, W / 2, H * 3 / 4, font, paint,
                            SkTextUtils::kCenter_Align);
}

class SurfacePropsGM : public skiagm::GM {
public:
    SurfacePropsGM() {}

protected:
    SkString onShortName() override {
        return SkString("surfaceprops");
    }

    SkISize onISize() override {
        return SkISize::Make(W, H * 5);
    }

    void onDraw(SkCanvas* canvas) override {
        GrContext* ctx = canvas->getGrContext();

        // must be opaque to have a hope of testing LCD text
        const SkImageInfo info = SkImageInfo::MakeN32(W, H, kOpaque_SkAlphaType);

        const struct {
            SkPixelGeometry fGeo;
            const char*     fLabel;
        } recs[] = {
            { kUnknown_SkPixelGeometry, "Unknown" },
            { kRGB_H_SkPixelGeometry,   "RGB_H" },
            { kBGR_H_SkPixelGeometry,   "BGR_H" },
            { kRGB_V_SkPixelGeometry,   "RGB_V" },
            { kBGR_V_SkPixelGeometry,   "BGR_V" },
        };

        SkScalar x = 0;
        SkScalar y = 0;
        for (const auto& rec : recs) {
            auto surface(make_surface(ctx, info, rec.fGeo));
            if (!surface) {
                SkDebugf("failed to create surface! label: %s", rec.fLabel);
                continue;
            }
            test_draw(surface->getCanvas(), rec.fLabel);
            surface->draw(canvas, x, y, nullptr);
            y += H;
        }
    }

private:
    typedef GM INHERITED;
};
DEF_GM( return new SurfacePropsGM )

#ifdef SK_DEBUG
static bool equal(const SkSurfaceProps& a, const SkSurfaceProps& b) {
    return a.flags() == b.flags() && a.pixelGeometry() == b.pixelGeometry();
}
#endif

class NewSurfaceGM : public skiagm::GM {
public:
    NewSurfaceGM() {}

protected:
    SkString onShortName() override {
        return SkString("surfacenew");
    }

    SkISize onISize() override {
        return SkISize::Make(300, 140);
    }

    static void drawInto(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorRED);
    }

    void onDraw(SkCanvas* canvas) override {
        SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);

        auto surf(ToolUtils::makeSurface(canvas, info, nullptr));
        drawInto(surf->getCanvas());

        sk_sp<SkImage> image(surf->makeImageSnapshot());
        canvas->drawImage(image, 10, 10, nullptr);

        auto surf2(surf->makeSurface(info));
        drawInto(surf2->getCanvas());

        // Assert that the props were communicated transitively through the first image
        SkASSERT(equal(surf->props(), surf2->props()));

        sk_sp<SkImage> image2(surf2->makeImageSnapshot());
        canvas->drawImage(image2.get(), 10 + SkIntToScalar(image->width()) + 10, 10, nullptr);
    }

private:
    typedef GM INHERITED;
};
DEF_GM( return new NewSurfaceGM )

///////////////////////////////////////////////////////////////////////////////////////////////////

DEF_SIMPLE_GM(copy_on_write_retain, canvas, 256, 256) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
    sk_sp<SkSurface>  surf = ToolUtils::makeSurface(canvas, info);

    surf->getCanvas()->clear(SK_ColorRED);
    // its important that image survives longer than the next draw, so the surface will see
    // an outstanding image, and have to decide if it should retain or discard those pixels
    sk_sp<SkImage> image = surf->makeImageSnapshot();

    // normally a clear+opaque should trigger the discard optimization, but since we have a clip
    // it should not (we need the previous red pixels).
    surf->getCanvas()->clipRect(SkRect::MakeWH(128, 256));
    surf->getCanvas()->clear(SK_ColorBLUE);

    // expect to see two rects: blue | red
    canvas->drawImage(surf->makeImageSnapshot(), 0, 0, nullptr);
}

DEF_SIMPLE_GM(copy_on_write_savelayer, canvas, 256, 256) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
    sk_sp<SkSurface>  surf = ToolUtils::makeSurface(canvas, info);
    surf->getCanvas()->clear(SK_ColorRED);
    // its important that image survives longer than the next draw, so the surface will see
    // an outstanding image, and have to decide if it should retain or discard those pixels
    sk_sp<SkImage> image = surf->makeImageSnapshot();

    // now draw into a full-screen layer. This should (a) trigger a copy-on-write, but it should
    // not trigger discard, even tho its alpha (SK_ColorBLUE) is opaque, since it is in a layer
    // with a non-opaque paint.
    SkPaint paint;
    paint.setAlphaf(0.25f);
    surf->getCanvas()->saveLayer({0, 0, 256, 256}, &paint);
    surf->getCanvas()->clear(SK_ColorBLUE);
    surf->getCanvas()->restore();

    // expect to see two rects: blue blended on red
    canvas->drawImage(surf->makeImageSnapshot(), 0, 0, nullptr);
}

DEF_SIMPLE_GM(surface_underdraw, canvas, 256, 256) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256, nullptr);
    auto        surf = ToolUtils::makeSurface(canvas, info);

    const SkIRect subset = SkIRect::MakeLTRB(180, 0, 256, 256);

    // noisy background
    {
        SkPoint pts[] = {{0, 0}, {40, 50}};
        SkColor colors[] = {SK_ColorRED, SK_ColorBLUE};
        auto sh = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kRepeat);
        SkPaint paint;
        paint.setShader(sh);
        surf->getCanvas()->drawPaint(paint);
    }

    // save away the right-hand strip, then clear it
    sk_sp<SkImage> saveImg = surf->makeImageSnapshot(subset);
    {
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kClear);
        surf->getCanvas()->drawRect(SkRect::Make(subset), paint);
    }

    // draw the "foreground"
    {
        SkPaint paint;
        paint.setColor(SK_ColorGREEN);
        SkRect r = { 0, 10, 256, 35 };
        while (r.fBottom < 256) {
            surf->getCanvas()->drawRect(r, paint);
            r.offset(0, r.height() * 2);
        }
    }

    // apply the "fade"
    {
        SkPoint pts[] = {{SkIntToScalar(subset.left()), 0}, {SkIntToScalar(subset.right()), 0}};
        SkColor colors[] = {0xFF000000, 0};
        auto sh = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);
        SkPaint paint;
        paint.setShader(sh);
        paint.setBlendMode(SkBlendMode::kDstIn);
        surf->getCanvas()->drawRect(SkRect::Make(subset), paint);
    }

    // restore the original strip, drawing it "under" the current foreground
    {
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kDstOver);
        surf->getCanvas()->drawImage(saveImg,
                                     SkIntToScalar(subset.left()), SkIntToScalar(subset.top()),
                                     &paint);
    }

    // show it on screen
   surf->draw(canvas, 0, 0, nullptr);
}
