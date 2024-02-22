/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkYUVMath.h"

#include "include/core/SkImageInfo.h"
#include "include/core/SkM44.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"

#include <cstring>

// in SkColorMatrix order (row-major)
// Created by running SkColorMatrix_DumpYUVMatrixTables()
const float JPEG_full_rgb_to_yuv[] = {
      0.299000f,  0.587000f,  0.114000f,  0.000000f,  0.000000f,
     -0.168736f, -0.331264f,  0.500000f,  0.000000f,  0.501961f,
      0.500000f, -0.418688f, -0.081312f,  0.000000f,  0.501961f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float JPEG_full_yuv_to_rgb[] = {
      1.000000f, -0.000000f,  1.402000f,  0.000000f, -0.703749f,
      1.000000f, -0.344136f, -0.714136f,  0.000000f,  0.531211f,
      1.000000f,  1.772000f,  0.000000f,  0.000000f, -0.889475f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float Rec601_limited_rgb_to_yuv[] = {
      0.256788f,  0.504129f,  0.097906f,  0.000000f,  0.062745f,
     -0.148223f, -0.290993f,  0.439216f,  0.000000f,  0.501961f,
      0.439216f, -0.367788f, -0.071427f,  0.000000f,  0.501961f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float Rec601_limited_yuv_to_rgb[] = {
      1.164384f, -0.000000f,  1.596027f,  0.000000f, -0.874202f,
      1.164384f, -0.391762f, -0.812968f,  0.000000f,  0.531668f,
      1.164384f,  2.017232f,  0.000000f,  0.000000f, -1.085631f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float Rec709_full_rgb_to_yuv[] = {
      0.212600f,  0.715200f,  0.072200f,  0.000000f,  0.000000f,
     -0.114572f, -0.385428f,  0.500000f,  0.000000f,  0.501961f,
      0.500000f, -0.454153f, -0.045847f,  0.000000f,  0.501961f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float Rec709_full_yuv_to_rgb[] = {
      1.000000f, -0.000000f,  1.574800f,  0.000000f, -0.790488f,
      1.000000f, -0.187324f, -0.468124f,  0.000000f,  0.329010f,
      1.000000f,  1.855600f, -0.000000f,  0.000000f, -0.931439f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float Rec709_limited_rgb_to_yuv[] = {
      0.182586f,  0.614231f,  0.062007f,  0.000000f,  0.062745f,
     -0.100644f, -0.338572f,  0.439216f,  0.000000f,  0.501961f,
      0.439216f, -0.398942f, -0.040274f,  0.000000f,  0.501961f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float Rec709_limited_yuv_to_rgb[] = {
      1.164384f, -0.000000f,  1.792741f,  0.000000f, -0.972945f,
      1.164384f, -0.213249f, -0.532909f,  0.000000f,  0.301483f,
      1.164384f,  2.112402f, -0.000000f,  0.000000f, -1.133402f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_8bit_full_rgb_to_yuv[] = {
      0.262700f,  0.678000f,  0.059300f,  0.000000f,  0.000000f,
     -0.139630f, -0.360370f,  0.500000f,  0.000000f,  0.501961f,
      0.500000f, -0.459786f, -0.040214f,  0.000000f,  0.501961f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_8bit_full_yuv_to_rgb[] = {
      1.000000f, -0.000000f,  1.474600f,  0.000000f, -0.740191f,
      1.000000f, -0.164553f, -0.571353f,  0.000000f,  0.369396f,
      1.000000f,  1.881400f, -0.000000f,  0.000000f, -0.944389f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_8bit_limited_rgb_to_yuv[] = {
      0.225613f,  0.582282f,  0.050928f,  0.000000f,  0.062745f,
     -0.122655f, -0.316560f,  0.439216f,  0.000000f,  0.501961f,
      0.439216f, -0.403890f, -0.035326f,  0.000000f,  0.501961f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_8bit_limited_yuv_to_rgb[] = {
      1.164384f, -0.000000f,  1.678674f,  0.000000f, -0.915688f,
      1.164384f, -0.187326f, -0.650424f,  0.000000f,  0.347458f,
      1.164384f,  2.141772f, -0.000000f,  0.000000f, -1.148145f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_10bit_full_rgb_to_yuv[] = {
      0.262700f,  0.678000f,  0.059300f,  0.000000f,  0.000000f,
     -0.139630f, -0.360370f,  0.500000f,  0.000000f,  0.500489f,
      0.500000f, -0.459786f, -0.040214f,  0.000000f,  0.500489f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_10bit_full_yuv_to_rgb[] = {
      1.000000f, -0.000000f,  1.474600f,  0.000000f, -0.738021f,
      1.000000f, -0.164553f, -0.571353f,  0.000000f,  0.368313f,
      1.000000f,  1.881400f, -0.000000f,  0.000000f, -0.941620f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_10bit_limited_rgb_to_yuv[] = {
      0.224951f,  0.580575f,  0.050779f,  0.000000f,  0.062561f,
     -0.122296f, -0.315632f,  0.437928f,  0.000000f,  0.500489f,
      0.437928f, -0.402706f, -0.035222f,  0.000000f,  0.500489f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_10bit_limited_yuv_to_rgb[] = {
      1.167808f, -0.000000f,  1.683611f,  0.000000f, -0.915688f,
      1.167808f, -0.187877f, -0.652337f,  0.000000f,  0.347458f,
      1.167808f,  2.148072f, -0.000000f,  0.000000f, -1.148145f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_12bit_full_rgb_to_yuv[] = {
      0.262700f,  0.678000f,  0.059300f,  0.000000f,  0.000000f,
     -0.139630f, -0.360370f,  0.500000f,  0.000000f,  0.500122f,
      0.500000f, -0.459786f, -0.040214f,  0.000000f,  0.500122f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_12bit_full_yuv_to_rgb[] = {
      1.000000f, -0.000000f,  1.474600f,  0.000000f, -0.737480f,
      1.000000f, -0.164553f, -0.571353f,  0.000000f,  0.368043f,
      1.000000f,  1.881400f, -0.000000f,  0.000000f, -0.940930f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_12bit_limited_rgb_to_yuv[] = {
      0.224787f,  0.580149f,  0.050742f,  0.000000f,  0.062515f,
     -0.122206f, -0.315401f,  0.437607f,  0.000000f,  0.500122f,
      0.437607f, -0.402411f, -0.035196f,  0.000000f,  0.500122f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_12bit_limited_yuv_to_rgb[] = {
      1.168664f, -0.000000f,  1.684846f,  0.000000f, -0.915688f,
      1.168664f, -0.188015f, -0.652816f,  0.000000f,  0.347458f,
      1.168664f,  2.149647f, -0.000000f,  0.000000f, -1.148145f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};

static_assert(kJPEG_Full_SkYUVColorSpace            == 0, "");
static_assert(kRec601_Limited_SkYUVColorSpace       == 1, "");
static_assert(kRec709_Full_SkYUVColorSpace          == 2, "");
static_assert(kRec709_Limited_SkYUVColorSpace       == 3, "");
static_assert(kBT2020_8bit_Full_SkYUVColorSpace     == 4, "");
static_assert(kBT2020_8bit_Limited_SkYUVColorSpace  == 5, "");
static_assert(kBT2020_10bit_Full_SkYUVColorSpace    == 6, "");
static_assert(kBT2020_10bit_Limited_SkYUVColorSpace == 7, "");
static_assert(kBT2020_12bit_Full_SkYUVColorSpace    == 8, "");
static_assert(kBT2020_12bit_Limited_SkYUVColorSpace == 9, "");

const float* yuv_to_rgb_array[] = {
    JPEG_full_yuv_to_rgb,
    Rec601_limited_yuv_to_rgb,
    Rec709_full_yuv_to_rgb,
    Rec709_limited_yuv_to_rgb,
    BT2020_8bit_full_yuv_to_rgb,
    BT2020_8bit_limited_yuv_to_rgb,
    BT2020_10bit_full_yuv_to_rgb,
    BT2020_10bit_limited_yuv_to_rgb,
    BT2020_12bit_full_yuv_to_rgb,
    BT2020_12bit_limited_yuv_to_rgb,
};

const float* rgb_to_yuv_array[] = {
    JPEG_full_rgb_to_yuv,
    Rec601_limited_rgb_to_yuv,
    Rec709_full_rgb_to_yuv,
    Rec709_limited_rgb_to_yuv,
    BT2020_8bit_full_rgb_to_yuv,
    BT2020_8bit_limited_rgb_to_yuv,
    BT2020_10bit_full_rgb_to_yuv,
    BT2020_10bit_limited_rgb_to_yuv,
    BT2020_12bit_full_rgb_to_yuv,
    BT2020_12bit_limited_rgb_to_yuv,
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
static void colormatrix_to_matrix44(const float src[20], SkM44* dst) {
    *dst = SkM44(src[ 0], src[ 1], src[ 2], src[ 4],
                 src[ 5], src[ 6], src[ 7], src[ 9],
                 src[10], src[11], src[12], src[14],
                       0,       0,       0,       1);
}

// input: ignore the bottom row
// output: inject identity row/column for alpha
static void matrix44_to_colormatrix(const SkM44& src, float dst[20]) {
    dst[0] = src.rc(0,0);
    dst[1] = src.rc(0,1);
    dst[2] = src.rc(0,2);
    dst[3] = 0;
    dst[4] = src.rc(0,3);    // tx

    dst[5] = src.rc(1,0);
    dst[6] = src.rc(1,1);
    dst[7] = src.rc(1,2);
    dst[8] = 0;
    dst[9] = src.rc(1,3);    // ty

    dst[10] = src.rc(2,0);
    dst[11] = src.rc(2,1);
    dst[12] = src.rc(2,2);
    dst[13] = 0;
    dst[14] = src.rc(2,3);   // tz

    dst[15] = dst[16] = dst[17] = dst[19] = 0;
    dst[18] = 1;
}

static void scale3(float m[], float s) {
    for (int i = 0; i < 3; ++i) {
        m[i] *= s;
    }
}

namespace {
enum Range { kFull, kLimited };
struct YUVCoeff {
    float   Kr, Kb;
    int     bits;
    Range   range;
};

const YUVCoeff gCoeff[] = {
    { 0.2990f, 0.1140f,  8, kFull    }, // kJPEG_Full_SkYUVColorSpace
    { 0.2990f, 0.1140f,  8, kLimited }, // kRec601_Limited_SkYUVColorSpace
    { 0.2126f, 0.0722f,  8, kFull    }, // kRec709_Full_SkYUVColorSpace
    { 0.2126f, 0.0722f,  8, kLimited }, // kRec709_Limited_SkYUVColorSpace
    { 0.2627f, 0.0593f,  8, kFull    }, // kBT2020_8bit_Full_SkYUVColorSpace
    { 0.2627f, 0.0593f,  8, kLimited }, // kBT2020_8bit_Limited_SkYUVColorSpace
    { 0.2627f, 0.0593f, 10, kFull    }, // kBT2020_10bit_Full_SkYUVColorSpace
    { 0.2627f, 0.0593f, 10, kLimited }, // kBT2020_10bit_Limited_SkYUVColorSpace
    { 0.2627f, 0.0593f, 12, kFull    }, // kBT2020_12bit_Full_SkYUVColorSpace
    { 0.2627f, 0.0593f, 12, kLimited }, // kBT2020_12bit_Limited_SkYUVColorSpace
};
}  // namespace

static void make_rgb_to_yuv_matrix(float mx[20], const YUVCoeff& c) {
    SkASSERT(c.bits >= 8);
    const float Kr = c.Kr;
    const float Kb = c.Kb;
    const float Kg = 1.0f - Kr - Kb;
    const float Cr = 0.5f / (1.0f - Kb);
    const float Cb = 0.5f / (1.0f - Kr);

    const int shift = c.bits - 8;

    const float denom = static_cast<float>((1 << c.bits) - 1);
    float scaleY  = 1.0f,
          addY    = 0.0f,
          scaleUV = 1.0f,
          addUV   = (128 << shift) / denom;

    if (c.range == kLimited) {
        scaleY  = (219 << shift) / denom;
        addY    = ( 16 << shift) / denom;
        scaleUV = (224 << shift) / denom;
    }

    float m[20] = {
          Kr,  Kg,   Kb,  0,  addY,
         -Kr, -Kg, 1-Kb,  0, addUV,
        1-Kr, -Kg,  -Kb,  0, addUV,
           0,   0,    0,  1,     0,
    };
    memcpy(mx, m, sizeof(m));
    scale3(mx +  0,      scaleY );
    scale3(mx +  5, Cr * scaleUV);
    scale3(mx + 10, Cb * scaleUV);
}

static void dump(const float m[20], SkYUVColorSpace cs, bool rgb2yuv) {
    const char* names[] = {
        "JPEG_full",
        "Rec601_limited",
        "Rec709_full",
        "Rec709_limited",
        "BT2020_8bit_full",
        "BT2020_8bit_limited",
        "BT2020_10bit_full",
        "BT2020_10bit_limited",
        "BT2020_12bit_full",
        "BT2020_12bit_limited",
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
    for (int i = 0; i < kLastEnum_SkYUVColorSpace; ++i) {
        SkYUVColorSpace cs = static_cast<SkYUVColorSpace>(i);
        float m[20];
        make_rgb_to_yuv_matrix(m, gCoeff[(unsigned)cs]);
        dump(m, cs, true);
        SkM44 m44, im44;
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
