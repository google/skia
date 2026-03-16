/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "src/gpu/Swizzle.h"

namespace skgpu {

DEF_TEST(SwizzleInvertTest, r) {
    struct {
        const char* fTest;
        const char* fExpectedInverse;
        bool fTrueInverse;
    } kSwizzleTests[] = {
        // These are true inverses since they use each of r,g,b,a exactly once
        {.fTest="rgba", .fExpectedInverse="rgba", .fTrueInverse=true},
        {.fTest="bgra", .fExpectedInverse="bgra", .fTrueInverse=true},
        {.fTest="garb", .fExpectedInverse="brag", .fTrueInverse=true},
        {.fTest="argb", .fExpectedInverse="gbar", .fTrueInverse=true},
        {.fTest="abgr", .fExpectedInverse="abgr", .fTrueInverse=true},
        // These are true inverses because the channels with constants are not referenced by the
        // other channels being swizzled
        {.fTest="000r", .fExpectedInverse="a001", .fTrueInverse=true},
        {.fTest="rgb1", .fExpectedInverse="rgb1", .fTrueInverse=true},
        {.fTest="bgr1", .fExpectedInverse="bgr1", .fTrueInverse=true},
        // These are not true inverses because not every channel is used, and the missing channels
        // are not set to a constant 0 or 1
        {.fTest="rrr1", .fExpectedInverse="r001", .fTrueInverse=false},
        {.fTest="rrra", .fExpectedInverse="r00a", .fTrueInverse=false},
        // This tests that a non-default constant is preserved if the channel is not in conflict
        // with any other swizzle component (e.g. B=1, but nothing references 'b' in rr11 so
        // preserve B=1 instead of falling back to the default B=0. At the same time, the inverse
        // G = 0 because had set it to 'r' and nothing references 'g' to provide another value).
        {.fTest="rr11", .fExpectedInverse="r011", .fTrueInverse=false},
        // This one is subtle, because both a000 and a001 map to 000r, but 000r maps back to a001
        {.fTest="a000", .fExpectedInverse="000r", .fTrueInverse=false},
    };

    for (auto t : kSwizzleTests) {
        Swizzle actualInverse = Swizzle(t.fTest).invert();
        REPORTER_ASSERT(r, Swizzle(actualInverse) == Swizzle(t.fExpectedInverse),
                        "Expected inverse(%s) == %s, but was %s",
                        t.fTest, t.fExpectedInverse, actualInverse.asString().c_str());
        if (t.fTrueInverse) {
            // For a true inverse, the inverse of the inverse should be the original
            Swizzle original = actualInverse.invert();
            REPORTER_ASSERT(r, Swizzle(t.fTest) == original,
                            "Expected inverse(inverse(%s)) == inverse(%s) == %s, but was %s",
                            t.fTest, actualInverse.asString().c_str(), t.fTest,
                            original.asString().c_str());
        }
    }
}

} // namespace skgpu
