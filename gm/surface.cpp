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
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Surface.h"
#endif
#include "include/utils/SkTextUtils.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/gpu/BackendSurfaceFactory.h"

#define W 800
#define H 100

static sk_sp<SkShader> make_shader() {
    int a = 0x99;
    int b = 0xBB;
    SkPoint pts[] = { { 0, 0 }, { W, H } };
    SkColor colors[] = { SkColorSetRGB(a, a, a), SkColorSetRGB(b, b, b) };
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);
}

static sk_sp<SkSurface> make_surface(GrRecordingContext* ctx,
                                     skgpu::graphite::Recorder* recorder,
                                     const SkImageInfo& info,
                                     uint32_t flags,
                                     SkPixelGeometry geo,
                                     SkScalar contrast,
                                     SkScalar gamma) {
    SkSurfaceProps props(flags, geo, contrast, gamma);
#if defined(SK_GRAPHITE)
    if (recorder) {
            return SkSurfaces::RenderTarget(recorder, info, skgpu::Mipmapped::kNo, &props);
    } else
#endif
    if (ctx) {
        return SkSurfaces::RenderTarget(ctx, skgpu::Budgeted::kNo, info, 0, &props);
    } else {
        return SkSurfaces::Raster(info, &props);
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
    SkFont font(ToolUtils::DefaultPortableTypeface(), 32);
    font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
    SkTextUtils::DrawString(canvas, label, W / 2, H * 3 / 4, font, paint,
                            SkTextUtils::kCenter_Align);
}

class SurfacePropsGM : public skiagm::GM {
public:
    SurfacePropsGM(uint32_t flags) : fFlags(flags) {
        recs = {
                {kUnknown_SkPixelGeometry,
                 "Unknown geometry, default contrast/gamma",
                 SK_GAMMA_CONTRAST,
                 SK_GAMMA_EXPONENT},
                {kRGB_H_SkPixelGeometry,
                 "RGB_H, default contrast/gamma",
                 SK_GAMMA_CONTRAST,
                 SK_GAMMA_EXPONENT},
                {kBGR_H_SkPixelGeometry,
                 "BGR_H, default contrast/gamma",
                 SK_GAMMA_CONTRAST,
                 SK_GAMMA_EXPONENT},
                {kRGB_V_SkPixelGeometry,
                 "RGB_V, default contrast/gamma",
                 SK_GAMMA_CONTRAST,
                 SK_GAMMA_EXPONENT},
                {kBGR_V_SkPixelGeometry,
                 "BGR_V, default contrast/gamma",
                 SK_GAMMA_CONTRAST,
                 SK_GAMMA_EXPONENT},
                {kRGB_H_SkPixelGeometry, "RGB_H contrast : 0 gamma: 0", 0, 0},
                {kRGB_H_SkPixelGeometry, "RGB_H contrast : 1 gamma: 0", 1, 0},
                {kRGB_H_SkPixelGeometry, "RGB_H contrast : 0 gamma: 3.9", 0, 3.9f},
                {kRGB_H_SkPixelGeometry, "RGB_H contrast : 1 gamma: 3.9", 1, 3.9f},
        };
    }

protected:
    SkString getName() const override {
        return SkStringPrintf("surfaceprops%s",
                              fFlags != 0 ? "_df" : "");
    }

    SkISize getISize() override { return SkISize::Make(W, H * recs.size()); }

    void onDraw(SkCanvas* canvas) override {
        auto ctx = canvas->recordingContext();
        auto recorder = canvas->recorder();

        // must be opaque to have a hope of testing LCD text
        const SkImageInfo info = SkImageInfo::MakeN32(W, H, kOpaque_SkAlphaType);

        SkScalar x = 0;
        SkScalar y = 0;
        for (const auto& rec : recs) {
            auto surface(make_surface(ctx, recorder, info, fFlags, rec.fGeo, rec.fContrast,
                                      rec.fGamma));
            if (!surface) {
                SkDebugf("failed to create surface! label: %s", rec.fLabel);
                continue;
            }
            test_draw(surface->getCanvas(), rec.fLabel);
            surface->draw(canvas, x, y);
            y += H;
        }
    }

private:
    struct SurfacePropsInput {
        SkPixelGeometry fGeo;
        const char*     fLabel;
        SkScalar fContrast;
        SkScalar fGamma;
    };
    std::vector<SurfacePropsInput> recs;

    uint32_t fFlags;

    using INHERITED = GM;
};
DEF_GM( return new SurfacePropsGM(0); )
DEF_GM( return new SurfacePropsGM(SkSurfaceProps::kUseDeviceIndependentFonts_Flag); )

#ifdef SK_DEBUG
static bool equal(const SkSurfaceProps& a, const SkSurfaceProps& b) {
    return a == b;
}
#endif

class NewSurfaceGM : public skiagm::GM {
public:
    NewSurfaceGM() {}

protected:
    SkString getName() const override { return SkString("surfacenew"); }

    SkISize getISize() override { return SkISize::Make(300, 140); }

    static void drawInto(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorRED);
    }

    void onDraw(SkCanvas* canvas) override {
        SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);

        auto surf(ToolUtils::makeSurface(canvas, info, nullptr));
        drawInto(surf->getCanvas());

        sk_sp<SkImage> image(surf->makeImageSnapshot());
        canvas->drawImage(image, 10, 10);

        auto surf2(surf->makeSurface(info));
        drawInto(surf2->getCanvas());

        // Assert that the props were communicated transitively through the first image
        SkASSERT(equal(surf->props(), surf2->props()));

        sk_sp<SkImage> image2(surf2->makeImageSnapshot());
        canvas->drawImage(image2.get(), 10 + SkIntToScalar(image->width()) + 10, 10);
    }

private:
    using INHERITED = GM;
};
DEF_GM( return new NewSurfaceGM )

///////////////////////////////////////////////////////////////////////////////////////////////////

// The GPU backend may behave differently when images are snapped from wrapped textures and
// render targets compared.
namespace {
enum SurfaceType {
    kManaged,
    kBackendTexture,
    kBackendRenderTarget
};
}

static sk_sp<SkSurface> make_surface(const SkImageInfo& ii, SkCanvas* canvas, SurfaceType type) {
    GrDirectContext* direct = GrAsDirectContext(canvas->recordingContext());
    switch (type) {
        case kManaged:
            return ToolUtils::makeSurface(canvas, ii);
        case kBackendTexture:
            if (!direct) {
                return nullptr;
            }
            return sk_gpu_test::MakeBackendTextureSurface(direct, ii, kTopLeft_GrSurfaceOrigin, 1);
        case kBackendRenderTarget:
            return sk_gpu_test::MakeBackendRenderTargetSurface(direct,
                                                               ii,
                                                               kTopLeft_GrSurfaceOrigin,
                                                               1);
    }
    return nullptr;
}

using MakeSurfaceFn = std::function<sk_sp<SkSurface>(const SkImageInfo&)>;

#define DEF_BASIC_SURFACE_TEST(name, canvas, main, W, H)            \
    DEF_SIMPLE_GM(name, canvas, W, H) {                             \
        auto make = [canvas](const SkImageInfo& ii) {               \
            return make_surface(ii, canvas, SurfaceType::kManaged); \
        };                                                          \
        main(canvas, MakeSurfaceFn(make));                          \
    }

#define DEF_BACKEND_SURFACE_TEST(name, canvas, main, type, W, H)                                \
    DEF_SIMPLE_GM_CAN_FAIL(name, canvas, err_msg, W, H) {                                       \
        GrDirectContext* direct = GrAsDirectContext(canvas->recordingContext());                \
        if (!direct || direct->abandoned()) {                                                   \
            *err_msg = "Requires non-abandoned GrDirectContext";                                \
            return skiagm::DrawResult::kSkip;                                                   \
        }                                                                                       \
        auto make = [canvas](const SkImageInfo& ii) { return make_surface(ii, canvas, type); }; \
        main(canvas, MakeSurfaceFn(make));                                                      \
        return skiagm::DrawResult::kOk;                                                         \
    }

#define DEF_BET_SURFACE_TEST(name, canvas, main, W, H)                  \
    DEF_BACKEND_SURFACE_TEST(SK_MACRO_CONCAT(name, _bet), canvas, main, \
                             SurfaceType::kBackendTexture, W, H)

#define DEF_BERT_SURFACE_TEST(name, canvas, main, W, H)                  \
    DEF_BACKEND_SURFACE_TEST(SK_MACRO_CONCAT(name, _bert), canvas, main, \
                             SurfaceType::kBackendRenderTarget, W, H)

// This makes 3 GMs from the same code, normal, wrapped backend texture, and wrapped backend
// render target.
#define DEF_SURFACE_TESTS(name, canvas, W, H)                                  \
    static void SK_MACRO_CONCAT(name, _main)(SkCanvas*, const MakeSurfaceFn&); \
    DEF_BASIC_SURFACE_TEST(name, canvas, SK_MACRO_CONCAT(name, _main), W, H)   \
    DEF_BET_SURFACE_TEST  (name, canvas, SK_MACRO_CONCAT(name, _main), W, H)   \
    DEF_BERT_SURFACE_TEST (name, canvas, SK_MACRO_CONCAT(name, _main), W, H)   \
    static void SK_MACRO_CONCAT(name, _main)(SkCanvas * canvas, const MakeSurfaceFn& make)

DEF_SURFACE_TESTS(copy_on_write_retain, canvas, 256, 256) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
    sk_sp<SkSurface> surf = make(info);

    surf->getCanvas()->clear(SK_ColorRED);
    // its important that image survives longer than the next draw, so the surface will see
    // an outstanding image, and have to decide if it should retain or discard those pixels
    sk_sp<SkImage> image = surf->makeImageSnapshot();

    // normally a clear+opaque should trigger the discard optimization, but since we have a clip
    // it should not (we need the previous red pixels).
    surf->getCanvas()->clipRect(SkRect::MakeWH(128, 256));
    surf->getCanvas()->clear(SK_ColorBLUE);

    // expect to see two rects: blue | red
    canvas->drawImage(surf->makeImageSnapshot(), 0, 0);
}

// Like copy_on_write_retain but draws the snapped image back to the surface it was snapped from.
DEF_SURFACE_TESTS(copy_on_write_retain2, canvas, 256, 256) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
    sk_sp<SkSurface> surf = make(info);

    surf->getCanvas()->clear(SK_ColorBLUE);
    // its important that image survives longer than the next draw, so the surface will see
    // an outstanding image, and have to decide if it should retain or discard those pixels
    sk_sp<SkImage> image = surf->makeImageSnapshot();

    surf->getCanvas()->clear(SK_ColorRED);
    // normally a clear+opaque should trigger the discard optimization, but since we have a clip
    // it should not (we need the previous red pixels).
    surf->getCanvas()->clipRect(SkRect::MakeWH(128, 256));
    surf->getCanvas()->drawImage(image, 0, 0);

    // expect to see two rects: blue | red
    canvas->drawImage(surf->makeImageSnapshot(), 0, 0);
}

DEF_SURFACE_TESTS(simple_snap_image, canvas, 256, 256) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
    sk_sp<SkSurface> surf = make(info);

    surf->getCanvas()->clear(SK_ColorRED);
    sk_sp<SkImage> image = surf->makeImageSnapshot();
    // expect to see just red
    canvas->drawImage(std::move(image), 0, 0);
}

// Like simple_snap_image but the surface dies before the image.
DEF_SURFACE_TESTS(simple_snap_image2, canvas, 256, 256) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
    sk_sp<SkSurface> surf = make(info);

    surf->getCanvas()->clear(SK_ColorRED);
    sk_sp<SkImage> image = surf->makeImageSnapshot();
    surf.reset();
    // expect to see just red
    canvas->drawImage(std::move(image), 0, 0);
}

DEF_SIMPLE_GM(snap_with_mips, canvas, 80, 75) {
    auto ct = canvas->imageInfo().colorType() == kUnknown_SkColorType
                      ? kRGBA_8888_SkColorType
                      : canvas->imageInfo().colorType();
    auto ii = SkImageInfo::Make({32, 32},
                                ct,
                                kPremul_SkAlphaType,
                                canvas->imageInfo().refColorSpace());
    auto surface = SkSurfaces::Raster(ii);

    auto nextImage = [&](SkColor color) {
        surface->getCanvas()->clear(color);
        SkPaint paint;
        paint.setColor(~color | 0xFF000000);
        surface->getCanvas()->drawRect(SkRect::MakeLTRB(surface->width() *2/5.f,
                                                        surface->height()*2/5.f,
                                                        surface->width() *3/5.f,
                                                        surface->height()*3/5.f),
                                    paint);
        return surface->makeImageSnapshot()->withDefaultMipmaps();
    };

    static constexpr int kPad = 8;
    static const SkSamplingOptions kSampling{SkFilterMode::kLinear, SkMipmapMode::kLinear};

    canvas->save();
    for (int y = 0; y < 3; ++y) {
        canvas->save();
        SkColor kColors[] = {0xFFF0F0F0, SK_ColorBLUE};
        for (int x = 0; x < 2; ++x) {
            auto image = nextImage(kColors[x]);
            canvas->drawImage(image, 0, 0, kSampling);
            canvas->translate(ii.width() + kPad, 0);
        }
        canvas->restore();
        canvas->translate(0, ii.width() + kPad);
        canvas->scale(.4f, .4f);
    }
    canvas->restore();
}

DEF_SURFACE_TESTS(copy_on_write_savelayer, canvas, 256, 256) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
    sk_sp<SkSurface> surf = make(info);
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
    canvas->drawImage(surf->makeImageSnapshot(), 0, 0);
}

DEF_SURFACE_TESTS(surface_underdraw, canvas, 256, 256) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256, nullptr);
    auto surf = make(info);

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
                                     SkSamplingOptions(), &paint);
    }

    // show it on screen
   surf->draw(canvas, 0, 0);
}
