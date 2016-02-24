/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColor.h"
#include "SkColorMatrixFilter.h"
#include "SkGradientShader.h"
#include "SkImage.h"
#include "SkPM4f.h"
#include "SkShader.h"

#include "Test.h"
#include "SkRandom.h"

const float kTolerance = 1.0f / (1 << 20);

static bool nearly_equal(float a, float b, float tol = kTolerance) {
    SkASSERT(tol >= 0);
    return fabsf(a - b) <= tol;
}

static bool nearly_equal(const SkPM4f a, const SkPM4f& b, float tol = kTolerance) {
    for (int i = 0; i < 4; ++i) {
        if (!nearly_equal(a.fVec[i], b.fVec[i], tol)) {
            return false;
        }
    }
    return true;
}

DEF_TEST(SkColor4f_FromColor, reporter) {
    const struct {
        SkColor     fC;
        SkColor4f   fC4;
    } recs[] = {
        { SK_ColorBLACK, { 1, 0, 0, 0 } },
        { SK_ColorWHITE, { 1, 1, 1, 1 } },
        { SK_ColorRED,   { 1, 1, 0, 0 } },
        { SK_ColorGREEN, { 1, 0, 1, 0 } },
        { SK_ColorBLUE,  { 1, 0, 0, 1 } },
        { 0,             { 0, 0, 0, 0 } },
        { 0x55AAFF00,    { 1/3.0f, 2/3.0f, 1, 0 } },
    };

    for (const auto& r : recs) {
        SkColor4f c4 = SkColor4f::FromColor(r.fC);
        REPORTER_ASSERT(reporter, c4 == r.fC4);
    }
}

DEF_TEST(Color4f_premul, reporter) {
    SkRandom rand;

    for (int i = 0; i < 1000000; ++i) {
        // First just test opaque colors, so that the premul should be exact
        SkColor4f c4 {
            1, rand.nextUScalar1(), rand.nextUScalar1(), rand.nextUScalar1()
        };
        SkPM4f pm4 = c4.premul();
        REPORTER_ASSERT(reporter, pm4.fVec[SK_A_INDEX] == c4.fA);
        REPORTER_ASSERT(reporter, pm4.fVec[SK_R_INDEX] == c4.fA * c4.fR);
        REPORTER_ASSERT(reporter, pm4.fVec[SK_G_INDEX] == c4.fA * c4.fG);
        REPORTER_ASSERT(reporter, pm4.fVec[SK_B_INDEX] == c4.fA * c4.fB);

        // We compare with a tolerance, in case our premul multiply is implemented at slightly
        // different precision than the test code.
        c4.fA = rand.nextUScalar1();
        pm4 = c4.premul();
        REPORTER_ASSERT(reporter, pm4.fVec[SK_A_INDEX] == c4.fA);
        REPORTER_ASSERT(reporter, nearly_equal(pm4.fVec[SK_R_INDEX], c4.fA * c4.fR));
        REPORTER_ASSERT(reporter, nearly_equal(pm4.fVec[SK_G_INDEX], c4.fA * c4.fG));
        REPORTER_ASSERT(reporter, nearly_equal(pm4.fVec[SK_B_INDEX], c4.fA * c4.fB));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////

static SkColorFilter* make_mode_cf() {
    return SkColorFilter::CreateModeFilter(0xFFBB8855, SkXfermode::kPlus_Mode);
}

static SkColorFilter* make_mx_cf() {
    const float mx[] = {
        0.5f, 0,    0, 0, 0.1f,
        0,    0.5f, 0, 0, 0.2f,
        0,    0,    1, 0, -0.1f,
        0,    0,    0, 1, 0,
    };
    return SkColorMatrixFilter::Create(mx);
}

static SkColorFilter* make_compose_cf() {
    SkAutoTUnref<SkColorFilter> cf0(make_mode_cf());
    SkAutoTUnref<SkColorFilter> cf1(make_mx_cf());
    return SkColorFilter::CreateComposeFilter(cf0, cf1);
}

static SkShader* make_color_sh() { return SkShader::CreateColorShader(0xFFBB8855); }

static SkShader* make_image_sh() {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(2, 2);
    const SkPMColor pixels[] {
        SkPackARGB32(0xFF, 0xBB, 0x88, 0x55),
        SkPackARGB32(0xFF, 0xBB, 0x88, 0x55),
        SkPackARGB32(0xFF, 0xBB, 0x88, 0x55),
        SkPackARGB32(0xFF, 0xBB, 0x88, 0x55),
    };
    SkAutoTUnref<SkImage> image(SkImage::NewRasterCopy(info, pixels, sizeof(SkPMColor) * 2));
    return image->newShader(SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);
}

static SkShader* make_grad_sh() {
    const SkPoint pts[] {{ 0, 0 }, { 100, 100 }};
    const SkColor colors[] { SK_ColorRED, SK_ColorBLUE };
    return SkGradientShader::CreateLinear(pts, colors, nullptr, 2, SkShader::kClamp_TileMode);
}

static SkShader* make_cf_sh() {
    SkAutoTUnref<SkColorFilter> filter(make_mx_cf());
    SkAutoTUnref<SkShader> shader(make_color_sh());
    return shader->newWithColorFilter(filter);
}

static void compare_spans(const SkPM4f span4f[], const SkPMColor span4b[], int count,
                          skiatest::Reporter* reporter, float tolerance = 1.0f/255) {
    for (int i = 0; i < count; ++i) {
        SkPM4f c0 = SkPM4f::FromPMColor(span4b[i]);
        SkPM4f c1 = span4f[i];
        REPORTER_ASSERT(reporter, nearly_equal(c0, c1, tolerance));
    }
}

DEF_TEST(Color4f_shader, reporter) {
    struct {
        SkShader* (*fFact)();
        bool      fSupports4f;
        float     fTolerance;
    } recs[] = {
        { make_color_sh, true,  1.0f/255   },
        // PMColor 4f gradients are interpolated in 255-multiplied values, so we need a
        // slightly relaxed tolerance to accommodate the cumulative precision deviation.
        { make_grad_sh,  true,  1.001f/255 },
        { make_image_sh, false, 1.0f/255   },
        { make_cf_sh,    true,  1.0f/255   },
    };

    SkPaint paint;
    for (const auto& rec : recs) {
        uint32_t storage[200];
        paint.setShader(rec.fFact())->unref();
        // Encourage 4f context selection. At some point we may need
        // to instantiate two separate contexts for optimal 4b/4f selection.
        const SkShader::ContextRec contextRec(paint, SkMatrix::I(), nullptr,
                                              SkShader::ContextRec::kPM4f_DstType);
        SkASSERT(paint.getShader()->contextSize(contextRec) <= sizeof(storage));
        SkShader::Context* ctx = paint.getShader()->createContext(contextRec, storage);
        if (rec.fSupports4f) {
            const int N = 100;
            SkPM4f buffer4f[N];
            ctx->shadeSpan4f(0, 0, buffer4f, N);
            SkPMColor buffer4b[N];
            ctx->shadeSpan(0, 0, buffer4b, N);
            compare_spans(buffer4f, buffer4b, N, reporter, rec.fTolerance);
        }
        ctx->~Context();
    }
}

DEF_TEST(Color4f_colorfilter, reporter) {
    struct {
        SkColorFilter* (*fFact)();
        bool           fSupports4f;
    } recs[] = {
        { make_mode_cf,     true },
        { make_mx_cf,       true },
        { make_compose_cf,  true },
    };

    // prepare the src
    const int N = 100;
    SkPMColor src4b[N];
    SkPM4f    src4f[N];
    SkRandom rand;
    for (int i = 0; i < N; ++i) {
        src4b[i] = SkPreMultiplyColor(rand.nextU());
        src4f[i] = SkPM4f::FromPMColor(src4b[i]);
    }
    // confirm that our srcs are (nearly) equal
    compare_spans(src4f, src4b, N, reporter);

    for (const auto& rec : recs) {
        SkAutoTUnref<SkColorFilter> filter(rec.fFact());
        SkPMColor dst4b[N];
        filter->filterSpan(src4b, N, dst4b);
        SkPM4f dst4f[N];
        filter->filterSpan4f(src4f, N, dst4f);
        compare_spans(dst4f, dst4b, N, reporter);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

typedef SkPM4f (*SkXfermodeProc4f)(const SkPM4f& src, const SkPM4f& dst);

static bool compare_procs(SkXfermodeProc proc32, SkXfermodeProc4f proc4f) {
    const float kTolerance = 1.0f / 255;

    const SkColor colors[] = {
        0, 0xFF000000, 0xFFFFFFFF, 0x80FF0000
    };

    for (auto s32 : colors) {
        SkPMColor s_pm32 = SkPreMultiplyColor(s32);
        SkPM4f    s_pm4f = SkColor4f::FromColor(s32).premul();
        for (auto d32 : colors) {
            SkPMColor d_pm32 = SkPreMultiplyColor(d32);
            SkPM4f    d_pm4f = SkColor4f::FromColor(d32).premul();

            SkPMColor r32 = proc32(s_pm32, d_pm32);
            SkPM4f    r4f = proc4f(s_pm4f, d_pm4f);

            SkPM4f r32_4f = SkPM4f::FromPMColor(r32);
            if (!nearly_equal(r4f, r32_4f, kTolerance)) {
                return false;
            }
        }
    }
    return true;
}

// Check that our Proc and Proc4f return (nearly) the same results
//
DEF_TEST(Color4f_xfermode_proc4f, reporter) {
    // TODO: extend xfermodes so that all cases can be tested.
    //
    for (int mode = SkXfermode::kClear_Mode; mode <= SkXfermode::kScreen_Mode; ++mode) {
        SkXfermodeProc   proc32 = SkXfermode::GetProc((SkXfermode::Mode)mode);
        SkXfermodeProc4f proc4f = SkXfermode::GetProc4f((SkXfermode::Mode)mode);
        REPORTER_ASSERT(reporter, compare_procs(proc32, proc4f));
    }
}
