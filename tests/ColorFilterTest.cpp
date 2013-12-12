/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkLumaColorFilter.h"
#include "SkRandom.h"
#include "SkXfermode.h"
#include "SkOrderedReadBuffer.h"
#include "SkOrderedWriteBuffer.h"

static SkColorFilter* reincarnate_colorfilter(SkFlattenable* obj) {
    SkOrderedWriteBuffer wb(1024);
    wb.writeFlattenable(obj);

    size_t size = wb.size();
    SkAutoSMalloc<1024> storage(size);
    // make a copy into storage
    wb.writeToMemory(storage.get());

    SkOrderedReadBuffer rb(storage.get(), size);
    return rb.readColorFilter();
}

///////////////////////////////////////////////////////////////////////////////

#define ILLEGAL_MODE    ((SkXfermode::Mode)-1)

DEF_TEST(ColorFilter, reporter) {
    SkRandom rand;

    for (int mode = 0; mode <= SkXfermode::kLastMode; mode++) {
        SkColor color = rand.nextU();

        // ensure we always get a filter, by avoiding the possibility of a
        // special case that would return NULL (if color's alpha is 0 or 0xFF)
        color = SkColorSetA(color, 0x7F);

        SkColorFilter* cf = SkColorFilter::CreateModeFilter(color,
                                                        (SkXfermode::Mode)mode);

        // allow for no filter if we're in Dst mode (its a no op)
        if (SkXfermode::kDst_Mode == mode && NULL == cf) {
            continue;
        }

        SkAutoUnref aur(cf);
        REPORTER_ASSERT(reporter, cf);

        SkColor c = ~color;
        SkXfermode::Mode m = ILLEGAL_MODE;

        SkColor expectedColor = color;
        SkXfermode::Mode expectedMode = (SkXfermode::Mode)mode;

//        SkDebugf("--- mc [%d %x] ", mode, color);

        REPORTER_ASSERT(reporter, cf->asColorMode(&c, &m));
        // handle special-case folding by the factory
        if (SkXfermode::kClear_Mode == mode) {
            if (c != expectedColor) {
                expectedColor = 0;
            }
            if (m != expectedMode) {
                expectedMode = SkXfermode::kSrc_Mode;
            }
        }

//        SkDebugf("--- got [%d %x] expected [%d %x]\n", m, c, expectedMode, expectedColor);

        REPORTER_ASSERT(reporter, c == expectedColor);
        REPORTER_ASSERT(reporter, m == expectedMode);

        {
            SkColorFilter* cf2 = reincarnate_colorfilter(cf);
            SkAutoUnref aur2(cf2);
            REPORTER_ASSERT(reporter, cf2);

            SkColor c2 = ~color;
            SkXfermode::Mode m2 = ILLEGAL_MODE;
            REPORTER_ASSERT(reporter, cf2->asColorMode(&c2, &m2));
            REPORTER_ASSERT(reporter, c2 == expectedColor);
            REPORTER_ASSERT(reporter, m2 == expectedMode);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

DEF_TEST(LumaColorFilter, reporter) {
    SkPMColor in, out;
    SkAutoTUnref<SkColorFilter> lf(SkLumaColorFilter::Create());

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
