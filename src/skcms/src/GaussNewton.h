/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <stdbool.h>
#include "LinearAlgebra.h"

// One Gauss-Newton step, tuning up to 4 parameters P to minimize [ t(x,ctx) - f(x,P) ]^2.
//
//   t:        target function of x to approximate
//   t_ctx:    any context needed for t, passed blindly into calls to t()
//   f:        function of x,P we're tuning to match t()
//   grad_f:   gradient of f() at x
//   P:        in-out, both your initial guess for parameters of f(), and our updated values
//   x0,x1,N:  N x-values to test in [x0,x1] (both inclusive) with even spacing
//
// If you have fewer than 4 parameters, set the unused P to zero, don't touch their dfdP.
//
// Returns true and updates P on success, or returns false on failure.
bool skcms_gauss_newton_step(float (*     t)(float x, const void*), const void* t_ctx,
                             float (*     f)(float x, const float P[4]),
                             void  (*grad_f)(float x, const float P[4], float dfdP[4]),
                             float P[4],
                             float x0, float x1, int N);
