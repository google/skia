/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAutoMalloc.h"
#include "SkBlendMode.h"
#include "SkColor.h"
#include "SkColorFilter.h"
#include "SkRandom.h"
#include "SkReadBuffer.h"
#include "SkRefCnt.h"
#include "SkWriteBuffer.h"
#include "SkTypes.h"
#include "Test.h"

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

static sk_sp<SkColorFilter> make_filter() {
    // pick a filter that cannot compose with itself via newComposed()
    return SkColorFilters::Blend(SK_ColorRED, SkBlendMode::kColorBurn);
}

static void test_composecolorfilter_limit(skiatest::Reporter* reporter) {
    // Test that CreateComposeFilter() has some finite limit (i.e. that the factory can return null)
    const int way_too_many = 100;
    auto parent(make_filter());
    for (int i = 2; i < way_too_many; ++i) {
        auto filter(make_filter());
        parent = parent->makeComposed(filter);
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
