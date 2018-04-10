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

typedef float skcms_TableFunc(int, const void*);
bool skcms_TransferFunction_approximate(skcms_TableFunc* t, const void* ctx, int n,
                                        skcms_TransferFunction*, float* max_error);
