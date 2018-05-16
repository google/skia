/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../skcms.h"
#include "GaussNewton.h"
#include "LinearAlgebra.h"
#include "PortableMath.h"
#include "TransferFunction.h"
#include <assert.h>
#include <string.h>

bool skcms_gauss_newton_step(float (*rg)(float x, const void*, const float P[3], float dfdP[3]),
                             const void* ctx,
                             float P[3],
                             float x0, float x1, int N) {
    // We'll sample x from the range [x0,x1] (both inclusive) N times with even spacing.
    //
    // We want to do P' = P + (Jf^T Jf)^-1 Jf^T r(P),
    //   where r(P) is the residual vector
    //   and Jf is the Jacobian matrix of f(), ∂r/∂P.
    //
    // Let's review the shape of each of these expressions:
    //   r(P)   is [N x 1], a column vector with one entry per value of x tested
    //   Jf     is [N x 3], a matrix with an entry for each (x,P) pair
    //   Jf^T   is [3 x N], the transpose of Jf
    //
    //   Jf^T Jf   is [3 x N] * [N x 3] == [3 x 3], a 3x3 matrix,
    //                                              and so is its inverse (Jf^T Jf)^-1
    //   Jf^T r(P) is [3 x N] * [N x 1] == [3 x 1], a column vector with the same shape as P
    //
    // Our implementation strategy to get to the final ∆P is
    //   1) evaluate Jf^T Jf,   call that lhs
    //   2) evaluate Jf^T r(P), call that rhs
    //   3) invert lhs
    //   4) multiply inverse lhs by rhs
    //
    // This is a friendly implementation strategy because we don't have to have any
    // buffers that scale with N, and equally nice don't have to perform any matrix
    // operations that are variable size.
    //
    // Other implementation strategies could trade this off, e.g. evaluating the
    // pseudoinverse of Jf ( (Jf^T Jf)^-1 Jf^T ) directly, then multiplying that by
    // the residuals.  That would probably require implementing singular value
    // decomposition, and would create a [3 x N] matrix to be multiplied by the
    // [N x 1] residual vector, but on the upside I think that'd eliminate the
    // possibility of this skcms_gauss_newton_step() function ever failing.

    // 0) start off with lhs and rhs safely zeroed.
    skcms_Matrix3x3 lhs = {{ {0,0,0}, {0,0,0}, {0,0,0} }};
    skcms_Vector3   rhs = {  {0,0,0} };

    // 1,2) evaluate lhs and evaluate rhs
    //   We want to evaluate Jf only once, but both lhs and rhs involve Jf^T,
    //   so we'll have to update lhs and rhs at the same time.
    float dx = (x1-x0)/(N-1);
    for (int i = 0; i < N; i++) {
        float x = x0 + i*dx;

        float dfdP[3] = {0,0,0};
        float resid = rg(x,ctx,P, dfdP);

        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 3; c++) {
                lhs.vals[r][c] += dfdP[r] * dfdP[c];
            }
            rhs.vals[r] += dfdP[r] * resid;
        }
    }

    // If any of the 3 P parameters are unused, this matrix will be singular.
    // Detect those cases and fix them up to indentity instead, so we can invert.
    for (int k = 0; k < 3; k++) {
        if (lhs.vals[0][k]==0 && lhs.vals[1][k]==0 && lhs.vals[2][k]==0 &&
            lhs.vals[k][0]==0 && lhs.vals[k][1]==0 && lhs.vals[k][2]==0) {
            lhs.vals[k][k] = 1;
        }
    }

    // 3) invert lhs
    skcms_Matrix3x3 lhs_inv;
    if (!skcms_Matrix3x3_invert(&lhs, &lhs_inv)) {
        return false;
    }

    // 4) multiply inverse lhs by rhs
    skcms_Vector3 dP = skcms_MV_mul(&lhs_inv, &rhs);
    P[0] += dP.vals[0];
    P[1] += dP.vals[1];
    P[2] += dP.vals[2];
    return isfinitef_(P[0]) && isfinitef_(P[1]) && isfinitef_(P[2]);
}
