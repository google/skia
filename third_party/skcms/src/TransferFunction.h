/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

// TransferFunction.h contains skcms-private APIs for working with skcms_TransferFunction.

#include <stdbool.h>

float skcms_TransferFunction_eval(const skcms_TransferFunction*, float);

bool skcms_TransferFunction_invert(const skcms_TransferFunction*, skcms_TransferFunction*);

// Fit c,d,f parameters of an skcms_TransferFunction to the first 2 < L ≤ N
// evenly-spaced points on an skcms_Curve within a given tolerance, returning L.
int skcms_fit_linear(const skcms_Curve*, int N, float tol, skcms_TransferFunction*);
