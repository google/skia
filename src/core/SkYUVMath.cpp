/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMatrix44.h"
#include "src/core/SkYUVMath.h"

// in SkColorMatrix order (row-major)
// Created by running SkColorMatrix_DumpYUVMatrixTables()

const float Rec709_rgb_to_yuv[] = {
    0.182586f,  0.614231f,  0.062007f,  0.000000f,  0.062745f,
   -0.100644f, -0.338572f,  0.439216f,  0.000000f,  0.501961f,
    0.439216f, -0.398942f, -0.040274f,  0.000000f,  0.501961f,
    0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float Rec709_yuv_to_rgb[] = {
    1.164384f,  0.000000f,  1.792741f,  0.000000f, -0.972945f,
    1.164384f, -0.213249f, -0.532909f,  0.000000f,  0.301483f,
    1.164384f,  2.112402f,  0.000000f,  0.000000f, -1.133402f,
    0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float Rec601_rgb_to_yuv[] = {
    0.256788f,  0.504129f,  0.097906f,  0.000000f,  0.062745f,
   -0.148223f, -0.290993f,  0.439216f,  0.000000f,  0.501961f,
    0.439216f, -0.367788f, -0.071427f,  0.000000f,  0.501961f,
    0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float Rec601_yuv_to_rgb[] = {
    1.164384f,  0.000000f,  1.596027f,  0.000000f, -0.874202f,
    1.164384f, -0.391762f, -0.812968f,  0.000000f,  0.531668f,
    1.164384f,  2.017232f,  0.000000f,  0.000000f, -1.085631f,
    0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float JPEG_rgb_to_yuv[] = {
    0.299000f,  0.587000f,  0.114000f,  0.000000f,  0.000000f,
   -0.168736f, -0.331264f,  0.500000f,  0.000000f,  0.501961f,
    0.500000f, -0.418688f, -0.081312f,  0.000000f,  0.501961f,
    0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float JPEG_yuv_to_rgb[] = {
    1.000000f,  0.000000f,  1.402000f,  0.000000f, -0.703749f,
    1.000000f, -0.344136f, -0.714136f,  0.000000f,  0.531211f,
    1.000000f,  1.772000f,  0.000000f,  0.000000f, -0.889475f,
    0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};

static_assert(kJPEG_SkYUVColorSpace   == 0, "");
static_assert(kRec601_SkYUVColorSpace == 1, "");
static_assert(kRec709_SkYUVColorSpace == 2, "");

const float* yuv_to_rgb_array[] = {
    JPEG_yuv_to_rgb,
    Rec601_yuv_to_rgb,
    Rec709_yuv_to_rgb,
};

const float* rgb_to_yuv_array[] = {
    JPEG_rgb_to_yuv,
    Rec601_rgb_to_yuv,
    Rec709_rgb_to_yuv,
};

constexpr size_t kSizeOfColorMatrix = 20 * sizeof(float);

void SkColorMatrix_RGB2YUV(SkYUVColorSpace cs, float m[20]) {
    if ((unsigned)cs < (unsigned)kIdentity_SkYUVColorSpace) {
        memcpy(m, rgb_to_yuv_array[(unsigned)cs], kSizeOfColorMatrix);
    } else {
        memset(m, 0, kSizeOfColorMatrix);
        m[0] = m[6] = m[12] = m[18] = 1;
    }
}

void SkColorMatrix_YUV2RGB(SkYUVColorSpace cs, float m[20]) {
    if ((unsigned)cs < (unsigned)kIdentity_SkYUVColorSpace) {
        memcpy(m, yuv_to_rgb_array[(unsigned)cs], kSizeOfColorMatrix);
    } else {
        memset(m, 0, kSizeOfColorMatrix);
        m[0] = m[6] = m[12] = m[18] = 1;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// we just drop the alpha rol/col from the colormatrix
// output is |        tr |
//           |  3x3   tg |
//           |        tb |
//           | 0 0 0  1  |
static void colormatrix_to_matrix44(const float src[20], SkMatrix44* dst) {
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

// input: ignore the bottom row
// output: inject identity row/column for alpha
static void matrix44_to_colormatrix(const SkMatrix44& src, float dst[20]) {
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            dst[r*5 + c] = src.get(r, c);
        }
        dst[r*5 + 3] = 0;   // scale alpha
        dst[r*5 + 4] = src.get(r, 3);  // translate
    }
    dst[15] = dst[16] = dst[17] = dst[19] = 0;
    dst[18] = 1;
}

static void scale3(float m[], float s) {
    for (int i = 0; i < 3; ++i) {
        m[i] *= s;
    }
}

namespace {
struct YUVCoeff {
    float   Kr, Kb;
    float   Cr, Cb;
    float   scaleY, addY;
    float   scaleUV;
};
}   // namespace

const YUVCoeff gCoeff[] = {
    // kJPEG_SkYUVColorSpace
    { 0.299f,  0.114f,  1/1.772f,  1/1.402f,          1,        0,         1, },

    // kRec601_SkYUVColorSpace
    { 0.299f,  0.114f,  1/1.772f,  1/1.402f,  219/255.f, 16/255.f, 224/255.f, },

    // kRec709_SkYUVColorSpace
    { 0.2126f, 0.0722f, 1/1.8556f, 1/1.5748f, 219/255.f, 16/255.f, 224/255.f, },
};

static void make_rgb_to_yuv_matrix(float mx[20], const YUVCoeff& c) {
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
            SkDebugf(" %9.6ff,", m[i * 5 + j]);
        }
        SkDebugf("\n");
    }
    SkDebugf("};\n");
}

// Used to create the prebuilt tables for each colorspace.
// Don't remove this function, in case we want to recompute those tables in the future.
void SkColorMatrix_DumpYUVMatrixTables() {
    for (auto cs : {kRec709_SkYUVColorSpace, kRec601_SkYUVColorSpace, kJPEG_SkYUVColorSpace}) {
        float m[20];
        make_rgb_to_yuv_matrix(m, gCoeff[(unsigned)cs]);
        dump(m, cs, true);
        SkMatrix44 m44, im44;
        colormatrix_to_matrix44(m, &m44);
        float im[20];
#ifdef SK_DEBUG
        // be sure our coversion between matrix44 and colormatrix is perfect
        matrix44_to_colormatrix(m44, im);
        SkASSERT(memcmp(m, im, sizeof(im)) == 0);
#endif
        SkAssertResult(m44.invert(&im44));
        matrix44_to_colormatrix(im44, im);
        dump(im, cs, false);
    }
}
