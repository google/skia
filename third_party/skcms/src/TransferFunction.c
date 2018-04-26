/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../skcms.h"
#include "GaussNewton.h"
#include "LinearAlgebra.h"
#include "Macros.h"
#include "PortableMath.h"
#include "TransferFunction.h"
#include <assert.h>
#include <limits.h>
#include <string.h>

float skcms_TransferFunction_eval(const skcms_TransferFunction* tf, float x) {
    float sign = x < 0 ? -1.0f : 1.0f;
    x *= sign;

    return sign * (x < tf->d ? tf->c * x + tf->f
                             : powf_(tf->a * x + tf->b, tf->g) + tf->e);
}

bool skcms_TransferFunction_isValid(const skcms_TransferFunction* tf) {
    // Reject obviously malformed inputs
    if (!isfinitef_(tf->a + tf->b + tf->c + tf->d + tf->e + tf->f + tf->g)) {
        return false;
    }

    // All of these parameters should be non-negative
    if (tf->a < 0 || tf->c < 0 || tf->d < 0 || tf->g < 0) {
        return false;
    }

    return true;
}

// TODO: Adjust logic here? This still assumes that purely linear inputs will have D > 1, which
// we never generate. It also emits inverted linear using the same formulation. Standardize on
// G == 1 here, too?
bool skcms_TransferFunction_invert(const skcms_TransferFunction* src, skcms_TransferFunction* dst) {
    // Original equation is:       y = (ax + b)^g + e   for x >= d
    //                             y = cx + f           otherwise
    //
    // so 1st inverse is:          (y - e)^(1/g) = ax + b
    //                             x = ((y - e)^(1/g) - b) / a
    //
    // which can be re-written as: x = (1/a)(y - e)^(1/g) - b/a
    //                             x = ((1/a)^g)^(1/g) * (y - e)^(1/g) - b/a
    //                             x = ([(1/a)^g]y + [-((1/a)^g)e]) ^ [1/g] + [-b/a]
    //
    // and 2nd inverse is:         x = (y - f) / c
    // which can be re-written as: x = [1/c]y + [-f/c]
    //
    // and now both can be expressed in terms of the same parametric form as the
    // original - parameters are enclosed in square brackets.
    skcms_TransferFunction tf_inv = { 0, 0, 0, 0, 0, 0, 0 };

    // This rejects obviously malformed inputs, as well as decreasing functions
    if (!skcms_TransferFunction_isValid(src)) {
        return false;
    }

    // There are additional constraints to be invertible
    bool has_nonlinear = (src->d <= 1);
    bool has_linear = (src->d > 0);

    // Is the linear section not invertible?
    if (has_linear && src->c == 0) {
        return false;
    }

    // Is the nonlinear section not invertible?
    if (has_nonlinear && (src->a == 0 || src->g == 0)) {
        return false;
    }

    // If both segments are present, they need to line up
    if (has_linear && has_nonlinear) {
        float l_at_d = src->c * src->d + src->f;
        float n_at_d = powf_(src->a * src->d + src->b, src->g) + src->e;
        if (fabsf_(l_at_d - n_at_d) > (1 / 512.0f)) {
            return false;
        }
    }

    // Invert linear segment
    if (has_linear) {
        tf_inv.c = 1.0f / src->c;
        tf_inv.f = -src->f / src->c;
    }

    // Invert nonlinear segment
    if (has_nonlinear) {
        tf_inv.g = 1.0f / src->g;
        tf_inv.a = powf_(1.0f / src->a, src->g);
        tf_inv.b = -tf_inv.a * src->e;
        tf_inv.e = -src->b / src->a;
    }

    if (!has_linear) {
        tf_inv.d = 0;
    } else if (!has_nonlinear) {
        // Any value larger than 1 works
        tf_inv.d = 2.0f;
    } else {
        tf_inv.d = src->c * src->d + src->f;
    }

    *dst = tf_inv;
    return true;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

// From here below we're approximating an skcms_Curve with an skcms_TransferFunction{g,a,b,c,d,e,f}:
//
//   tf(x) =  cx + f          x < d
//   tf(x) = (ax + b)^g + e   x ≥ d
//
// When fitting, we add the additional constraint that both pieces meet at d:
//
//   cd + f = (ad + b)^g + e
//
// Solving for e and folding it through gives an alternate formulation of the non-linear piece:
//
//   tf(x) =                           cx + f   x < d
//   tf(x) = (ax + b)^g - (ad + b)^g + cd + f   x ≥ d
//
// Our overall strategy is then:
//    For a couple tolerances,
//       - skcms_fit_linear(): fit c,d,f iteratively to as many points as our tolerance allows
//       - fit_nonlinear():    fit g,a,b using Gauss-Newton given c,d,f (and by constraint, e)
//    Return the parameters with least maximum error.
//
// To run Gauss-Newton to find g,a,b, we'll also need the gradient of the non-linear piece:
//    ∂tf/∂g = ln(ax + b)*(ax + b)^g
//           - ln(ad + b)*(ad + b)^g
//    ∂tf/∂a = xg(ax + b)^(g-1)
//           - dg(ad + b)^(g-1)
//    ∂tf/∂b =  g(ax + b)^(g-1)
//           -  g(ad + b)^(g-1)

static float eval_nonlinear(float x, const void* ctx, const float P[3]) {
    const skcms_TransferFunction* tf = (const skcms_TransferFunction*)ctx;

    const float g = P[0],  a = P[1],  b = P[2],
                c = tf->c, d = tf->d, f = tf->f;

    const float X = a*x+b,
                D = a*d+b;
    assert (X >= 0 && D >= 0);

    // (Notice how a large amount of this work is independent of x.)
    return powf_(X, g)
         - powf_(D, g)
         + c*d + f;
}
static void grad_nonlinear(float x, const void* ctx, const float P[3], float dfdP[3]) {
    const skcms_TransferFunction* tf = (const skcms_TransferFunction*)ctx;

    const float g = P[0], a = P[1], b = P[2],
                d = tf->d;

    const float X = a*x+b,
                D = a*d+b;
    assert (X >= 0 && D >= 0);

    dfdP[0] = 0.69314718f*log2f_(X)*powf_(X, g)
            - 0.69314718f*log2f_(D)*powf_(D, g);
    dfdP[1] = x*g*powf_(X, g-1)
            - d*g*powf_(D, g-1);
    dfdP[2] =   g*powf_(X, g-1)
            -   g*powf_(D, g-1);
}

int skcms_fit_linear(const skcms_Curve* curve, int N, float tol, float* c, float* d, float* f) {
    assert(N > 1);
    // We iteratively fit the first points to the TF's linear piece.
    // We want the cx + f line to pass through the first and last points we fit exactly.
    //
    // As we walk along the points we find the minimum and maximum slope of the line before the
    // error would exceed our tolerance.  We stop when the range [slope_min, slope_max] becomes
    // emtpy, when we definitely can't add any more points.
    //
    // Some points' error intervals may intersect the running interval but not lie fully
    // within it.  So we keep track of the last point we saw that is a valid end point candidate,
    // and once the search is done, back up to build the line through *that* point.
    const float x_scale = 1.0f / (N - 1);

    int lin_points = 1;
    *f = skcms_eval_curve(0, curve);

    float slope_min = -INFINITY_;
    float slope_max = +INFINITY_;
    for (int i = 1; i < N; ++i) {
        float x = i * x_scale;
        float y = skcms_eval_curve(x, curve);

        float slope_max_i = (y + tol - *f) / x,
              slope_min_i = (y - tol - *f) / x;
        if (slope_max_i < slope_min || slope_max < slope_min_i) {
            // Slope intervals would no longer overlap.
            break;
        }
        slope_max = fminf_(slope_max, slope_max_i);
        slope_min = fmaxf_(slope_min, slope_min_i);

        float cur_slope = (y - *f) / x;
        if (slope_min <= cur_slope && cur_slope <= slope_max) {
            lin_points = i + 1;
            *c = cur_slope;
        }
    }

    // Set D to the last point that met our tolerance.
    *d = (lin_points - 1) * x_scale;
    return lin_points;
}

// Fit the points in [start,N) to the non-linear piece of tf, or return false if we can't.
static bool fit_nonlinear(const skcms_Curve* curve, int start, int N, skcms_TransferFunction* tf) {
    float P[3] = { tf->g, tf->a, tf->b };

    // No matter where we start, x_scale should always represent N even steps from 0 to 1.
    const float x_scale = 1.0f / (N-1);

    for (int j = 0; j < 3/*TODO: tune*/; j++) {
        // These extra constraints a >= 0 and ad+b >= 0 are not modeled in the optimization.
        assert (P[1] >= 0 &&
                P[1] * tf->d + P[2] >= 0);

        if (!skcms_gauss_newton_step(skcms_eval_curve, curve,
                                     eval_nonlinear, tf,
                                     grad_nonlinear, tf,
                                     P,
                                     start*x_scale, 1, N-start)) {
            return false;
        }

        // We don't really know how to fix up a if it goes negative.
        if (P[1] < 0) {
            return false;
        }
        // If ad+b goes negative, we feel just barely not uneasy enough to tweak b so ad+b is zero.
        if (P[1] * tf->d + P[2] < 0) {
            P[2] = -P[1] * tf->d;
        }
    }

    tf->g = P[0];
    tf->a = P[1];
    tf->b = P[2];
    tf->e =   tf->c*tf->d + tf->f
      - powf_(tf->a*tf->d + tf->b, tf->g);
    return true;
}

bool skcms_ApproximateCurve(const skcms_Curve* curve,
                            skcms_TransferFunction* approx,
                            float* max_error) {
    if (!curve || !approx || !max_error) {
        return false;
    }

    if (curve->table_entries == 0) {
        // No point approximating an skcms_TransferFunction with an skcms_TransferFunction!
        return false;
    }

    if (curve->table_entries == 1 || curve->table_entries > (uint32_t)INT_MAX) {
        // We need at least two points, and must put some reasonable cap on the maximum number.
        return false;
    }

    int N = (int)curve->table_entries;
    const float x_scale = 1.0f / (N - 1);

    *max_error = INFINITY_;
    const float kTolerances[] = { 1.5f / 65535.0f, 1.0f / 512.0f };
    for (int t = 0; t < ARRAY_COUNT(kTolerances); t++) {
        skcms_TransferFunction tf;
        int L = skcms_fit_linear(curve, N, kTolerances[t], &tf.c, &tf.d, &tf.f);

        if (L == N) {
            // If the entire data set was linear, move the coefficients to the nonlinear portion
            // with G == 1.  This lets use a canonical representation with d == 0.
            tf.g = 1;
            tf.a = tf.c;
            tf.b = tf.f;
            tf.c = tf.d = tf.e = tf.f = 0;
        } else if (L == N - 1) {
            // Degenerate case with only two points in the nonlinear segment. Solve directly.
            tf.g = 1;
            tf.a = (skcms_eval_curve((N-1)*x_scale, curve) - skcms_eval_curve((N-2)*x_scale, curve))
                 * (N-1);
            tf.b = skcms_eval_curve((N-1)*x_scale, curve)
                 - tf.a * (N-2) * x_scale;
            tf.e = 0;
        } else {
            // Start by guessing a gamma-only curve through the midpoint.
            int mid = (L + N) / 2;
            float mid_x = mid / (N - 1.0f);
            float mid_y = skcms_eval_curve(mid_x, curve);
            tf.g = log2f_(mid_y) / log2f_(mid_x);;
            tf.a = 1;
            tf.b = 0;

            if (!fit_nonlinear(curve, L,N, &tf)) {
                continue;
            }
        }

        // We want to calculate the error of this approximation now.
        // The most likely use case for this approximation is to be inverted and
        // used as the transfer function for a destination color space, so we'll
        // exercise that scenario here.
        skcms_TransferFunction inv;
        if (!skcms_TransferFunction_invert(&tf, &inv)) {
            // If we can't invert the transfer function, it's not of much practical use anyway.
            continue;
        }

        float err = 0;
        for (int i = 0; i < N; i++) {
            float x = i * x_scale,
                  y = skcms_eval_curve(x, curve);
            err = fmaxf_(err, fabsf_(x - skcms_TransferFunction_eval(&inv, y)));
        }
        if (*max_error > err) {
            *max_error = err;
            *approx    = tf;
        }
    }
    return isfinitef_(*max_error);
}
