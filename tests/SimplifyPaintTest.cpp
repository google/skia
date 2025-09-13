/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRRect.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "include/private/base/SkTemplates.h"

#if defined(SK_GANESH)
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#endif

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#endif

namespace {

constexpr int kSurfaceSize = 32;

void draw_paint(SkCanvas* canvas, const SkPaint& paint) {
    canvas->drawPaint(paint);
}

void draw_rect(SkCanvas* canvas, const SkPaint& paint) {
    canvas->drawRect(SkRect::MakeWH(kSurfaceSize, kSurfaceSize), paint);
}

void draw_rrect(SkCanvas* canvas, const SkPaint& paint) {
    canvas->drawRRect(SkRRect::MakeRectXY({ 0, 0, kSurfaceSize, kSurfaceSize }, 4, 4), paint);
}

void draw_arc(SkCanvas* canvas, const SkPaint& paint) {
    SkArc arc = SkArc::Make({ -kSurfaceSize, -kSurfaceSize, kSurfaceSize, kSurfaceSize},
                            0, 270, SkArc::Type::kWedge);

    canvas->drawArc(arc, paint);
}

SkPath create_dented_rect() {
    SkPathBuilder b;
    b.moveTo(0, 0);
    b.lineTo(kSurfaceSize/2, 1);
    b.lineTo(kSurfaceSize, 0);
    b.lineTo(kSurfaceSize, kSurfaceSize);
    b.lineTo(0, kSurfaceSize);
    b.close();
    return b.detach();
}

void draw_path(SkCanvas* canvas, const SkPaint& paint) {
    SkPath p = create_dented_rect();

    canvas->drawPath(p, paint);
}

typedef void (*PFDrawMth)(SkCanvas*, const SkPaint&);
static constexpr PFDrawMth kDrawMethods[] = {
        draw_paint,
        draw_rect,
        draw_rrect,
        draw_arc,
        draw_path
};

// Create an SkColorSpace that removes the specified color channel with the transfer
// function of the provided colorSpace
sk_sp<SkColorSpace> make_knockout(SkColorSpace* cs, int index) {
    skcms_Matrix3x3 knockoutMat = {{
                                    { 1, 0, 0 },
                                    { 0, 1, 0 },
                                    { 0, 0, 1 },
                            }};

    knockoutMat.vals[index][0] = knockoutMat.vals[index][1] = knockoutMat.vals[index][2] = 0;

    skcms_Matrix3x3 tmpXYZD50;
    cs->toXYZD50(&tmpXYZD50);
    skcms_TransferFunction tmpTransferFn;
    cs->transferFn(&tmpTransferFn);

    skcms_Matrix3x3 knockedOut = skcms_Matrix3x3_concat(&tmpXYZD50, &knockoutMat);

    return SkColorSpace::MakeRGB(tmpTransferFn, knockedOut);
}

SkImageInfo get_surface_ii() {
    sk_sp<SkColorSpace> spinCS = SkColorSpace::MakeSRGB()->makeColorSpin();

    SkImageInfo ii = SkImageInfo::Make(SkISize::Make(kSurfaceSize, kSurfaceSize),
                                       kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType,
                                       std::move(spinCS));
    return ii;
}

bool almost_equals(SkColor a, SkColor b, int tolerance) {
    if (SkTAbs((int)SkColorGetR(a) - (int)SkColorGetR(b)) > tolerance) { return false; }
    if (SkTAbs((int)SkColorGetG(a) - (int)SkColorGetG(b)) > tolerance) { return false; }
    if (SkTAbs((int)SkColorGetB(a) - (int)SkColorGetB(b)) > tolerance) { return false; }
    if (SkTAbs((int)SkColorGetA(a) - (int)SkColorGetA(b)) > tolerance) { return false; }
    return true;
}

void run_test(SkSurface* surface, skiatest::Reporter* reporter) {
    SkImageInfo ii = SkImageInfo::Make(SkISize::Make(kSurfaceSize, kSurfaceSize),
                                       kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType);
    SkBitmap bitmap;
    SkPixmap pixmap;
    bitmap.allocPixels(ii);
    SkAssertResult(bitmap.peekPixels(&pixmap));

    sk_sp<SkColorSpace> srgbCS = SkColorSpace::MakeSRGB();
    sk_sp<SkColorSpace> knockoutG = make_knockout(srgbCS.get(), 1);
    sk_sp<SkColorSpace> knockoutB = make_knockout(srgbCS.get(), 2);

    // The same as the color spin ColorSpace: R->B, G->R, B->R
    static constexpr float kSpinMatrix[] = { 0, 1, 0, 0, 0,
                                             0, 0, 1, 0, 0,
                                             1, 0, 0, 0, 0,
                                             0, 0, 0, 1, 0  };
    sk_sp<SkColorFilter> spinMatrixCF = SkColorFilters::Matrix(kSpinMatrix);

    constexpr SkColor4f kTransWhite = { 1, 1, 1, 0.5f };

    constexpr SkColor4f kCyan100 = { 0, 1, 1, 1.0f };
    constexpr SkColor4f kCyan50  = { 0, 1, 1, 0.5f };
    constexpr SkColor4f kCyan25  = { 0, 1, 1, 0.25f };

    constexpr SkColor4f kYellow100 = { 1, 1, 0, 1.0f };
    constexpr SkColor4f kYellow50  = { 1, 1, 0, 0.5f };
    constexpr SkColor4f kYellow25  = { 1, 1, 0, 0.25f };

    // These will be used to create an SkPaint w/ the specified paint color, solid color shader,
    // and color filter.
    const struct {
        SkColor4f fPaint;
        SkColor4f fShader;
        sk_sp<SkColorFilter> fColorFilter;
        SkColor fExpected;
    } kCases[] = {
            { SkColors::kWhite, SkColors::kWhite, nullptr,      kCyan100.toSkColor() },
            { kTransWhite,      SkColors::kWhite, nullptr,      kCyan50.toSkColor() },
            { SkColors::kWhite, kTransWhite,      nullptr,      kCyan50.toSkColor() },
            { kTransWhite,      kTransWhite,      nullptr,      kCyan25.toSkColor() },

            { SkColors::kWhite, SkColors::kWhite, spinMatrixCF, kYellow100.toSkColor() },
            { kTransWhite,      SkColors::kWhite, spinMatrixCF, kYellow50.toSkColor() },
            { SkColors::kWhite, kTransWhite,      spinMatrixCF, kYellow50.toSkColor() },
            { kTransWhite,      kTransWhite,      spinMatrixCF, kYellow25.toSkColor() },
    };

    for (const auto& c : kCases) {
        SkPaint paint;
        paint.setColor(c.fPaint, knockoutB.get());               // yellow
        paint.setShader(SkShaders::Color(c.fShader, knockoutG)); // magenta
        paint.setColorFilter(c.fColorFilter);                    // magenta -> cyan, if !nullptr
        paint.setBlendMode(SkBlendMode::kSrc);

        for (auto draw : kDrawMethods) {
            surface->getCanvas()->clear(SK_ColorBLACK);

            // The spinCS on the surface spins:
            //    cyan    -> yellow
            //    magenta -> cyan
            //    yellow  -> magenta
            draw(surface->getCanvas(), paint);

            if (!surface->readPixels(pixmap, 0, 0)) {
                ERRORF(reporter, "readPixels failed");
                return;
            }

            SkColor actual = pixmap.getColor(kSurfaceSize/2, kSurfaceSize/2);
            REPORTER_ASSERT(reporter,
                            almost_equals(actual, c.fExpected, /* tolerance= */ 1),
                            "Wrong color, expected %08x, found %08x", c.fExpected, actual);
        }
    }
}

} // anonymous namespace

#if defined(SK_GRAPHITE)
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SimplifyPaintTest_Graphite,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNextRelease) {
    using namespace skgpu::graphite;

    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(),
                                                        get_surface_ii());

    run_test(surface.get(), reporter);
}
#endif

#if defined(SK_GANESH)
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SimplifyPaintTest_Ganesh,
                                       reporter,
                                       contextInfo,
                                       CtsEnforcement::kNextRelease) {
    GrDirectContext* context = contextInfo.directContext();
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(context,
                                                        skgpu::Budgeted::kYes,
                                                        get_surface_ii());

    run_test(surface.get(), reporter);
}
#endif

DEF_TEST(SimplifyPaintTest_Raster, reporter) {
    sk_sp<SkSurface> surface = SkSurfaces::Raster(get_surface_ii());

    run_test(surface.get(), reporter);
}
