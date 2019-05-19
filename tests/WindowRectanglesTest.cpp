/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "tests/Test.h"

#include "include/utils/SkRandom.h"
#include "src/core/SkRectPriv.h"
#include "src/gpu/GrWindowRectangles.h"

static SkIRect next_irect(SkRandom& r) {
    return {r.nextS(), r.nextS(), r.nextS(), r.nextS()};
}

DEF_TEST(WindowRectangles, reporter) {
    SkRandom r;

    SkIRect windowData[GrWindowRectangles::kMaxWindows];
    for (int i = 0; i < GrWindowRectangles::kMaxWindows; ++i) {
        windowData[i] = next_irect(r);
    }

    GrWindowRectangles wr;
    for (int i = 0; i < GrWindowRectangles::kMaxWindows - 1; ++i) {
        REPORTER_ASSERT(reporter, wr.count() == i);
        REPORTER_ASSERT(reporter, !memcmp(wr.data(), windowData, i * sizeof(SkIRect)));

        GrWindowRectangles wr2(wr);
        REPORTER_ASSERT(reporter, wr2 == wr);
        REPORTER_ASSERT(reporter, wr2.count() == wr.count());
        REPORTER_ASSERT(reporter, !memcmp(wr2.data(), wr.data(), i * sizeof(SkIRect)));

        wr.addWindow(windowData[i]);
    }

    SkASSERT(wr.count() == GrWindowRectangles::kMaxWindows - 1);
    {
        GrWindowRectangles A(wr), B(wr);
        REPORTER_ASSERT(reporter, B == A);
        REPORTER_ASSERT(reporter, B.data() == A.data()); // Should use copy-on-write.

        A.addWindow(windowData[GrWindowRectangles::kMaxWindows - 1]);
        REPORTER_ASSERT(reporter, B.data() != A.data());
        REPORTER_ASSERT(reporter, B != A);

        B.addWindow(SkRectPriv::MakeILarge());
        REPORTER_ASSERT(reporter, B != A);

        REPORTER_ASSERT(reporter, !memcmp(A.data(), windowData,
                                          GrWindowRectangles::kMaxWindows * sizeof(SkIRect)));
        REPORTER_ASSERT(reporter, !memcmp(B.data(), windowData,
                                          (GrWindowRectangles::kMaxWindows - 1) * sizeof(SkIRect)));
        REPORTER_ASSERT(reporter,
                        B.data()[GrWindowRectangles::kMaxWindows - 1] == SkRectPriv::MakeILarge());
    }
    {
        GrWindowRectangles A(wr), B(wr);
        REPORTER_ASSERT(reporter, B == A);
        REPORTER_ASSERT(reporter, B.data() == A.data()); // Should use copy-on-write.

        A.addWindow(windowData[GrWindowRectangles::kMaxWindows - 1]);
        B.addWindow(windowData[GrWindowRectangles::kMaxWindows - 1]);
        REPORTER_ASSERT(reporter, B == A);
        REPORTER_ASSERT(reporter, B.data() != A.data());
        REPORTER_ASSERT(reporter, !memcmp(B.data(), A.data(),
                                          GrWindowRectangles::kMaxWindows * sizeof(SkIRect)));
        REPORTER_ASSERT(reporter, !memcmp(A.data(), windowData,
                                          GrWindowRectangles::kMaxWindows * sizeof(SkIRect)));
    }
}
