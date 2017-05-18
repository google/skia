/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAutoMalloc.h"
#include "SkColor.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkLumaColorFilter.h"
#include "SkRandom.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkRandom.h"
#include "Test.h"

static sk_sp<SkColorFilter> reincarnate_colorfilter(SkFlattenable* obj) {
    SkBinaryWriteBuffer wb;
    wb.writeFlattenable(obj);

    size_t size = wb.bytesWritten();
    SkAutoSMalloc<1024> storage(size);
    // make a copy into storage
    wb.writeToMemory(storage.get());

    SkReadBuffer rb(storage.get(), size);
    return rb.readColorFilter();
}

///////////////////////////////////////////////////////////////////////////////

static sk_sp<SkColorFilter> make_filter() {
    // pick a filter that cannot compose with itself via newComposed()
    return SkColorFilter::MakeModeFilter(SK_ColorRED, SkBlendMode::kColorBurn);
}

static void test_composecolorfilter_limit(skiatest::Reporter* reporter) {
    // Test that CreateComposeFilter() has some finite limit (i.e. that the factory can return null)
    const int way_too_many = 100;
    auto parent(make_filter());
    for (int i = 2; i < way_too_many; ++i) {
        auto filter(make_filter());
        parent = SkColorFilter::MakeComposeFilter(parent, filter);
        if (nullptr == parent) {
            REPORTER_ASSERT(reporter, i > 2); // we need to have succeeded at least once!
            return;
        }
    }
    REPORTER_ASSERT(reporter, false); // we never saw a nullptr :(
}

#define ILLEGAL_MODE    ((SkBlendMode)-1)

DEF_TEST(ColorFilter, reporter) {
    SkRandom rand;

    for (int mode = 0; mode <= (int)SkBlendMode::kLastMode; mode++) {
        SkColor color = rand.nextU();

        // ensure we always get a filter, by avoiding the possibility of a
        // special case that would return nullptr (if color's alpha is 0 or 0xFF)
        color = SkColorSetA(color, 0x7F);

        auto cf = SkColorFilter::MakeModeFilter(color, (SkBlendMode)mode);

        // allow for no filter if we're in Dst mode (its a no op)
        if (SkBlendMode::kDst == (SkBlendMode)mode && nullptr == cf) {
            continue;
        }

        REPORTER_ASSERT(reporter, cf);

        SkColor c = ~color;
        SkBlendMode m = ILLEGAL_MODE;

        SkColor expectedColor = color;
        SkBlendMode expectedMode = (SkBlendMode)mode;

//        SkDebugf("--- mc [%d %x] ", mode, color);

        REPORTER_ASSERT(reporter, cf->asColorMode(&c, (SkBlendMode*)&m));
        // handle special-case folding by the factory
        if (SkBlendMode::kClear == (SkBlendMode)mode) {
            if (c != expectedColor) {
                expectedColor = 0;
            }
            if (m != expectedMode) {
                expectedMode = SkBlendMode::kSrc;
            }
        }

//        SkDebugf("--- got [%d %x] expected [%d %x]\n", m, c, expectedMode, expectedColor);

        REPORTER_ASSERT(reporter, c == expectedColor);
        REPORTER_ASSERT(reporter, m == expectedMode);

        {
            auto cf2 = reincarnate_colorfilter(cf.get());
            REPORTER_ASSERT(reporter, cf2);

            SkColor c2 = ~color;
            SkBlendMode m2 = ILLEGAL_MODE;
            REPORTER_ASSERT(reporter, cf2->asColorMode(&c2, (SkBlendMode*)&m2));
            REPORTER_ASSERT(reporter, c2 == expectedColor);
            REPORTER_ASSERT(reporter, m2 == expectedMode);
        }
    }

    test_composecolorfilter_limit(reporter);
}

///////////////////////////////////////////////////////////////////////////////

DEF_TEST(LumaColorFilter, reporter) {
    SkPMColor in, out;
    auto lf(SkLumaColorFilter::Make());

    // Applying luma to white produces black with the same transparency.
    for (unsigned i = 0; i < 256; ++i) {
        in = SkPackARGB32(i, i, i, i);
        lf->filterSpan(&in, 1, &out);
        REPORTER_ASSERT(reporter, SkGetPackedA32(out) == i);
        REPORTER_ASSERT(reporter, SkGetPackedR32(out) == 0);
        REPORTER_ASSERT(reporter, SkGetPackedG32(out) == 0);
        REPORTER_ASSERT(reporter, SkGetPackedB32(out) == 0);
    }

    // Applying luma to black yields transparent black (luminance(black) == 0)
    for (unsigned i = 0; i < 256; ++i) {
        in = SkPackARGB32(i, 0, 0, 0);
        lf->filterSpan(&in, 1, &out);
        REPORTER_ASSERT(reporter, out == SK_ColorTRANSPARENT);
    }

    // For general colors, a luma filter generates black with an attenuated alpha channel.
    for (unsigned i = 1; i < 256; ++i) {
        in = SkPackARGB32(i, i, i / 2, i / 3);
        lf->filterSpan(&in, 1, &out);
        REPORTER_ASSERT(reporter, out != in);
        REPORTER_ASSERT(reporter, SkGetPackedA32(out) <= i);
        REPORTER_ASSERT(reporter, SkGetPackedR32(out) == 0);
        REPORTER_ASSERT(reporter, SkGetPackedG32(out) == 0);
        REPORTER_ASSERT(reporter, SkGetPackedB32(out) == 0);
    }
}

///////////////////////////////////////////////////////////////////////////////

#include "SkColorMatrixFilter.h"

static void get_brightness_matrix(float amount, float matrix[20]) {
    // Spec implementation
    // (http://dvcs.w3.org/hg/FXTF/raw-file/tip/filters/index.html#brightnessEquivalent)
    // <feFunc[R|G|B] type="linear" slope="[amount]">
    memset(matrix, 0, 20 * sizeof(SkScalar));
    matrix[0] = matrix[6] = matrix[12] = amount;
    matrix[18] = 1.f;
}

static void get_grayscale_matrix(float amount, float matrix[20]) {
    // Note, these values are computed to ensure MatrixNeedsClamping is false
    // for amount in [0..1]
    matrix[0] = 0.2126f + 0.7874f * amount;
    matrix[1] = 0.7152f - 0.7152f * amount;
    matrix[2] = 1.f - (matrix[0] + matrix[1]);
    matrix[3] = matrix[4] = 0.f;

    matrix[5] = 0.2126f - 0.2126f * amount;
    matrix[6] = 0.7152f + 0.2848f * amount;
    matrix[7] = 1.f - (matrix[5] + matrix[6]);
    matrix[8] = matrix[9] = 0.f;

    matrix[10] = 0.2126f - 0.2126f * amount;
    matrix[11] = 0.7152f - 0.7152f * amount;
    matrix[12] = 1.f - (matrix[10] + matrix[11]);
    matrix[13] = matrix[14] = 0.f;

    matrix[15] = matrix[16] = matrix[17] = matrix[19] = 0.f;
    matrix[18] = 1.f;
}

static sk_sp<SkColorFilter> make_cf0() {
    SkScalar matrix[20];
    get_brightness_matrix(0.5f, matrix);
    return SkColorFilter::MakeMatrixFilterRowMajor255(matrix);
}
static sk_sp<SkColorFilter> make_cf1() {
    SkScalar matrix[20];
    get_grayscale_matrix(1, matrix);
    return SkColorFilter::MakeMatrixFilterRowMajor255(matrix);
}
static sk_sp<SkColorFilter> make_cf2() {
    SkColorMatrix m0, m1;
    get_brightness_matrix(0.5f, m0.fMat);
    get_grayscale_matrix(1, m1.fMat);
    m0.preConcat(m1);
    return SkColorFilter::MakeMatrixFilterRowMajor255(m0.fMat);
}
static sk_sp<SkColorFilter> make_cf3() {
    SkColorMatrix m0, m1;
    get_brightness_matrix(0.5f, m0.fMat);
    get_grayscale_matrix(1, m1.fMat);
    m0.postConcat(m1);
    return SkColorFilter::MakeMatrixFilterRowMajor255(m0.fMat);
}
typedef sk_sp<SkColorFilter> (*CFProc)();

// Test that a colormatrix that "should" preserve opaquness actually does.
DEF_TEST(ColorMatrixFilter, reporter) {
    const CFProc procs[] = {
        make_cf0, make_cf1, make_cf2, make_cf3,
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(procs); ++i) {
        auto cf(procs[i]());

        // generate all possible r,g,b triples
        for (int r = 0; r < 256; ++r) {
            for (int g = 0; g < 256; ++g) {
                SkPMColor storage[256];
                for (int b = 0; b < 256; ++b) {
                    storage[b] = SkPackARGB32(0xFF, r, g, b);
                }
                cf->filterSpan(storage, 256, storage);
                for (int b = 0; b < 256; ++b) {
                    REPORTER_ASSERT(reporter, 0xFF == SkGetPackedA32(storage[b]));
                }
            }
        }
    }
}
