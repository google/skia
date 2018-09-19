/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/Sk3D.h"

static void set_col(SkMatrix44* m, int col, const SkPoint3& v) {
    m->set(0, col, v.fX);
    m->set(1, col, v.fY);
    m->set(2, col, v.fZ);
}

static SkPoint3 cross(const SkPoint3& a, const SkPoint3& b) {
    return {
        a.fY * b.fZ - a.fZ * b.fY,
        a.fZ * b.fX - a.fX * b.fZ,
        a.fX * b.fY - a.fY * b.fX,
    };
}

void Sk3LookAt(SkMatrix44* dst, const SkPoint3& eye, const SkPoint3& center, const SkPoint3& up) {
    SkPoint3 f = center - eye;
    f.normalize();
    SkPoint3 u = up;
    u.normalize();
    SkPoint3 s = cross(f, u);
    s.normalize();
    u = cross(s, f);

    dst->setIdentity();
    set_col(dst, 0, s);
    set_col(dst, 1, u);
    set_col(dst, 2, -f);
    set_col(dst, 3, eye);
    dst->invert(dst);
}

bool Sk3Perspective(SkMatrix44* dst, float near, float far, float angle) {
    SkASSERT(far > near);

    float denomInv = sk_ieee_float_divide(1, far - near);
    float halfAngle = angle * 0.5f;
    float cot = sk_float_cos(halfAngle) / sk_float_sin(halfAngle);

    dst->setIdentity();
    dst->set(0, 0, cot);
    dst->set(1, 1, cot);
    dst->set(2, 2, (far + near) * denomInv);
    dst->set(2, 3, 2 * far * near * denomInv);
    dst->set(3, 2, -1);
    return true;
}

void Sk3MapPts(SkPoint dst[], const SkMatrix44& m4, const SkPoint3 src[], int count) {
    for (int i = 0; i < count; ++i) {
        SkVector4 v = m4 * SkVector4{ src[i].fX, src[i].fY, src[i].fZ, 1 };
        // clip v;
        dst[i] = { v.fData[0] / v.fData[3], v.fData[1] / v.fData[3] };
    }
}

