/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Curve.h"
#include "PortableMath.h"
#include "TransferFunction.h"
#include <assert.h>

static float minus_1_ulp(float x) {
    int32_t bits;
    memcpy(&bits, &x, sizeof(bits));
    bits = bits - 1;
    memcpy(&x, &bits, sizeof(bits));
    return x;
}

float skcms_eval_curve(const skcms_Curve* curve, float x) {
    if (curve->table_entries == 0) {
        return skcms_TransferFunction_eval(&curve->parametric, x);
    }

    float ix = fmaxf_(0, fminf_(x, 1)) * (curve->table_entries - 1);
    int   lo = (int)            ix,
          hi = (int)minus_1_ulp(ix + 1.0f);
    float t = ix - (float)lo;

    float l, h;
    if (curve->table_8) {
        l = curve->table_8[lo] * (1/255.0f);
        h = curve->table_8[hi] * (1/255.0f);
    } else {
        uint16_t be_l, be_h;
        memcpy(&be_l, curve->table_16 + 2*lo, 2);
        memcpy(&be_h, curve->table_16 + 2*hi, 2);
        uint16_t le_l = ((be_l << 8) | (be_l >> 8)) & 0xffff;
        uint16_t le_h = ((be_h << 8) | (be_h >> 8)) & 0xffff;
        l = le_l * (1/65535.0f);
        h = le_h * (1/65535.0f);
    }
    return l + (h-l)*t;
}
