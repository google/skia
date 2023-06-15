/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkQuads.h"

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkFloatingPoint.h"

#include <cmath>
#include <limits>

// Solve 0 = M * x + B. If M is 0, there are no solutions, unless B is also 0,
// in which case there are infinite solutions, so we just return 1 of them.
static int solve_linear(const double M, const double B, double solution[2]) {
    if (sk_double_nearly_zero(M)) {
        solution[0] = 0;
        if (sk_double_nearly_zero(B)) {
            return 1;
        }
        return 0;
    }
    solution[0] = -B / M;
    if (!std::isfinite(solution[0])) {
        return 0;
    }
    return 1;
}

// When B >> A, then the x^2 component doesn't contribute much to the output, so the second root
// will be very large, but have massive round off error. Because of the round off error, the
// second root will not evaluate to zero when substituted back into the quadratic equation. In
// the situation when B >> A, then just treat the quadratic as a linear equation.
static bool close_to_linear(double A, double B) {
    if (A != 0) {
        // Return if B is much bigger than A.
        return std::abs(B / A) >= 1.0e+16;
    }

    // Otherwise A is zero, and the quadratic is linear.
    return true;
}

double SkQuads::Discriminant(const double a, const double b, const double c) {
    const double b2 = b * b;
    const double ac = a * c;

    // Calculate the rough discriminate which may suffer from a loss in precision due to b2 and
    // ac being too close.
    const double roughDiscriminant = b2 - ac;

    // We would like the calculated discriminant to have a relative error of 2-bits or less. For
    // doubles, this means the relative error is <= E = 3*2^-53. This gives a relative error
    // bounds of:
    //
    //     |D - D~| / |D| <= E,
    //
    // where D = B*B - AC, and D~ is the floating point approximation of D.
    // Define the following equations
    //     B2 = B*B,
    //     B2~ = B2(1 + eB2), where eB2 is the floating point round off,
    //     AC = A*C,
    //     AC~ = AC(1 + eAC), where eAC is the floating point round off, and
    //     D~ = B2~ - AC~.
    //  We can now rewrite the above bounds as
    //
    //     |B2 - AC - (B2~ - AC~)| / |B2 - AC| = |B2 - AC - B2~ + AC~| / |B2 - AC| <= E.
    //
    //  Substituting B2~ and AC~, and canceling terms gives
    //
    //     |eAC * AC - eB2 * B2| / |B2 - AC| <= max(|eAC|, |eBC|) * (|AC| + |B2|) / |B2 - AC|.
    //
    //  We know that B2 is always positive, if AC is negative, then there is no cancellation
    //  problem, and max(|eAC|, |eBC|) <= 2^-53, thus
    //
    //     2^-53 * (AC + B2) / |B2 - AC| <= 3 * 2^-53. Leading to
    //     AC + B2 <= 3 * |B2 - AC|.
    //
    // If 3 * |B2 - AC| >= AC + B2 holds, then the roughDiscriminant has 2-bits of rounding error
    // or less and can be used.
    if (3 * std::abs(roughDiscriminant) >= b2 + ac) {
        return roughDiscriminant;
    }

    // Use the extra internal precision afforded by fma to calculate the rounding error for
    // b^2 and ac.
    const double b2RoundingError = std::fma(b, b, -b2);
    const double acRoundingError = std::fma(a, c, -ac);

    // Add the total rounding error back into the discriminant guess.
    const double discriminant = (b2 - ac) + (b2RoundingError - acRoundingError);
    return discriminant;
}

SkQuads::RootResult SkQuads::Roots(double A, double B, double C) {
    const double discriminant = Discriminant(A, B, C);

    if (A == 0) {
        double root;
        if (B == 0) {
            if (C == 0) {
                root = std::numeric_limits<double>::infinity();
            } else {
                root = std::numeric_limits<double>::quiet_NaN();
            }
        } else {
            // Solve -2*B*x + C == 0; x = c/(2*b).
            root = C / (2 * B);
        }
        return {discriminant, root, root};
    }

    SkASSERT(A != 0);
    if (discriminant == 0) {
        return {discriminant, B / A, B / A};
    }

    if (discriminant > 0) {
        const double D = sqrt(discriminant);
        const double R = B > 0 ? B + D : B - D;
        return {discriminant, R / A, C / R};
    }

    // The discriminant is negative or is not finite.
    return {discriminant, NAN, NAN};
}

static double zero_if_tiny(double x) {
    return sk_double_nearly_zero(x) ? 0 : x;
}

int SkQuads::RootsReal(const double A, const double B, const double C, double solution[2]) {

    if (close_to_linear(A, B)) {
        return solve_linear(B, C, solution);
    }

    SkASSERT(A != 0);
    auto [discriminant, root0, root1] = Roots(A, -0.5 * B, C);

    // Handle invariants to mesh with existing code from here on.
    if (!std::isfinite(discriminant) || discriminant < 0) {
        return 0;
    }

    int roots = 0;
    if (const double r0 = zero_if_tiny(root0); std::isfinite(r0)) {
        solution[roots++] = r0;
    }
    if (const double r1 = zero_if_tiny(root1); std::isfinite(r1)) {
        solution[roots++] = r1;
    }

    if (roots == 2 && sk_doubles_nearly_equal_ulps(solution[0], solution[1])) {
        roots = 1;
    }

    return roots;
}

double SkQuads::EvalAt(double A, double B, double C, double t) {
    // Use fused-multiply-add to reduce the amount of round-off error between terms.
    return std::fma(std::fma(A, t, B), t, C);
}
