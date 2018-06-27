/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <stdbool.h>

// A row-major 4x4 matrix (ie vals[row][col])
typedef struct { float vals[4][4]; } skcms_Matrix4x4;
// (skcms_Matrix3x3 is in skcms.h, exactly analogous to skcms_Matrix4x4.)

typedef struct { float vals[4]; } skcms_Vector4;


// It is _not_ safe to alias the pointers to invert in-place.
bool skcms_Matrix4x4_invert(const skcms_Matrix4x4*, skcms_Matrix4x4*);
bool skcms_Matrix3x3_invert(const skcms_Matrix3x3*, skcms_Matrix3x3*);

skcms_Vector4 skcms_Matrix4x4_Vector4_mul(const skcms_Matrix4x4*, const skcms_Vector4*);
