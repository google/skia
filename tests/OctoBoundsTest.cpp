/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkRandom.h"
#include "src/gpu/ccpr/GrOctoBounds.h"
#include "tests/Test.h"

using namespace skiatest;

constexpr static float kEpsilon = 1e-3f;

static int numClipsOut = 0;
static int numIntersectClips = 0;

// Ensures devBounds and devBounds45 are valid. Namely, that they are both tight bounding boxes
// around a valid octagon.
static void validate_octo_bounds(
        Reporter* reporter, const SkIRect& clipRect, const GrOctoBounds& octoBounds) {
    // Verify dev bounds are inside the clip rect.
    REPORTER_ASSERT(reporter, octoBounds.left() >= (float)clipRect.left() - kEpsilon);
    REPORTER_ASSERT(reporter, octoBounds.top() >= (float)clipRect.top() - kEpsilon);
    REPORTER_ASSERT(reporter, octoBounds.right() <= (float)clipRect.right() + kEpsilon);
    REPORTER_ASSERT(reporter, octoBounds.bottom() <= (float)clipRect.bottom() + kEpsilon);

    octoBounds.validateBoundsAreTight([reporter](
            bool cond, const char* file, int line, const char* code) {
        if (!cond) {
            reporter->reportFailedWithContext(skiatest::Failure(file, line, code, SkString()));
        }
    });
}

// This is a variant of SkRandom::nextRangeU that can handle negative numbers. As currently written,
// and assuming two's compliment, it would probably work to just call the existing nextRangeU
// implementation with negative value(s), but we go through this method as an extra precaution.
static int next_range_i(SkRandom* rand, int min, int max) {
    int u = rand->nextRangeU(0, max - min);
    return u + min;
}

static void test_octagon(Reporter* reporter, SkRandom* rand, float l, float t, float r, float b) {
    for (int i = 0; i < 20; ++i) {
        float minL45 = GrOctoBounds::Get_x45(l,b);
        float maxL45 = std::min(GrOctoBounds::Get_x45(r,b), GrOctoBounds::Get_x45(l,t));
        float minT45 = GrOctoBounds::Get_y45(l,t);
        float maxT45 = std::min(GrOctoBounds::Get_y45(l,b), GrOctoBounds::Get_y45(r,t));
        float minR45 = std::max(GrOctoBounds::Get_x45(l,t), GrOctoBounds::Get_x45(r,b));
        float maxR45 = GrOctoBounds::Get_x45(r,t);
        float minB45 = std::max(GrOctoBounds::Get_y45(r,t), GrOctoBounds::Get_y45(l,b));
        float maxB45 = GrOctoBounds::Get_y45(r,b);

        // Pick somewhat valid 45 degree bounds.
        float l45 = rand->nextRangeF(minL45, maxL45);
        float t45 = rand->nextRangeF(minT45, maxT45);
        float r45 = rand->nextRangeF(minR45, maxR45);
        float b45 = rand->nextRangeF(minB45, maxB45);

        // Grow out diagonal corners if too tight, making 45 bounds valid.
        std::function<void()> growOutDiagonals[4] = {
            [&]() {  // Push top-left diagonal corner outside left edge.
                float miss = GrOctoBounds::Get_x(l45,t45) - l;
                if (miss > 0) {
                    // x = (x45 + y45)/2
                    l45 -= miss;
                    if (l45 < minL45) {
                        t45 -= minL45 - l45;
                        l45 = minL45;
                    }
                    t45 -= miss;
                    if (t45 < minT45) {
                        l45 -= minT45 - t45;
                        t45 = minT45;
                    }
                }
            },
            [&]() {  // Push top-right diagonal corner outside top edge.
                float miss = GrOctoBounds::Get_y(r45,t45) - t;
                if (miss > 0) {
                    // y = (y45 - x45)/2
                    r45 += miss;
                    if (r45 > maxR45) {
                        t45 -= r45 - maxR45;
                        r45 = maxR45;
                    }
                    t45 -= miss;
                    if (t45 < minT45) {
                        r45 += minT45 - t45;
                        t45 = minT45;
                    }
                }
            },
            [&]() {  // Push bottom-right diagonal corner outside right edge.
                float miss = r - GrOctoBounds::Get_x(r45,b45);
                if (miss > 0) {
                    // x = (x45 + y45)/2
                    r45 += miss;
                    if (r45 > maxR45) {
                        b45 += r45 - maxR45;
                        r45 = maxR45;
                    }
                    b45 += miss;
                    if (b45 > maxB45) {
                        r45 += b45 - maxB45;
                        b45 = maxB45;
                    }
                }
            },
            [&]() {  // Push bottom-left diagonal corner outside bottom edge.
                float miss = b - GrOctoBounds::Get_y(l45,b45);
                if (miss > 0) {
                    // y = (y45 - x45)/2
                    l45 -= miss;
                    if (l45 < minL45) {
                        b45 += minL45 - l45;
                        l45 = minL45;
                    }
                    b45 += miss;
                    if (b45 > maxB45) {
                        l45 -= b45 - maxB45;
                        b45 = maxB45;
                    }
                }
            },
        };
        // Shuffle.
        for (int i = 0; i < 10; ++i) {
            std::swap(growOutDiagonals[rand->nextRangeU(0, 3)],
                      growOutDiagonals[rand->nextRangeU(0, 3)]);
        }
        for (const auto& f : growOutDiagonals) {
            f();
        }

        GrOctoBounds octoBounds(SkRect::MakeLTRB(l,t,r,b), SkRect::MakeLTRB(l45,t45,r45,b45));

        SkIRect devIBounds;
        octoBounds.roundOut(&devIBounds);

        // Test a clip rect that completely encloses the octagon.
        bool clipSuccess = octoBounds.clip(devIBounds);
        REPORTER_ASSERT(reporter, clipSuccess);
        // Should not have clipped anything.
        REPORTER_ASSERT(reporter, octoBounds == GrOctoBounds({l,t,r,b}, {l45,t45,r45,b45}));
        validate_octo_bounds(reporter, devIBounds, octoBounds);

        // Test a bunch of random clip rects.
        for (int j = 0; j < 20; ++j) {
            SkIRect clipRect;
            do {
                clipRect.fLeft = next_range_i(rand, devIBounds.left(), devIBounds.right() - 1);
                clipRect.fTop = next_range_i(rand, devIBounds.top(), devIBounds.bottom() - 1);
                clipRect.fRight = next_range_i(rand, clipRect.left() + 1, devIBounds.right());
                clipRect.fBottom = next_range_i(rand, clipRect.top() + 1, devIBounds.bottom());
            } while (clipRect == devIBounds);

            GrOctoBounds octoBoundsClipped = octoBounds;
            if (!octoBoundsClipped.clip(clipRect)) {
                // Ensure clipRect is completely outside one of the diagonals.
                float il = (float)clipRect.left();
                float it = (float)clipRect.top();
                float ir = (float)clipRect.right();
                float ib = (float)clipRect.bottom();
                REPORTER_ASSERT(reporter,
                        GrOctoBounds::Get_x45(ir,it) <= l45 + kEpsilon ||
                        GrOctoBounds::Get_y45(ir,ib) <= t45 + kEpsilon ||
                        GrOctoBounds::Get_x45(il,ib) >= r45 - kEpsilon ||
                        GrOctoBounds::Get_y45(il,it) >= b45 - kEpsilon);
                ++numClipsOut;
            } else {
                validate_octo_bounds(reporter, clipRect, octoBoundsClipped);
                ++numIntersectClips;
            }
        }
    }

}

DEF_TEST(OctoBounds, reporter) {
    numClipsOut = 0;
    numIntersectClips = 0;

    SkRandom rand;
    test_octagon(reporter, &rand, 0, 0, 100, 100);
    test_octagon(reporter, &rand, -2, 0, 2, 100);
    test_octagon(reporter, &rand, 0, -10, 100, 0);
    // We can't test Infs or NaNs because they trigger internal asserts when setting GrOctoBounds.

    // Sanity check on our random clip testing.. Just make we hit both types of clip.
    REPORTER_ASSERT(reporter, numClipsOut > 0);
    REPORTER_ASSERT(reporter, numIntersectClips > 0);
}
