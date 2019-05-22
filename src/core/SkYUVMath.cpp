/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkYUVMath.h"

static void scale3(float m[], float s) {
    for (int i = 0; i < 3; ++i) {
        m[i] *= s;
    }
}

struct YUVCoeff {
    float   Kr, Kb;
    float   scaleY, addY;
    float   scaleUV, addUV;
};

void make_rgb_to_yuv_matrix(float mx[20], const YUVCoeff& c) {
    const float Kr = c.Kr;
    const float Kb = c.Kb;
    const float Kg = 1.0f - Kr - Kb;
    const float Su = 1 / 1.8556f;
    const float Sv = 1 / 1.5748f;

    float m[20] = {
               Kr,     Kg,        Kb,  0,   c.addY,
           -Kr*Su, -Kg*Su, (1-Kb)*Su,  0,  c.addUV,
        (1-Kr)*Sv, -Kg*Sv,    -Kb*Sv,  0,  c.addUV,
                0,      0,         0,  1,        0,
    };
    memcpy(mx, m, sizeof(m));
    scale3(mx +  0, c.scaleY);
    scale3(mx +  5, c.scaleUV);
    scale3(mx + 10, c.scaleUV);
}

void SkMakeRGB2YUV(float m[20], SkYUVColorSpace cs) {
    YUVCoeff coeff[] = {
        { 0.299f,  0.114f,          1,        1,         0,         0 }, // kJPEG_SkYUVColorSpace
        { 0.299f,  0.114f,  219/255.f, 16/255.f, 224/255.f, 128/255.f }, // kRec601_SkYUVColorSpace
        { 0.2126f, 0.0722f, 219/255.f, 16/255.f, 224/255.f, 128/255.f }, // kRec709_SkYUVColorSpace
    };
    if ((unsigned)cs < (unsigned)kIdentity_SkYUVColorSpace) {
        make_rgb_to_yuv_matrix(m, coeff[cs]);
    } else {
        memset(m, 0, 20 * sizeof(float));
        m[0] = m[6] = m[12] = m[18] = 1;
    }
}
