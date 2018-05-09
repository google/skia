/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <stdbool.h>

typedef struct { float vals[3]; } skcms_Vector3;

// It is _not_ safe to alias the pointers to invert in-place.
bool skcms_Matrix3x3_invert(const skcms_Matrix3x3*, skcms_Matrix3x3*);
skcms_Matrix3x3 skcms_Matrix3x3_concat(const skcms_Matrix3x3* A, const skcms_Matrix3x3* B);

skcms_Vector3 skcms_MV_mul(const skcms_Matrix3x3*, const skcms_Vector3*);
