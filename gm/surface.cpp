/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkGradientShader.h"
#include "SkSurface.h"
#include "SkSurfaceProps.h"

#define W 200
#define H 100

static SkShader* make_shader() {
    int a = 0x99;
    int b = 0xBB;
    SkPoint pts[] = { { 0, 0 }, { W, H } };
    SkColor colors[] = { SkColorSetRGB(a, a, a), SkColorSetRGB(b, b, b) };
    return SkGradientShader::CreateLinear(pts, colors, nullptr, 2, SkShader::kClamp_TileMode);
}

static SkSurface* make_surface(GrContext* ctx, const SkImageInfo& info, SkPixelGeometry geo,
                               int disallowAA, int disallowDither) {
    uint32_t flags = 0;
    if (disallowAA) {
        flags |= SkSurfaceProps::kDisallowAntiAlias_Flag;
    }
    if (disallowDither) {
        flags |= SkSurfaceProps::kDisallowDither_Flag;
    }

    SkSurfaceProps props(flags, geo);
    if (ctx) {
        return SkSurface::NewRenderTarget(ctx, SkSurface::kNo_Budgeted, info, 0, &props);
    } else {
        return SkSurface::NewRaster(info, &props);
    }
}

static void test_draw(SkCanvas* canvas, const char label[]) {
    SkPaint paint;

    paint.setAntiAlias(true);
    paint.setLCDRenderText(true);
    paint.setDither(true);

    paint.setShader(make_shader())->unref();
    canvas->drawRect(SkRect::MakeWH(W, H), paint);
    paint.setShader(nullptr);

    paint.setColor(SK_ColorWHITE);
    paint.setTextSize(32);
    paint.setTextAlign(SkPaint::kCenter_Align);
    sk_tool_utils::set_portable_typeface(&paint);
    canvas->drawText(label, strlen(label), W / 2, H * 3 / 4, paint);
}

class SurfacePropsGM : public skiagm::GM {
public:
    SurfacePropsGM() {}

protected:
    SkString onShortName() override {
        return SkString("surfaceprops");
    }

    SkISize onISize() override {
        return SkISize::Make(W * 4, H * 5);
    }

    void onDraw(SkCanvas* canvas) override {
        GrContext* ctx = canvas->getGrContext();

        // must be opaque to have a hope of testing LCD text
        const SkImageInfo info = SkImageInfo::MakeN32(W, H, kOpaque_SkAlphaType);

        const struct {
            SkPixelGeometry fGeo;
            const char*     fLabel;
        } rec[] = {
            { kUnknown_SkPixelGeometry, "Unknown" },
            { kRGB_H_SkPixelGeometry,   "RGB_H" },
            { kBGR_H_SkPixelGeometry,   "BGR_H" },
            { kRGB_V_SkPixelGeometry,   "RGB_V" },
            { kBGR_V_SkPixelGeometry,   "BGR_V" },
        };
    
        SkScalar x = 0;
        for (int disallowAA = 0; disallowAA <= 1; ++disallowAA) {
            for (int disallowDither = 0; disallowDither <= 1; ++disallowDither) {
                SkScalar y = 0;
                for (size_t i = 0; i < SK_ARRAY_COUNT(rec); ++i) {
                    SkAutoTUnref<SkSurface> surface(make_surface(ctx, info, rec[i].fGeo,
                                                                 disallowAA, disallowDither));
                    test_draw(surface->getCanvas(), rec[i].fLabel);
                    surface->draw(canvas, x, y, nullptr);
                    y += H;
                }
                x += W;
            }
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

        SkAutoTUnref<SkSurface> surf(canvas->newSurface(info, nullptr));
        if (!surf.get()) {
            surf.reset(SkSurface::NewRaster(info));
        }
        drawInto(surf->getCanvas());

        SkAutoTUnref<SkImage> image(surf->newImageSnapshot());
        canvas->drawImage(image, 10, 10, nullptr);

        SkAutoTUnref<SkSurface> surf2(surf->newSurface(info));
        drawInto(surf2->getCanvas());

        // Assert that the props were communicated transitively through the first image
        SkASSERT(equal(surf->props(), surf2->props()));

        SkAutoTUnref<SkImage> image2(surf2->newImageSnapshot());
        canvas->drawImage(image2, 10 + SkIntToScalar(image->width()) + 10, 10, nullptr);
    }

private:
    typedef GM INHERITED;
};
DEF_GM( return new NewSurfaceGM )

