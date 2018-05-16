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

float skcms_eval_curve(const skcms_Curve* curve, float x) {
    if (curve->table_entries == 0) {
        return skcms_TransferFunction_eval(&curve->parametric, x);
    }

    // TODO: today we should always hit an entry exactly, but if that changes, lerp?
    // (We add half to account for slight int -> float -> int round tripping issues.)
    float fx = x*(curve->table_entries - 1);
    int ix = (int)( fx + 0.5f );

    assert ( fabsf_(fx - (float)ix) < 0.0005 );

    if (curve->table_8) {
        return curve->table_8[ix] * (1/255.0f);
    } else {
        uint16_t be;
        memcpy(&be, curve->table_16 + 2*ix, 2);

        uint16_t le = ((be << 8) | (be >> 8)) & 0xffff;
        return le * (1/65535.0f);
    }
}
