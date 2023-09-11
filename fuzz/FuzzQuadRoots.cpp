/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkFloatingPoint.h"
#include "src/base/SkCubics.h"
#include "src/base/SkQuads.h"
#include "src/base/SkUtils.h"

#include <cmath>

static void fuzz_quad_real_roots(double A, double B, double C) {
    double roots[2];
    const int numSolutions = SkQuads::RootsReal(A, B, C, roots);
    SkASSERT_RELEASE(numSolutions >= 0 && numSolutions <= 2);
    for (int i = 0; i < numSolutions; i++) {
        SkASSERT_RELEASE(std::isfinite(roots[i]));
        // You may be tempted to add assertions that plug the provided solutions into
        // the quadratic equation and verify that the result is zero. Be advised
        // that the fuzzer is very good at finding float values that result in
        // seemingly arbitrarily large errors, due to the imprecision of floating
        // point math. Unless the input range is sufficiently small, such an
        // effort seems fruitless.
    }
    if (numSolutions == 2) {
        // Roots should not be duplicated
        SkASSERT_RELEASE(!sk_doubles_nearly_equal_ulps(roots[0], roots[1]));
    }
}

DEF_FUZZ(QuadRoots, fuzz) {
    double A, B, C;
    fuzz->next(&A);
    fuzz->next(&B);
    fuzz->next(&C);

    // Uncomment for easy test case creation
//    SkDebugf("A %16e (0x%lx) B %16e (0x%lx) C %16e (0x%lx)\n",
//             A, sk_bit_cast<uint64_t>(A), B, sk_bit_cast<uint64_t>(B),
//             C, sk_bit_cast<uint64_t>(C));

    fuzz_quad_real_roots(A, B, C);
}
