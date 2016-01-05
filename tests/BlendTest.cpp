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
#include "SkXfermode.h"
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

DEF_TEST(Blend_premul_begets_premul, r) {
    // This test is quite slow, even if you have enough cores to run each mode in parallel.
    if (!r->allowExtendedTest()) {
        return;
    }

    // No matter what xfermode we use, premul inputs should create premul outputs.
    auto test_mode = [&](int m) {
        SkXfermode::Mode mode = (SkXfermode::Mode)m;
        if (mode == SkXfermode::kSrcOver_Mode) {
            return;  // TODO: can't create a SrcOver xfermode.
        }
        SkAutoTUnref<SkXfermode> xfermode(SkXfermode::Create(mode));
        SkASSERT(xfermode);
        // We'll test all alphas and legal color values, assuming all colors work the same.
        // This is not true for non-separable blend modes, but this test still can't hurt.
        for (int sa = 0; sa <= 255; sa++) {
        for (int da = 0; da <= 255; da++) {
        for (int  s = 0;  s <= sa;   s++) {
        for (int  d = 0;  d <= da;   d++) {
            SkPMColor src = SkPackARGB32(sa, s, s, s),
                      dst = SkPackARGB32(da, d, d, d);
            xfermode->xfer32(&dst, &src, 1, nullptr);  // To keep it simple, no AA.
            if (!SkPMColorValid(dst)) {
                ERRORF(r, "%08x is not premul using %s", dst, SkXfermode::ModeName(mode));
            }
        }}}}
    };

    // Parallelism helps speed things up on my desktop from ~725s to ~50s.
    SkTaskGroup().batch(SkXfermode::kLastMode, test_mode);
}
