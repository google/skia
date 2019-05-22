/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkYUVMath.h"
#include "include/core/SkMatrix44.h"

void SkColorMatrix_to_Matrix44(const float src[20], SkMatrix44* dst) {
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            dst->set(r, c, src[r*5 + c]);
        }
        dst->set(r, 3, src[r*5 + 4]);
    }
    dst->set(3, 0, 0);
    dst->set(3, 1, 0);
    dst->set(3, 2, 0);
    dst->set(3, 3, 1);
}

void SkMatrix44_to_ColorMatrix(const SkMatrix44& src, float dst[20]) {
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            dst[r*5 + c] = src.get(r, c);
        }
        dst[r*5 + 3] = 0;   // scale alpha
        dst[r*5 + 4] = src.get(r, 3);  // translate
    }
    dst[15] = dst[16] = dst[17] = dst[19] = 0; dst[18] = 1;
}

static void scale3(float m[], float s) {
    for (int i = 0; i < 3; ++i) {
        m[i] *= s;
    }
}

struct YUVCoeff {
    float   Kr, Kb;
    float   Cr, Cb;
    float   scaleY, addY;
    float   scaleUV;
};

void make_rgb_to_yuv_matrix(float mx[20], const YUVCoeff& c) {
    const float Kr = c.Kr;
    const float Kb = c.Kb;
    const float Kg = 1.0f - Kr - Kb;

    float m[20] = {
          Kr,  Kg,   Kb,  0,    c.addY,
         -Kr, -Kg, 1-Kb,  0, 128/255.f,
        1-Kr, -Kg,  -Kb,  0, 128/255.f,
           0,   0,    0,  1,         0,
    };
    memcpy(mx, m, sizeof(m));
    scale3(mx +  0, c.scaleY);
    scale3(mx +  5, c.Cr * c.scaleUV);
    scale3(mx + 10, c.Cb * c.scaleUV);
}

static void dump(const float m[20], SkYUVColorSpace cs, bool rgb2yuv) {
    const char* names[] = {
        "JPEG", "Rec601", "Rec709",
    };
    const char* dirnames[] = {
        "yuv_to_rgb", "rgb_to_yuv",
    };
    SkDebugf("const float %s_%s[] = {\n", names[cs], dirnames[rgb2yuv]);
    for (int i = 0; i < 4; ++i) {
        SkDebugf("    ");
        for (int j = 0; j < 5; ++j) {
            SkDebugf(" %9.6f,", m[i * 5 + j]);
        }
        SkDebugf("\n");
    }
    SkDebugf("};\n");
}

void dump(SkYUVColorSpace cs, const float m[20]) {
    dump(m, cs, true);
    SkMatrix44 m44, im44;
    SkColorMatrix_to_Matrix44(m, &m44);
    float im[20];
    if (1) {
        SkMatrix44_to_ColorMatrix(m44, im);
        SkASSERT(memcmp(m, im, sizeof(im)) == 0);
    }
    SkAssertResult(m44.invert(&im44));
    SkMatrix44_to_ColorMatrix(im44, im);
    dump(im, cs, false);
}

void SkColorMatrix_RGB2YUV(SkYUVColorSpace cs, float m[20]) {
    const YUVCoeff coeff[] = {
        { 0.299f,  0.114f,  1/1.772f,  1/1.402f,          1,        0,         1, }, // kJPEG_SkYUVColorSpace
        { 0.299f,  0.114f,  1/1.772f,  1/1.402f,  219/255.f, 16/255.f, 224/255.f, }, // kRec601_SkYUVColorSpace
        { 0.2126f, 0.0722f, 1/1.8556f, 1/1.5748f, 219/255.f, 16/255.f, 224/255.f, }, // kRec709_SkYUVColorSpace
    };
    if ((unsigned)cs < (unsigned)kIdentity_SkYUVColorSpace) {
        make_rgb_to_yuv_matrix(m, coeff[cs]);
        if (0) {
            dump(cs, m);
        }
    } else {
        memset(m, 0, 20 * sizeof(float));
        m[0] = m[6] = m[12] = m[18] = 1;
    }
}

void SkColorMatrix_YUV2RGB(SkYUVColorSpace cs, float m[20]) {
    SkMatrix44 m44, im44;
    SkColorMatrix_RGB2YUV(cs, m);
    SkColorMatrix_to_Matrix44(m, &m44);
    SkAssertResult(m44.invert(&im44));
    SkMatrix44_to_ColorMatrix(im44, m);
}

void SkMatrix44_RGB2YUV(SkYUVColorSpace cs, SkMatrix44* m44) {
    float m[20];
    SkColorMatrix_RGB2YUV(cs, m);
    SkColorMatrix_to_Matrix44(m, m44);
}

void SkMatrix44_YUV2RGB(SkYUVColorSpace cs, SkMatrix44* m44) {
    SkMatrix44 tmp;
    SkMatrix44_RGB2YUV(cs, &tmp);
    SkAssertResult(tmp.invert(m44));
}
