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
    return SkGradientShader::CreateLinear(pts, colors, NULL, 2, SkShader::kClamp_TileMode);
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
        return SkSurface::NewRenderTarget(ctx, info, 0, &props);
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
    paint.setShader(NULL);

    paint.setColor(SK_ColorWHITE);
    paint.setTextSize(32);
    paint.setTextAlign(SkPaint::kCenter_Align);
    canvas->drawText(label, strlen(label), W / 2, H * 3 / 4, paint);
}

class SurfacePropsGM : public skiagm::GM {
public:
    SurfacePropsGM() {}

protected:
    SkString onShortName() SK_OVERRIDE {
        return SkString("surfaceprops");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(W * 4, H * 5);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
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
                    surface->draw(canvas, x, y, NULL);
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
