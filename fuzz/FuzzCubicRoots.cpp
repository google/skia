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

static void fuzz_cubic_real_roots(double A, double B, double C, double D) {
    double roots[3];
    const int numSolutions = SkCubics::RootsReal(A, B, C, D, roots);
    SkASSERT_RELEASE(numSolutions >= 0 && numSolutions <= 3);
    for (int i = 0; i < numSolutions; i++) {
        SkASSERT_RELEASE(std::isfinite(roots[i]));
    }
    // Roots should not be duplicated
    if (numSolutions >= 2) {
        SkASSERT_RELEASE(!sk_doubles_nearly_equal_ulps(roots[0], roots[1]));
    }
    if (numSolutions == 3) {
        SkASSERT_RELEASE(!sk_doubles_nearly_equal_ulps(roots[1], roots[2]));
        SkASSERT_RELEASE(!sk_doubles_nearly_equal_ulps(roots[0], roots[2]));
    }
}

static void fuzz_cubic_roots_valid_t(double A, double B, double C, double D) {
    double roots[3];
    const int numSolutions = SkCubics::RootsValidT(A, B, C, D, roots);
    SkASSERT_RELEASE(numSolutions >= 0 && numSolutions <= 3);
    for (int i = 0; i < numSolutions; i++) {
        SkASSERT_RELEASE(std::isfinite(roots[i]));
        SkASSERT_RELEASE(roots[i] >= 0.0);
        SkASSERT_RELEASE(roots[i] <= 1.0);
    }
    // Roots should not be duplicated
    if (numSolutions >= 2) {
        SkASSERT_RELEASE(!sk_doubles_nearly_equal_ulps(roots[0], roots[1]));
    }
    if (numSolutions == 3) {
        SkASSERT_RELEASE(!sk_doubles_nearly_equal_ulps(roots[1], roots[2]));
        SkASSERT_RELEASE(!sk_doubles_nearly_equal_ulps(roots[0], roots[2]));
    }
}

static void fuzz_cubic_roots_binary_search(double A, double B, double C, double D) {
    double roots[3];
    const int numSolutions = SkCubics::BinarySearchRootsValidT(A, B, C, D, roots);
    SkASSERT_RELEASE(numSolutions >= 0 && numSolutions <= 3);
    for (int i = 0; i < numSolutions; i++) {
        SkASSERT_RELEASE(std::isfinite(roots[i]));
        SkASSERT_RELEASE(roots[i] >= 0.0);
        SkASSERT_RELEASE(roots[i] <= 1.0);
        double actual = SkCubics::EvalAt(A, B, C, D, roots[i]);
        // The binary search algorithm *should* be accurate regardless of the inputs.
        SkASSERT_RELEASE(std::abs(actual) < 0.001);
    }
    // Roots should not be duplicated
    if (numSolutions >= 2) {
        SkASSERT_RELEASE(!sk_doubles_nearly_equal_ulps(roots[0], roots[1]));
    }
    if (numSolutions == 3) {
        SkASSERT_RELEASE(!sk_doubles_nearly_equal_ulps(roots[1], roots[2]));
        SkASSERT_RELEASE(!sk_doubles_nearly_equal_ulps(roots[0], roots[2]));
    }
}

DEF_FUZZ(CubicRoots, fuzz) {
    double A, B, C, D;
    fuzz->next(&A);
    fuzz->next(&B);
    fuzz->next(&C);
    fuzz->next(&D);

    // Uncomment for easy test case creation
//    SkDebugf("A %16e (0x%lx) B %16e (0x%lx) C %16e (0x%lx) D %16e (0x%lx)\n",
//             A, sk_bit_cast<uint64_t>(A), B, sk_bit_cast<uint64_t>(B),
//             C, sk_bit_cast<uint64_t>(C), D, sk_bit_cast<uint64_t>(D));
    fuzz_cubic_real_roots(A, B, C, D);

    fuzz_cubic_roots_valid_t(A, B, C, D);

    fuzz_cubic_roots_binary_search(A, B, C, D);
}
