/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBlendMode.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkColorMatrix.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "tests/Test.h"

class SkFlattenable;

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

#define ILLEGAL_MODE    ((SkBlendMode)-1)

DEF_TEST(ColorFilter, reporter) {
    SkRandom rand;

    for (int mode = 0; mode <= (int)SkBlendMode::kLastMode; mode++) {
        SkColor color = rand.nextU();

        // ensure we always get a filter, by avoiding the possibility of a
        // special case that would return nullptr (if color's alpha is 0 or 0xFF)
        color = SkColorSetA(color, 0x7F);

        auto cf = SkColorFilters::Blend(color, (SkBlendMode)mode);

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

        REPORTER_ASSERT(reporter, cf->asAColorMode(&c, (SkBlendMode*)&m));
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
            REPORTER_ASSERT(reporter, cf2->asAColorMode(&c2, (SkBlendMode*)&m2));
            REPORTER_ASSERT(reporter, c2 == expectedColor);
            REPORTER_ASSERT(reporter, m2 == expectedMode);
        }
    }
}

DEF_TEST(WorkingFormatFilterFlags, r) {
    {
        // A matrix with final row 0,0,0,1,0 shouldn't change alpha.
        sk_sp<SkColorFilter> cf = SkColorFilters::Matrix({1,0,0,0,0,
                                                          0,1,0,0,0,
                                                          0,0,1,0,0,
                                                          0,0,0,1,0});
        REPORTER_ASSERT(r, cf->isAlphaUnchanged());

        // No working format change will itself change alpha.
        SkAlphaType unpremul = kUnpremul_SkAlphaType;
        cf = SkColorFilters::WithWorkingFormat(std::move(cf),
                                               &SkNamedTransferFn::kLinear,
                                               &SkNamedGamut::kDisplayP3,
                                               &unpremul);
        REPORTER_ASSERT(r, cf->isAlphaUnchanged());
    }

    {
        // Here's a matrix that definitely does change alpha.
        sk_sp<SkColorFilter> cf = SkColorFilters::Matrix({1,0,0,0,0,
                                                          0,1,0,0,0,
                                                          0,0,1,0,0,
                                                          0,0,0,0,1});
        REPORTER_ASSERT(r, !cf->isAlphaUnchanged());

        SkAlphaType unpremul = kUnpremul_SkAlphaType;
        cf = SkColorFilters::WithWorkingFormat(std::move(cf),
                                               &SkNamedTransferFn::kLinear,
                                               &SkNamedGamut::kDisplayP3,
                                               &unpremul);
        REPORTER_ASSERT(r, !cf->isAlphaUnchanged());
    }
}
