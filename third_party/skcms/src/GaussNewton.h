/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <stdbool.h>

// One Gauss-Newton step, tuning up to 3 parameters P to minimize [ r(x,ctx) ]^2.
//
//   rg:       residual function r(x,P) to minimize, and gradient at x in dfdP
//   ctx:      arbitrary context argument passed to rg
//   P:        in-out, both your initial guess for parameters of r(), and our updated values
//   x0,x1,N:  N x-values to test in [x0,x1] (both inclusive) with even spacing
//
// If you have fewer than 3 parameters, set the unused P to zero, don't touch their dfdP.
//
// Returns true and updates P on success, or returns false on failure.
bool skcms_gauss_newton_step(float (*rg)(float x, const void*, const float P[3], float dfdP[3]),
                             const void* ctx,
                             float P[3],
                             float x0, float x1, int N);
