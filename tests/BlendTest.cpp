/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkTaskGroup.h"
#include <functional>

struct Results { int diffs, diffs_0x00, diffs_0xff, diffs_by_1; };

static bool acceptable(const Results& r) {
#if 0
    SkDebugf("%d diffs, %d at 0x00, %d at 0xff, %d off by 1, all out of 65536\n",
             r.diffs, r.diffs_0x00, r.diffs_0xff, r.diffs_by_1);
#endif
    return r.diffs_by_1 == r.diffs   // never off by more than 1
        && r.diffs_0x00 == 0         // transparent must stay transparent
        && r.diffs_0xff == 0;        // opaque must stay opaque
}

template <typename Fn>
static Results test(Fn&& multiply) {
    Results r = { 0,0,0,0 };
    for (int x = 0; x < 256; x++) {
    for (int y = 0; y < 256; y++) {
        int p = multiply(x, y),
            ideal = (x*y+127)/255;
        if (p != ideal) {
            r.diffs++;
            if (x == 0x00 || y == 0x00) { r.diffs_0x00++; }
            if (x == 0xff || y == 0xff) { r.diffs_0xff++; }
            if (SkTAbs(ideal - p) == 1) { r.diffs_by_1++; }
        }
    }}
    return r;
}

DEF_TEST(Blend_byte_multiply, r) {
    // These are all temptingly close but fundamentally broken.
    int (*broken[])(int, int) = {
        [](int x, int y) { return (x*y)>>8; },
        [](int x, int y) { return (x*y+128)>>8; },
        [](int x, int y) { y += y>>7; return (x*y)>>8; },
    };
    for (auto multiply : broken) { REPORTER_ASSERT(r, !acceptable(test(multiply))); }

    // These are fine to use, but not perfect.
    int (*fine[])(int, int) = {
        [](int x, int y) { return (x*y+x)>>8; },
        [](int x, int y) { return (x*y+y)>>8; },
        [](int x, int y) { return (x*y+255)>>8; },
        [](int x, int y) { y += y>>7; return (x*y+128)>>8; },
    };
    for (auto multiply : fine) { REPORTER_ASSERT(r, acceptable(test(multiply))); }

    // These are pefect.
    int (*perfect[])(int, int) = {
        [](int x, int y) { return (x*y+127)/255; },  // Duh.
        [](int x, int y) { int p = (x*y+128); return (p+(p>>8))>>8; },
        [](int x, int y) { return ((x*y+128)*257)>>16; },
    };
    for (auto multiply : perfect) { REPORTER_ASSERT(r, test(multiply).diffs == 0); }
}
