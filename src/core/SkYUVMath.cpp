/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkYUVMath.h"

#include "include/core/SkImageInfo.h"
#include "include/core/SkM44.h"
#include "include/private/SkAssert.h"
#include "include/private/SkDebug.h"

#include <algorithm>
#include <array>
#include <cstring>

// in SkColorMatrix order (row-major)
// Created by running SkColorMatrix_DumpYUVMatrixTables()
constexpr float Rec601_8bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.322d0ep-2f,    0x1.2c8b42p-1f,    0x1.d2f1aap-4f,           0x0p+0f,           0x0p+0f,
    -0x1.599234p-3f,   -0x1.5336e4p-2f,           0x1p-1f,           0x0p+0f,    0x1.010102p-1f,
            0x1p-1f,    -0x1.acbc7p-2f,   -0x1.4d0e3ep-4f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.299000f,         0.587000f,         0.114000f,         0.000000f,         0.000000f,
         -0.168736f,        -0.331264f,         0.500000f,         0.000000f,         0.501961f,
          0.500000f,        -0.418688f,        -0.081312f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec601_8bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,  -0x1.d5babap-27f,    0x1.66e978p+0f,           0x0p+0f,   -0x1.6851ccp-1f,
     0x1.000002p+0f,   -0x1.606544p-2f,   -0x1.6da346p-1f,           0x0p+0f,     0x1.0ffafp-1f,
            0x1p+0f,    0x1.c5a1cap+0f,    0x1.72004p-27f,           0x0p+0f,   -0x1.c76936p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,        -0.000000f,         1.402000f,         0.000000f,        -0.703749f,
          1.000000f,        -0.344136f,        -0.714136f,         0.000000f,         0.531211f,
          1.000000f,         1.772000f,         0.000000f,         0.000000f,        -0.889475f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec601_8bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.06f37ep-2f,    0x1.021d3ep-1f,    0x1.9105c2p-4f,           0x0p+0f,    0x1.010102p-4f,
    -0x1.2f8f7ep-3f,   -0x1.29fa02p-2f,    0x1.c1c1c2p-2f,           0x0p+0f,    0x1.010102p-1f,
     0x1.c1c1c2p-2f,   -0x1.789d7ep-2f,   -0x1.249108p-4f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.256788f,         0.504129f,         0.097906f,         0.000000f,         0.062745f,
         -0.148223f,        -0.290993f,         0.439216f,         0.000000f,         0.501961f,
          0.439216f,        -0.367788f,        -0.071427f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec601_8bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2a150cp+0f,  -0x1.eeec98p-31f,    0x1.989536p+0f,           0x0p+0f,    -0x1.bf977p-1f,
     0x1.2a150cp+0f,   -0x1.912a24p-2f,   -0x1.a03d4ep-1f,           0x0p+0f,    0x1.1036c6p-1f,
     0x1.2a150cp+0f,    0x1.0234aap+1f,   0x1.2de0b6p-26f,           0x0p+0f,   -0x1.15ebe8p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.164384f,        -0.000000f,         1.596027f,         0.000000f,        -0.874202f,
          1.164384f,        -0.391762f,        -0.812968f,         0.000000f,         0.531668f,
          1.164384f,         2.017232f,         0.000000f,         0.000000f,        -1.085631f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec709_8bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
      0x1.b367ap-3f,    0x1.6e2eb2p-1f,     0x1.27bb3p-4f,           0x0p+0f,           0x0p+0f,
    -0x1.d5498ep-4f,   -0x1.8aad9cp-2f,           0x1p-1f,           0x0p+0f,    0x1.010102p-1f,
     0x1.fffffep-2f,   -0x1.d10d74p-2f,    -0x1.77945p-5f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.212600f,         0.715200f,         0.072200f,         0.000000f,         0.000000f,
         -0.114572f,        -0.385428f,         0.500000f,         0.000000f,         0.501961f,
          0.500000f,        -0.454153f,        -0.045847f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec709_8bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,   -0x1.134d1p-29f,    0x1.93261ap+0f,           0x0p+0f,   -0x1.94bad6p-1f,
            0x1p+0f,   -0x1.7fa3dep-3f,   -0x1.df5bf8p-2f,           0x0p+0f,    0x1.50e7dep-2f,
            0x1p+0f,    0x1.db089ap+0f,  -0x1.1fd88ap-31f,           0x0p+0f,   -0x1.dce582p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,        -0.000000f,         1.574800f,         0.000000f,        -0.790488f,
          1.000000f,        -0.187324f,        -0.468124f,         0.000000f,         0.329010f,
          1.000000f,         1.855600f,        -0.000000f,         0.000000f,        -0.931439f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec709_8bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.75ef96p-3f,    0x1.3a7c6ep-1f,    0x1.fbf63ap-5f,           0x0p+0f,    0x1.010102p-4f,
    -0x1.9c3c9ap-4f,   -0x1.5ab29cp-2f,    0x1.c1c1c4p-2f,           0x0p+0f,    0x1.010102p-1f,
      0x1.c1c1cp-2f,   -0x1.98844ap-2f,   -0x1.49ebb2p-5f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.182586f,         0.614231f,         0.062007f,         0.000000f,         0.062745f,
         -0.100644f,        -0.338572f,         0.439216f,         0.000000f,         0.501961f,
          0.439216f,        -0.398942f,        -0.040274f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float Rec709_8bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2a150cp+0f,  -0x1.532b74p-28f,    0x1.caf116p+0f,           0x0p+0f,   -0x1.f225dep-1f,
     0x1.2a150ap+0f,   -0x1.b4bbaep-3f,   -0x1.10d97ep-1f,           0x0p+0f,     0x1.34b7ep-2f,
     0x1.2a150ap+0f,    0x1.0e632ep+1f,  -0x1.65554ap-26f,           0x0p+0f,   -0x1.2226a6p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.164384f,        -0.000000f,         1.792741f,         0.000000f,        -0.972945f,
          1.164384f,        -0.213249f,        -0.532909f,         0.000000f,         0.301483f,
          1.164384f,         2.112402f,        -0.000000f,         0.000000f,        -1.133402f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_8bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.0d013ap-2f,    0x1.5b22d2p-1f,    0x1.e5c91ep-5f,           0x0p+0f,           0x0p+0f,
    -0x1.1df65cp-3f,   -0x1.7104d2p-2f,    0x1.fffffep-2f,           0x0p+0f,    0x1.010102p-1f,
            0x1p-1f,   -0x1.d6d212p-2f,   -0x1.496f7ep-5f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.262700f,         0.678000f,         0.059300f,         0.000000f,         0.000000f,
         -0.139630f,        -0.360370f,         0.500000f,         0.000000f,         0.501961f,
          0.500000f,        -0.459786f,        -0.040214f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_8bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,  -0x1.1b1682p-30f,    0x1.797f62p+0f,           0x0p+0f,   -0x1.7afa5ep-1f,
     0x1.fffffep-1f,   -0x1.51013cp-3f,   -0x1.248864p-1f,           0x0p+0f,    0x1.7a42f8p-2f,
            0x1p+0f,     0x1.e1a37p+0f,  -0x1.512cd4p-26f,           0x0p+0f,   -0x1.e386f8p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,        -0.000000f,         1.474600f,         0.000000f,        -0.740191f,
          1.000000f,        -0.164553f,        -0.571353f,         0.000000f,         0.369396f,
          1.000000f,         1.881400f,        -0.000000f,         0.000000f,        -0.944389f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_8bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.ce0e28p-3f,    0x1.2a20eap-1f,    0x1.a13442p-5f,           0x0p+0f,    0x1.010102p-4f,
    -0x1.f66588p-4f,    -0x1.44286p-2f,    0x1.c1c1c2p-2f,           0x0p+0f,    0x1.010102p-1f,
     0x1.c1c1c4p-2f,   -0x1.9d9566p-2f,   -0x1.2162f2p-5f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.225613f,         0.582282f,         0.050928f,         0.000000f,         0.062745f,
         -0.122655f,        -0.316560f,         0.439216f,         0.000000f,         0.501961f,
          0.439216f,        -0.403890f,        -0.035326f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_8bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2a150ap+0f,   -0x1.22eb3p-27f,    0x1.adbd94p+0f,           0x0p+0f,   -0x1.d4d50cp-1f,
     0x1.2a150ap+0f,   -0x1.7fa4d4p-3f,   -0x1.4d0468p-1f,           0x0p+0f,    0x1.63cc28p-2f,
     0x1.2a150ap+0f,    0x1.122598p+1f,  -0x1.73d38cp-26f,           0x0p+0f,   -0x1.25ecd6p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.164384f,        -0.000000f,         1.678674f,         0.000000f,        -0.915688f,
          1.164384f,        -0.187326f,        -0.650424f,         0.000000f,         0.347458f,
          1.164384f,         2.141772f,        -0.000000f,         0.000000f,        -1.148145f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_10bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.0d013ap-2f,    0x1.5b22d2p-1f,    0x1.e5c91ep-5f,           0x0p+0f,           0x0p+0f,
    -0x1.1df65cp-3f,   -0x1.7104d2p-2f,    0x1.fffffep-2f,           0x0p+0f,     0x1.00401p-1f,
            0x1p-1f,   -0x1.d6d212p-2f,   -0x1.496f7ep-5f,           0x0p+0f,     0x1.00401p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.262700f,         0.678000f,         0.059300f,         0.000000f,         0.000000f,
         -0.139630f,        -0.360370f,         0.500000f,         0.000000f,         0.500489f,
          0.500000f,        -0.459786f,        -0.040214f,         0.000000f,         0.500489f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_10bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,  -0x1.1b1682p-30f,    0x1.797f62p+0f,           0x0p+0f,   -0x1.79dddap-1f,
     0x1.fffffep-1f,   -0x1.51013cp-3f,   -0x1.248864p-1f,           0x0p+0f,    0x1.7926fcp-2f,
            0x1p+0f,     0x1.e1a37p+0f,  -0x1.512cd4p-26f,           0x0p+0f,   -0x1.e21bf6p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,        -0.000000f,         1.474600f,         0.000000f,        -0.738021f,
          1.000000f,        -0.164553f,        -0.571353f,         0.000000f,         0.368313f,
          1.000000f,         1.881400f,        -0.000000f,         0.000000f,        -0.941620f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_10bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.ccb346p-3f,    0x1.29411ap-1f,    0x1.9ffb0cp-5f,           0x0p+0f,     0x1.00401p-4f,
    -0x1.f4ec5cp-4f,   -0x1.433504p-2f,    0x1.c0701ap-2f,           0x0p+0f,     0x1.00401p-1f,
     0x1.c0701ep-2f,   -0x1.9c5ee8p-2f,    -0x1.2089bp-5f,           0x0p+0f,     0x1.00401p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.224951f,         0.580575f,         0.050779f,         0.000000f,         0.062561f,
         -0.122296f,        -0.315632f,         0.437928f,         0.000000f,         0.500489f,
          0.437928f,        -0.402706f,        -0.035222f,         0.000000f,         0.500489f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_10bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2af57ap+0f,  -0x1.641122p-27f,    0x1.af0126p+0f,           0x0p+0f,   -0x1.d4d50ap-1f,
     0x1.2af57ap+0f,   -0x1.80c5b2p-3f,   -0x1.4dff26p-1f,           0x0p+0f,    0x1.63cc26p-2f,
     0x1.2af57ap+0f,    0x1.12f404p+1f,  -0x1.23e582p-33f,           0x0p+0f,   -0x1.25ecd8p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.167808f,        -0.000000f,         1.683611f,         0.000000f,        -0.915688f,
          1.167808f,        -0.187877f,        -0.652337f,         0.000000f,         0.347458f,
          1.167808f,         2.148072f,        -0.000000f,         0.000000f,        -1.148145f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_12bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.0d013ap-2f,    0x1.5b22d2p-1f,    0x1.e5c91ep-5f,           0x0p+0f,           0x0p+0f,
    -0x1.1df65cp-3f,   -0x1.7104d2p-2f,    0x1.fffffep-2f,           0x0p+0f,    0x1.001002p-1f,
            0x1p-1f,   -0x1.d6d212p-2f,   -0x1.496f7ep-5f,           0x0p+0f,    0x1.001002p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.262700f,         0.678000f,         0.059300f,         0.000000f,         0.000000f,
         -0.139630f,        -0.360370f,         0.500000f,         0.000000f,         0.500122f,
          0.500000f,        -0.459786f,        -0.040214f,         0.000000f,         0.500122f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_12bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,  -0x1.1b1682p-30f,    0x1.797f62p+0f,           0x0p+0f,   -0x1.7996fep-1f,
     0x1.fffffep-1f,   -0x1.51013cp-3f,   -0x1.248864p-1f,           0x0p+0f,    0x1.78e042p-2f,
            0x1p+0f,     0x1.e1a37p+0f,  -0x1.512cd4p-26f,           0x0p+0f,   -0x1.e1c18ep-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,        -0.000000f,         1.474600f,         0.000000f,        -0.737480f,
          1.000000f,        -0.164553f,        -0.571353f,         0.000000f,         0.368043f,
          1.000000f,         1.881400f,        -0.000000f,         0.000000f,        -0.940930f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_12bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
      0x1.cc5cep-3f,    0x1.29095ap-1f,    0x1.9fad08p-5f,           0x0p+0f,    0x1.001002p-4f,
    -0x1.f48e6ap-4f,   -0x1.42f866p-2f,      0x1.c01cp-2f,           0x0p+0f,    0x1.001002p-1f,
     0x1.c01c04p-2f,   -0x1.9c1192p-2f,   -0x1.205394p-5f,           0x0p+0f,    0x1.001002p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.224787f,         0.580149f,         0.050742f,         0.000000f,         0.062515f,
         -0.122206f,        -0.315401f,         0.437607f,         0.000000f,         0.500122f,
          0.437607f,        -0.402411f,        -0.035196f,         0.000000f,         0.500122f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_12bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2b2d96p+0f,   -0x1.e313dp-28f,    0x1.af520ap+0f,           0x0p+0f,   -0x1.d4d50cp-1f,
     0x1.2b2d96p+0f,   -0x1.810deap-3f,   -0x1.4e3dd6p-1f,           0x0p+0f,    0x1.63cc28p-2f,
     0x1.2b2d96p+0f,    0x1.13279ep+1f,  -0x1.daca5ep-27f,           0x0p+0f,   -0x1.25ecd8p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.168664f,        -0.000000f,         1.684846f,         0.000000f,        -0.915688f,
          1.168664f,        -0.188015f,        -0.652816f,         0.000000f,         0.347458f,
          1.168664f,         2.149647f,        -0.000000f,         0.000000f,        -1.148145f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_16bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.0d013ap-2f,    0x1.5b22d2p-1f,    0x1.e5c91ep-5f,           0x0p+0f,           0x0p+0f,
    -0x1.1df65cp-3f,   -0x1.7104d2p-2f,    0x1.fffffep-2f,           0x0p+0f,      0x1.0001p-1f,
            0x1p-1f,   -0x1.d6d212p-2f,   -0x1.496f7ep-5f,           0x0p+0f,      0x1.0001p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.262700f,         0.678000f,         0.059300f,         0.000000f,         0.000000f,
         -0.139630f,        -0.360370f,         0.500000f,         0.000000f,         0.500008f,
          0.500000f,        -0.459786f,        -0.040214f,         0.000000f,         0.500008f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_16bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,  -0x1.1b1682p-30f,    0x1.797f62p+0f,           0x0p+0f,   -0x1.7980dcp-1f,
     0x1.fffffep-1f,   -0x1.51013cp-3f,   -0x1.248864p-1f,           0x0p+0f,    0x1.78ca2cp-2f,
            0x1p+0f,     0x1.e1a37p+0f,  -0x1.512cd4p-26f,           0x0p+0f,   -0x1.e1a552p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,        -0.000000f,         1.474600f,         0.000000f,        -0.737311f,
          1.000000f,        -0.164553f,        -0.571353f,         0.000000f,         0.367959f,
          1.000000f,         1.881400f,        -0.000000f,         0.000000f,        -0.940714f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_16bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.cc41e6p-3f,    0x1.28f7f2p-1f,    0x1.9f94acp-5f,           0x0p+0f,      0x1.0001p-4f,
    -0x1.f47116p-4f,   -0x1.42e57ap-2f,     0x1.c001cp-2f,           0x0p+0f,      0x1.0001p-1f,
      0x1.c001cp-2f,   -0x1.9bf96ap-2f,   -0x1.2042aep-5f,           0x0p+0f,      0x1.0001p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.224735f,         0.580017f,         0.050730f,         0.000000f,         0.062501f,
         -0.122178f,        -0.315329f,         0.437507f,         0.000000f,         0.500008f,
          0.437507f,        -0.402319f,        -0.035188f,         0.000000f,         0.500008f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float BT2020_16bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
      0x1.2b3f2p+0f,   0x1.57718ep-31f,    0x1.af6b54p+0f,           0x0p+0f,   -0x1.d4d50cp-1f,
      0x1.2b3f2p+0f,   -0x1.81247ap-3f,   -0x1.4e516ep-1f,           0x0p+0f,    0x1.63cc28p-2f,
      0x1.2b3f2p+0f,    0x1.1337bep+1f,  -0x1.88ceb8p-26f,           0x0p+0f,   -0x1.25ecd6p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.168932f,         0.000000f,         1.685231f,         0.000000f,        -0.915688f,
          1.168932f,        -0.188058f,        -0.652965f,         0.000000f,         0.347458f,
          1.168932f,         2.150139f,        -0.000000f,         0.000000f,        -1.148145f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float FCC_8bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.333334p-2f,    0x1.2e147ap-1f,    0x1.c28f5cp-4f,           0x0p+0f,           0x0p+0f,
    -0x1.592b26p-3f,   -0x1.536a6cp-2f,    0x1.fffffep-2f,           0x0p+0f,    0x1.010102p-1f,
            0x1p-1f,   -0x1.af8af8p-2f,   -0x1.41d41ep-4f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.300000f,         0.590000f,         0.110000f,         0.000000f,         0.000000f,
         -0.168539f,        -0.331461f,         0.500000f,         0.000000f,         0.501961f,
          0.500000f,        -0.421429f,        -0.078571f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float FCC_8bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,   0x1.12f3e2p-28f,    0x1.666666p+0f,           0x0p+0f,   -0x1.67ce36p-1f,
            0x1p+0f,   -0x1.53d446p-2f,    -0x1.6c798p-1f,           0x0p+0f,     0x1.0c3e1p-1f,
            0x1p+0f,    0x1.c7ae16p+0f,   0x1.1bcdf2p-31f,           0x0p+0f,    -0x1.c9779p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,         0.000000f,         1.400000f,         0.000000f,        -0.702745f,
          1.000000f,        -0.331864f,        -0.711864f,         0.000000f,         0.523911f,
          1.000000f,         1.780000f,         0.000000f,         0.000000f,        -0.893490f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float FCC_8bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.07d4a2p-2f,    0x1.036ef4p-1f,    0x1.82f398p-4f,           0x0p+0f,    0x1.010102p-4f,
    -0x1.2f34f6p-3f,   -0x1.2a2746p-2f,    0x1.c1c1c2p-2f,           0x0p+0f,    0x1.010102p-1f,
     0x1.c1c1c2p-2f,   -0x1.7b14aep-2f,   -0x1.1ab44ep-4f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.257647f,         0.506706f,         0.094471f,         0.000000f,         0.062745f,
         -0.148050f,        -0.291165f,         0.439216f,         0.000000f,         0.501961f,
          0.439216f,        -0.370196f,        -0.069020f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float FCC_8bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2a150ap+0f,   -0x1.3a8c8p-29f,        0x1.98p+0f,           0x0p+0f,   -0x1.bf01a4p-1f,
     0x1.2a150ap+0f,   -0x1.82dbeep-2f,   -0x1.9eea4ep-1f,           0x0p+0f,    0x1.0bf578p-1f,
     0x1.2a150ap+0f,    0x1.035f16p+1f,   0x1.a8fbccp-27f,           0x0p+0f,    -0x1.17178p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.164384f,        -0.000000f,         1.593750f,         0.000000f,        -0.873059f,
          1.164384f,        -0.377792f,        -0.810381f,         0.000000f,         0.523357f,
          1.164384f,         2.026339f,         0.000000f,         0.000000f,        -1.090202f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float SMPTE240_8bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.b22d0ep-3f,    0x1.66e978p-1f,    0x1.645a1cp-4f,           0x0p+0f,           0x0p+0f,
    -0x1.db8c7cp-4f,   -0x1.891ce2p-2f,           0x1p-1f,           0x0p+0f,    0x1.010102p-1f,
            0x1p-1f,   -0x1.c778dap-2f,   -0x1.c4392ep-5f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.212000f,         0.701000f,         0.087000f,         0.000000f,         0.000000f,
         -0.116101f,        -0.383899f,         0.500000f,         0.000000f,         0.501961f,
          0.500000f,        -0.444797f,        -0.055203f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float SMPTE240_8bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,   0x1.d39c56p-29f,    0x1.9374bcp+0f,           0x0p+0f,   -0x1.9509c8p-1f,
            0x1p+0f,   -0x1.d01f2ep-3f,   -0x1.e80f98p-2f,           0x0p+0f,    0x1.697912p-2f,
            0x1p+0f,    0x1.d374bcp+0f,   0x1.ea3024p-29f,           0x0p+0f,   -0x1.d54a08p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,         0.000000f,         1.576000f,         0.000000f,        -0.791090f,
          1.000000f,        -0.226622f,        -0.476622f,         0.000000f,         0.353001f,
          1.000000f,         1.826000f,         0.000000f,         0.000000f,        -0.916580f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float SMPTE240_8bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.74e16cp-3f,    0x1.343df8p-1f,    0x1.320b22p-4f,           0x0p+0f,    0x1.010102p-4f,
    -0x1.a1bcaap-4f,   -0x1.595298p-2f,    0x1.c1c1c4p-2f,           0x0p+0f,    0x1.010102p-1f,
     0x1.c1c1c2p-2f,   -0x1.9019d8p-2f,   -0x1.8d3f46p-5f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.182071f,         0.602035f,         0.074718f,         0.000000f,         0.062745f,
         -0.101987f,        -0.337229f,         0.439216f,         0.000000f,         0.501961f,
          0.439216f,        -0.390724f,        -0.048492f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float SMPTE240_8bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2a150ap+0f,   -0x1.03331p-27f,    0x1.cb4a9cp+0f,           0x0p+0f,   -0x1.f27fbep-1f,
     0x1.2a150cp+0f,   -0x1.082d2cp-2f,   -0x1.15cd72p-1f,           0x0p+0f,    0x1.50af76p-2f,
     0x1.2a150ap+0f,    0x1.0a1304p+1f,   0x1.2b37d6p-25f,           0x0p+0f,   -0x1.1dd228p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.164384f,        -0.000000f,         1.794107f,         0.000000f,        -0.973631f,
          1.164384f,        -0.257985f,        -0.542583f,         0.000000f,         0.328794f,
          1.164384f,         2.078705f,         0.000000f,         0.000000f,        -1.116488f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YDZDX_8bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,
            0x0p+0f,          -0x1p-1f,    0x1.f91f2ep-2f,           0x0p+0f,    0x1.010102p-1f,
            0x1p-1f,   -0x1.fbda94p-2f,           0x0p+0f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         1.000000f,         0.000000f,         0.000000f,         0.000000f,
          0.000000f,        -0.500000f,         0.493283f,         0.000000f,         0.501961f,
          0.500000f,        -0.495951f,         0.000000f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YDZDX_8bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.fbda94p-1f,          -0x0p+0f,           0x1p+1f,           0x0p+0f,   -0x1.010102p+0f,
            0x1p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,
     0x1.037c66p+0f,    0x1.037c66p+1f,           0x0p+0f,           0x0p+0f,   -0x1.0480e8p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.991902f,        -0.000000f,         2.000000f,         0.000000f,        -1.003922f,
          1.000000f,         0.000000f,         0.000000f,         0.000000f,         0.000000f,
          1.013617f,         2.027234f,         0.000000f,         0.000000f,        -1.017592f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YDZDX_8bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,    0x1.b7b7b8p-1f,           0x0p+0f,           0x0p+0f,    0x1.010102p-4f,
            0x0p+0f,   -0x1.c1c1c2p-2f,      0x1.bbb7p-2f,           0x0p+0f,    0x1.010102p-1f,
     0x1.c1c1c2p-2f,    -0x1.be1d6p-2f,           0x0p+0f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         0.858824f,         0.000000f,         0.000000f,         0.062745f,
          0.000000f,        -0.439216f,         0.433315f,         0.000000f,         0.501961f,
          0.439216f,        -0.435659f,         0.000000f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YDZDX_8bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.27ab18p+0f,          -0x0p+0f,    0x1.236db6p+1f,           0x0p+0f,   -0x1.371f88p+0f,
     0x1.2a150ap+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,   -0x1.2b404cp-4f,
     0x1.2e2422p+0f,    0x1.27659cp+1f,           0x0p+0f,           0x0p+0f,   -0x1.3b8364p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.154954f,        -0.000000f,         2.276786f,         0.000000f,        -1.215325f,
          1.164384f,         0.000000f,         0.000000f,         0.000000f,        -0.073059f,
          1.180239f,         2.307788f,         0.000000f,         0.000000f,        -1.232474f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float GBR_8bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,
            0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,
            0x1p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         1.000000f,         0.000000f,         0.000000f,         0.000000f,
          0.000000f,         0.000000f,         1.000000f,         0.000000f,         0.000000f,
          1.000000f,         0.000000f,         0.000000f,         0.000000f,         0.000000f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float GBR_8bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,
            0x1p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,
            0x0p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,          -0x0p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         0.000000f,         1.000000f,         0.000000f,         0.000000f,
          1.000000f,         0.000000f,         0.000000f,         0.000000f,         0.000000f,
          0.000000f,         1.000000f,         0.000000f,         0.000000f,        -0.000000f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float GBR_8bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,    0x1.b7b7b8p-1f,           0x0p+0f,           0x0p+0f,    0x1.010102p-4f,
            0x0p+0f,           0x0p+0f,    0x1.b7b7b8p-1f,           0x0p+0f,    0x1.010102p-4f,
     0x1.b7b7b8p-1f,           0x0p+0f,           0x0p+0f,           0x0p+0f,    0x1.010102p-4f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         0.858824f,         0.000000f,         0.000000f,         0.062745f,
          0.000000f,         0.000000f,         0.858824f,         0.000000f,         0.062745f,
          0.858824f,         0.000000f,         0.000000f,         0.000000f,         0.062745f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float GBR_8bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x0p+0f,           0x0p+0f,    0x1.2a150ap+0f,           0x0p+0f,   -0x1.2b404cp-4f,
     0x1.2a150ap+0f,           0x0p+0f,           0x0p+0f,           0x0p+0f,   -0x1.2b404cp-4f,
            0x0p+0f,    0x1.2a150ap+0f,           0x0p+0f,           0x0p+0f,   -0x1.2b404cp-4f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.000000f,         0.000000f,         1.164384f,         0.000000f,        -0.073059f,
          1.164384f,         0.000000f,         0.000000f,         0.000000f,        -0.073059f,
          0.000000f,         1.164384f,         0.000000f,         0.000000f,        -0.073059f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_8bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p-2f,           0x1p-1f,           0x1p-2f,           0x0p+0f,           0x0p+0f,
           -0x1p-2f,           0x1p-1f,          -0x1p-2f,           0x0p+0f,    0x1.010102p-1f,
            0x1p-1f,           0x0p+0f,          -0x1p-1f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.250000f,         0.500000f,         0.250000f,         0.000000f,         0.000000f,
         -0.250000f,         0.500000f,        -0.250000f,         0.000000f,         0.501961f,
          0.500000f,         0.000000f,        -0.500000f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_8bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,          -0x1p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,
            0x1p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,   -0x1.010102p-1f,
            0x1p+0f,          -0x1p+0f,          -0x1p+0f,           0x0p+0f,    0x1.010102p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,        -1.000000f,         1.000000f,         0.000000f,         0.000000f,
          1.000000f,         1.000000f,         0.000000f,         0.000000f,        -0.501961f,
          1.000000f,        -1.000000f,        -1.000000f,         0.000000f,         1.003922f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_8bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.b7b7b8p-3f,    0x1.b7b7b8p-2f,    0x1.b7b7b8p-3f,           0x0p+0f,    0x1.010102p-4f,
    -0x1.b7b7b8p-3f,    0x1.b7b7b8p-2f,   -0x1.b7b7b8p-3f,           0x0p+0f,    0x1.010102p-1f,
     0x1.b7b7b8p-2f,           0x0p+0f,   -0x1.b7b7b8p-2f,           0x0p+0f,    0x1.010102p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.214706f,         0.429412f,         0.214706f,         0.000000f,         0.062745f,
         -0.214706f,         0.429412f,        -0.214706f,         0.000000f,         0.501961f,
          0.429412f,         0.000000f,        -0.429412f,         0.000000f,         0.501961f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_8bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2a150ap+0f,   -0x1.2a150ap+0f,    0x1.2a150ap+0f,           0x0p+0f,   -0x1.2b404cp-4f,
     0x1.2a150ap+0f,    0x1.2a150ap+0f,           0x0p+0f,           0x0p+0f,   -0x1.50a856p-1f,
     0x1.2a150ap+0f,   -0x1.2a150ap+0f,   -0x1.2a150ap+0f,           0x0p+0f,    0x1.188c48p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.164384f,        -1.164384f,         1.164384f,         0.000000f,        -0.073059f,
          1.164384f,         1.164384f,         0.000000f,         0.000000f,        -0.657534f,
          1.164384f,        -1.164384f,        -1.164384f,         0.000000f,         1.095891f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_10bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p-2f,           0x1p-1f,           0x1p-2f,           0x0p+0f,           0x0p+0f,
           -0x1p-2f,           0x1p-1f,          -0x1p-2f,           0x0p+0f,     0x1.00401p-1f,
            0x1p-1f,           0x0p+0f,          -0x1p-1f,           0x0p+0f,     0x1.00401p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.250000f,         0.500000f,         0.250000f,         0.000000f,         0.000000f,
         -0.250000f,         0.500000f,        -0.250000f,         0.000000f,         0.500489f,
          0.500000f,         0.000000f,        -0.500000f,         0.000000f,         0.500489f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_10bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,          -0x1p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,
            0x1p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,    -0x1.00401p-1f,
            0x1p+0f,          -0x1p+0f,          -0x1p+0f,           0x0p+0f,     0x1.00401p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,        -1.000000f,         1.000000f,         0.000000f,         0.000000f,
          1.000000f,         1.000000f,         0.000000f,         0.000000f,        -0.500489f,
          1.000000f,        -1.000000f,        -1.000000f,         0.000000f,         1.000978f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_10bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.b66d9cp-3f,    0x1.b66d9cp-2f,    0x1.b66d9cp-3f,           0x0p+0f,     0x1.00401p-4f,
    -0x1.b66d9cp-3f,    0x1.b66d9cp-2f,   -0x1.b66d9cp-3f,           0x0p+0f,     0x1.00401p-1f,
     0x1.b66d9cp-2f,           0x0p+0f,   -0x1.b66d9cp-2f,           0x0p+0f,     0x1.00401p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.214076f,         0.428153f,         0.214076f,         0.000000f,         0.062561f,
         -0.214076f,         0.428153f,        -0.214076f,         0.000000f,         0.500489f,
          0.428153f,         0.000000f,        -0.428153f,         0.000000f,         0.500489f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_10bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2af57ap+0f,   -0x1.2af57ap+0f,    0x1.2af57ap+0f,           0x0p+0f,   -0x1.2b404ap-4f,
     0x1.2af57ap+0f,    0x1.2af57ap+0f,           0x0p+0f,           0x0p+0f,   -0x1.50a854p-1f,
     0x1.2af57ap+0f,   -0x1.2af57ap+0f,   -0x1.2af57ap+0f,           0x0p+0f,    0x1.188c46p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.167808f,        -1.167808f,         1.167808f,         0.000000f,        -0.073059f,
          1.167808f,         1.167808f,         0.000000f,         0.000000f,        -0.657534f,
          1.167808f,        -1.167808f,        -1.167808f,         0.000000f,         1.095890f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_12bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p-2f,           0x1p-1f,           0x1p-2f,           0x0p+0f,           0x0p+0f,
           -0x1p-2f,           0x1p-1f,          -0x1p-2f,           0x0p+0f,    0x1.001002p-1f,
            0x1p-1f,           0x0p+0f,          -0x1p-1f,           0x0p+0f,    0x1.001002p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.250000f,         0.500000f,         0.250000f,         0.000000f,         0.000000f,
         -0.250000f,         0.500000f,        -0.250000f,         0.000000f,         0.500122f,
          0.500000f,         0.000000f,        -0.500000f,         0.000000f,         0.500122f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_12bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,          -0x1p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,
            0x1p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,   -0x1.001002p-1f,
            0x1p+0f,          -0x1p+0f,          -0x1p+0f,           0x0p+0f,    0x1.001002p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,        -1.000000f,         1.000000f,         0.000000f,         0.000000f,
          1.000000f,         1.000000f,         0.000000f,         0.000000f,        -0.500122f,
          1.000000f,        -1.000000f,        -1.000000f,         0.000000f,         1.000244f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_12bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.b61b62p-3f,    0x1.b61b62p-2f,    0x1.b61b62p-3f,           0x0p+0f,    0x1.001002p-4f,
    -0x1.b61b62p-3f,    0x1.b61b62p-2f,   -0x1.b61b62p-3f,           0x0p+0f,    0x1.001002p-1f,
     0x1.b61b62p-2f,           0x0p+0f,   -0x1.b61b62p-2f,           0x0p+0f,    0x1.001002p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.213919f,         0.427839f,         0.213919f,         0.000000f,         0.062515f,
         -0.213919f,         0.427839f,        -0.213919f,         0.000000f,         0.500122f,
          0.427839f,         0.000000f,        -0.427839f,         0.000000f,         0.500122f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_12bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.2b2d96p+0f,   -0x1.2b2d96p+0f,    0x1.2b2d96p+0f,           0x0p+0f,   -0x1.2b404cp-4f,
     0x1.2b2d96p+0f,    0x1.2b2d96p+0f,           0x0p+0f,           0x0p+0f,   -0x1.50a856p-1f,
     0x1.2b2d96p+0f,   -0x1.2b2d96p+0f,   -0x1.2b2d96p+0f,           0x0p+0f,    0x1.188c48p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.168664f,        -1.168664f,         1.168664f,         0.000000f,        -0.073059f,
          1.168664f,         1.168664f,         0.000000f,         0.000000f,        -0.657534f,
          1.168664f,        -1.168664f,        -1.168664f,         0.000000f,         1.095891f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_16bit_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p-2f,           0x1p-1f,           0x1p-2f,           0x0p+0f,           0x0p+0f,
           -0x1p-2f,           0x1p-1f,          -0x1p-2f,           0x0p+0f,      0x1.0001p-1f,
            0x1p-1f,           0x0p+0f,          -0x1p-1f,           0x0p+0f,      0x1.0001p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.250000f,         0.500000f,         0.250000f,         0.000000f,         0.000000f,
         -0.250000f,         0.500000f,        -0.250000f,         0.000000f,         0.500008f,
          0.500000f,         0.000000f,        -0.500000f,         0.000000f,         0.500008f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_16bit_full_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
            0x1p+0f,          -0x1p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,
            0x1p+0f,           0x1p+0f,           0x0p+0f,           0x0p+0f,     -0x1.0001p-1f,
            0x1p+0f,          -0x1p+0f,          -0x1p+0f,           0x0p+0f,      0x1.0001p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.000000f,        -1.000000f,         1.000000f,         0.000000f,         0.000000f,
          1.000000f,         1.000000f,         0.000000f,         0.000000f,        -0.500008f,
          1.000000f,        -1.000000f,        -1.000000f,         0.000000f,         1.000015f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_16bit_limited_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
     0x1.b601b6p-3f,    0x1.b601b6p-2f,    0x1.b601b6p-3f,           0x0p+0f,      0x1.0001p-4f,
    -0x1.b601b6p-3f,    0x1.b601b6p-2f,   -0x1.b601b6p-3f,           0x0p+0f,      0x1.0001p-1f,
     0x1.b601b6p-2f,           0x0p+0f,   -0x1.b601b6p-2f,           0x0p+0f,      0x1.0001p-1f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          0.213870f,         0.427741f,         0.213870f,         0.000000f,         0.062501f,
         -0.213870f,         0.427741f,        -0.213870f,         0.000000f,         0.500008f,
          0.427741f,         0.000000f,        -0.427741f,         0.000000f,         0.500008f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
};
constexpr float YCgCo_16bit_limited_yuv_to_rgb[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
      0x1.2b3f2p+0f,    -0x1.2b3f2p+0f,     0x1.2b3f2p+0f,           0x0p+0f,   -0x1.2b404ap-4f,
      0x1.2b3f2p+0f,     0x1.2b3f2p+0f,           0x0p+0f,           0x0p+0f,   -0x1.50a854p-1f,
      0x1.2b3f2p+0f,    -0x1.2b3f2p+0f,    -0x1.2b3f2p+0f,           0x0p+0f,    0x1.188c46p+0f,
            0x0p+0f,           0x0p+0f,           0x0p+0f,           0x1p+0f,           0x0p+0f,
#else
          1.168932f,        -1.168932f,         1.168932f,         0.000000f,        -0.073059f,
          1.168932f,         1.168932f,         0.000000f,         0.000000f,        -0.657534f,
          1.168932f,        -1.168932f,        -1.168932f,         0.000000f,         1.095890f,
          0.000000f,         0.000000f,         0.000000f,         1.000000f,         0.000000f,
#endif
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
static_assert(kBT2020_16bit_Full_SkYUVColorSpace    == 10, "");
static_assert(kBT2020_16bit_Limited_SkYUVColorSpace == 11, "");
static_assert(kFCC_Full_SkYUVColorSpace             == 12, "");
static_assert(kFCC_Limited_SkYUVColorSpace          == 13, "");
static_assert(kSMPTE240_Full_SkYUVColorSpace        == 14, "");
static_assert(kSMPTE240_Limited_SkYUVColorSpace     == 15, "");
static_assert(kYDZDX_Full_SkYUVColorSpace           == 16, "");
static_assert(kYDZDX_Limited_SkYUVColorSpace        == 17, "");
static_assert(kGBR_Full_SkYUVColorSpace             == 18, "");
static_assert(kGBR_Limited_SkYUVColorSpace          == 19, "");
static_assert(kYCgCo_8bit_Full_SkYUVColorSpace      == 20, "");
static_assert(kYCgCo_8bit_Limited_SkYUVColorSpace   == 21, "");
static_assert(kYCgCo_10bit_Full_SkYUVColorSpace     == 22, "");
static_assert(kYCgCo_10bit_Limited_SkYUVColorSpace  == 23, "");
static_assert(kYCgCo_12bit_Full_SkYUVColorSpace     == 24, "");
static_assert(kYCgCo_12bit_Limited_SkYUVColorSpace  == 25, "");
static_assert(kYCgCo_16bit_Full_SkYUVColorSpace     == 26, "");
static_assert(kYCgCo_16bit_Limited_SkYUVColorSpace  == 27, "");


// Helpers for converting and generating color matrices

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
enum class Range : int {
    kFull,
    kLimited,
};

enum class MatrixCoefficients : int {
    kRec601,
    kRec709,
    kBT2020,
    kFCC,
    kSMPTE240,
    kYDZDX,
    kGBR,
    kYCgCo,
};

struct ColorSpaceInfo {
    MatrixCoefficients matrix_coefficients;
    Range range;
    int bit_depth;
};

static constexpr auto kColorSpaceInfo = std::to_array<ColorSpaceInfo>({
    {MatrixCoefficients::kRec601,   Range::kFull,     8},  // kJPEG_Full
    {MatrixCoefficients::kRec601,   Range::kLimited,  8},  // kRec601_Limited
    {MatrixCoefficients::kRec709,   Range::kFull,     8},  // kRec709_Full
    {MatrixCoefficients::kRec709,   Range::kLimited,  8},  // kRec709_Limited
    {MatrixCoefficients::kBT2020,   Range::kFull,     8},  // kBT2020_8bit_Full
    {MatrixCoefficients::kBT2020,   Range::kLimited,  8},  // kBT2020_8bit_Limited
    {MatrixCoefficients::kBT2020,   Range::kFull,    10},  // kBT2020_10bit_Full
    {MatrixCoefficients::kBT2020,   Range::kLimited, 10},  // kBT2020_10bit_Limited
    {MatrixCoefficients::kBT2020,   Range::kFull,    12},  // kBT2020_12bit_Full
    {MatrixCoefficients::kBT2020,   Range::kLimited, 12},  // kBT2020_12bit_Limited
    {MatrixCoefficients::kBT2020,   Range::kFull,    16},  // kBT2020_16bit_Full
    {MatrixCoefficients::kBT2020,   Range::kLimited, 16},  // kBT2020_16bit_Limited
    {MatrixCoefficients::kFCC,      Range::kFull,     8},  // kFCC_Full
    {MatrixCoefficients::kFCC,      Range::kLimited,  8},  // kFCC_Limited
    {MatrixCoefficients::kSMPTE240, Range::kFull,     8},  // kSMPTE240_Full
    {MatrixCoefficients::kSMPTE240, Range::kLimited,  8},  // kSMPTE240_Limited
    {MatrixCoefficients::kYDZDX,    Range::kFull,     8},  // kYDZDX_Full
    {MatrixCoefficients::kYDZDX,    Range::kLimited,  8},  // kYDZDX_Limited
    {MatrixCoefficients::kGBR,      Range::kFull,     8},  // kGBR_Full
    {MatrixCoefficients::kGBR,      Range::kLimited,  8},  // kGBR_Limited
    {MatrixCoefficients::kYCgCo,    Range::kFull,     8},  // kYCgCo_8bit_Full
    {MatrixCoefficients::kYCgCo,    Range::kLimited,  8},  // kYCgCo_8bit_Limited
    {MatrixCoefficients::kYCgCo,    Range::kFull,    10},  // kYCgCo_10bit_Full
    {MatrixCoefficients::kYCgCo,    Range::kLimited, 10},  // kYCgCo_10bit_Limited
    {MatrixCoefficients::kYCgCo,    Range::kFull,    12},  // kYCgCo_12bit_Full
    {MatrixCoefficients::kYCgCo,    Range::kLimited, 12},  // kYCgCo_12bit_Limited
    {MatrixCoefficients::kYCgCo,    Range::kFull,    16},  // kYCgCo_16bit_Full
    {MatrixCoefficients::kYCgCo,    Range::kLimited, 16},  // kYCgCo_16bit_Limited
});
static_assert(std::size(kColorSpaceInfo) == kIdentity_SkYUVColorSpace);
}  // namespace

static void make_rgb_to_yuv_matrix_ycbcr(float mx[20],
                                         MatrixCoefficients mc,
                                         int bits,
                                         Range range) {
    SkASSERT(bits >= 8);
    float Kr = 0.0f, Kb = 0.0f;
    switch (mc) {
        case MatrixCoefficients::kRec601:
            Kr = 0.2990f;
            Kb = 0.1140f;
            break;
        case MatrixCoefficients::kRec709:
            Kr = 0.2126f;
            Kb = 0.0722f;
            break;
        case MatrixCoefficients::kBT2020:
            Kr = 0.2627f;
            Kb = 0.0593f;
            break;
        case MatrixCoefficients::kFCC:
            Kr = 0.3000f;
            Kb = 0.1100f;
            break;
        case MatrixCoefficients::kSMPTE240:
            Kr = 0.2120f;
            Kb = 0.0870f;
            break;
        default:
            SkUNREACHABLE;
    }

    const float Kg = 1.0f - Kr - Kb;
    const float Cr = 0.5f / (1.0f - Kb);
    const float Cb = 0.5f / (1.0f - Kr);

    const int shift = bits - 8;

    const float denom = static_cast<float>((1 << bits) - 1);
    float scaleY  = 1.0f,
          addY    = 0.0f,
          scaleUV = 1.0f,
          addUV   = (128 << shift) / denom;

    if (range == Range::kLimited) {
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

static void make_rgb_to_yuv_matrix_ydzdx(float mx[20], int bits, Range range) {
    SkASSERT(bits >= 8);
    const int shift = bits - 8;
    const float denom = static_cast<float>((1 << bits) - 1);
    float scaleY  = 1.0f,
          addY    = 0.0f,
          scaleUV = 1.0f,
          addUV   = (128 << shift) / denom;

    if (range == Range::kLimited) {
        scaleY  = (219 << shift) / denom;
        addY    = ( 16 << shift) / denom;
        scaleUV = (224 << shift) / denom;
    }

    // YDZDX applies range correction to YUV values similar to YCbCr.
    float m[20] = {
          0.0f,              1.0f,             0.0f, 0.0f,  addY,  // Y
          0.0f,             -0.5f, 0.986566f / 2.0f, 0.0f, addUV,  // DX or DZ
          0.5f, -0.991902f / 2.0f,             0.0f, 0.0f, addUV,  // DZ or DX
          0.0f,              0.0f,             0.0f, 1.0f,  0.0f,
    };
    memcpy(mx, m, sizeof(m));
    scale3(mx +  0, scaleY );
    scale3(mx +  5, scaleUV);
    scale3(mx + 10, scaleUV);
}

static void make_rgb_to_yuv_matrix_gbr(float mx[20], int bits, Range range) {
    SkASSERT(bits >= 8);
    const int shift = bits - 8;
    const float denom = static_cast<float>((1 << bits) - 1);
    float scaleY   = 1.0f,
          addY     = 0.0f;

    if (range == Range::kLimited) {
        scaleY  = (219 << shift) / denom;
        addY    = ( 16 << shift) / denom;
    }

    // GBR applies range correction to RGB values similar to YCgCo.
    float m[20] = {
          0.0f, 1.0f, 0.0f, 0.0f,  addY, // G
          0.0f, 0.0f, 1.0f, 0.0f,  addY, // B
          1.0f, 0.0f, 0.0f, 0.0f,  addY, // R
          0.0f, 0.0f, 0.0f, 1.0f,  0.0f,
    };
    memcpy(mx, m, sizeof(m));
    scale3(mx +  0, scaleY);
    scale3(mx +  5, scaleY);
    scale3(mx + 10, scaleY);
}

static void make_rgb_to_yuv_matrix_ycgco(float mx[20], int bits, Range range) {
    SkASSERT(bits >= 8);
    const int shift = bits - 8;
    const float denom = static_cast<float>((1 << bits) - 1);
    float scaleY   = 1.0f,
          addY     = 0.0f,
          chroma05 = static_cast<float>(1 << (bits - 1)) / denom;

    if (range == Range::kLimited) {
        scaleY  = (219 << shift) / denom;
        addY    = ( 16 << shift) / denom;
    }

    float m[20] = {
          0.25f,  0.5f,   0.25f,  0.0f,  addY,
         -0.25f,  0.5f,  -0.25f,  0.0f,  chroma05,
           0.5f,  0.0f,  -0.5f,   0.0f,  chroma05,
           0.0f,  0.0f,   0.0f,   1.0f,  0.0f,
    };
    memcpy(mx, m, sizeof(m));
    scale3(mx +  0, scaleY);
    scale3(mx +  5, scaleY);
    scale3(mx + 10, scaleY);
}

static void make_rgb_to_yuv_matrix(float mx[20], MatrixCoefficients mc, int bits, Range range) {
    switch (mc) {
        case MatrixCoefficients::kRec601:
        case MatrixCoefficients::kRec709:
        case MatrixCoefficients::kBT2020:
        case MatrixCoefficients::kFCC:
        case MatrixCoefficients::kSMPTE240:
            return make_rgb_to_yuv_matrix_ycbcr(mx, mc, bits, range);
        case MatrixCoefficients::kYDZDX:
            return make_rgb_to_yuv_matrix_ydzdx(mx, bits, range);
        case MatrixCoefficients::kGBR:
            return make_rgb_to_yuv_matrix_gbr(mx, bits, range);
        case MatrixCoefficients::kYCgCo:
            return make_rgb_to_yuv_matrix_ycgco(mx, bits, range);
    }
}

// Returns nullptr for kIdentity_SkYUVColorSpace and invalid values.
static const ColorSpaceInfo* get_color_space_info(SkYUVColorSpace cs) {
    if (cs < 0 || cs >= kIdentity_SkYUVColorSpace) {
        return nullptr;
    }
    return &kColorSpaceInfo[cs];
}

static void make_rgb_to_yuv_matrix(float mx[20], const ColorSpaceInfo& info) {
    make_rgb_to_yuv_matrix(mx, info.matrix_coefficients, info.bit_depth, info.range);
}

struct PrecomputedMatrices {
    const float* rgb_to_yuv = nullptr;
    const float* yuv_to_rgb = nullptr;
};

// Returns the precomputed tables for `cs`, or nullptrs if they must be computed at runtime.
static PrecomputedMatrices get_precomputed_matrices(SkYUVColorSpace cs) {
    switch (cs) {
        case kJPEG_Full_SkYUVColorSpace:
            return {Rec601_8bit_full_rgb_to_yuv, Rec601_8bit_full_yuv_to_rgb};
        case kRec601_Limited_SkYUVColorSpace:
            return {Rec601_8bit_limited_rgb_to_yuv, Rec601_8bit_limited_yuv_to_rgb};
        case kRec709_Full_SkYUVColorSpace:
            return {Rec709_8bit_full_rgb_to_yuv, Rec709_8bit_full_yuv_to_rgb};
        case kRec709_Limited_SkYUVColorSpace:
            return {Rec709_8bit_limited_rgb_to_yuv, Rec709_8bit_limited_yuv_to_rgb};
        case kBT2020_8bit_Full_SkYUVColorSpace:
            return {BT2020_8bit_full_rgb_to_yuv, BT2020_8bit_full_yuv_to_rgb};
        case kBT2020_8bit_Limited_SkYUVColorSpace:
            return {BT2020_8bit_limited_rgb_to_yuv, BT2020_8bit_limited_yuv_to_rgb};
        case kBT2020_10bit_Full_SkYUVColorSpace:
            return {BT2020_10bit_full_rgb_to_yuv, BT2020_10bit_full_yuv_to_rgb};
        case kBT2020_10bit_Limited_SkYUVColorSpace:
            return {BT2020_10bit_limited_rgb_to_yuv, BT2020_10bit_limited_yuv_to_rgb};
        case kBT2020_12bit_Full_SkYUVColorSpace:
            return {BT2020_12bit_full_rgb_to_yuv, BT2020_12bit_full_yuv_to_rgb};
        case kBT2020_12bit_Limited_SkYUVColorSpace:
            return {BT2020_12bit_limited_rgb_to_yuv, BT2020_12bit_limited_yuv_to_rgb};
        case kBT2020_16bit_Full_SkYUVColorSpace:
            return {BT2020_16bit_full_rgb_to_yuv, BT2020_16bit_full_yuv_to_rgb};
        case kBT2020_16bit_Limited_SkYUVColorSpace:
            return {BT2020_16bit_limited_rgb_to_yuv, BT2020_16bit_limited_yuv_to_rgb};
        case kFCC_Full_SkYUVColorSpace:
            return {FCC_8bit_full_rgb_to_yuv, FCC_8bit_full_yuv_to_rgb};
        case kFCC_Limited_SkYUVColorSpace:
            return {FCC_8bit_limited_rgb_to_yuv, FCC_8bit_limited_yuv_to_rgb};
        case kSMPTE240_Full_SkYUVColorSpace:
            return {SMPTE240_8bit_full_rgb_to_yuv, SMPTE240_8bit_full_yuv_to_rgb};
        case kSMPTE240_Limited_SkYUVColorSpace:
            return {SMPTE240_8bit_limited_rgb_to_yuv, SMPTE240_8bit_limited_yuv_to_rgb};
        case kYDZDX_Full_SkYUVColorSpace:
            return {YDZDX_8bit_full_rgb_to_yuv, YDZDX_8bit_full_yuv_to_rgb};
        case kYDZDX_Limited_SkYUVColorSpace:
            return {YDZDX_8bit_limited_rgb_to_yuv, YDZDX_8bit_limited_yuv_to_rgb};
        case kGBR_Full_SkYUVColorSpace:
            return {GBR_8bit_full_rgb_to_yuv, GBR_8bit_full_yuv_to_rgb};
        case kGBR_Limited_SkYUVColorSpace:
            return {GBR_8bit_limited_rgb_to_yuv, GBR_8bit_limited_yuv_to_rgb};
        case kYCgCo_8bit_Full_SkYUVColorSpace:
            return {YCgCo_8bit_full_rgb_to_yuv, YCgCo_8bit_full_yuv_to_rgb};
        case kYCgCo_8bit_Limited_SkYUVColorSpace:
            return {YCgCo_8bit_limited_rgb_to_yuv, YCgCo_8bit_limited_yuv_to_rgb};
        case kYCgCo_10bit_Full_SkYUVColorSpace:
            return {YCgCo_10bit_full_rgb_to_yuv, YCgCo_10bit_full_yuv_to_rgb};
        case kYCgCo_10bit_Limited_SkYUVColorSpace:
            return {YCgCo_10bit_limited_rgb_to_yuv, YCgCo_10bit_limited_yuv_to_rgb};
        case kYCgCo_12bit_Full_SkYUVColorSpace:
            return {YCgCo_12bit_full_rgb_to_yuv, YCgCo_12bit_full_yuv_to_rgb};
        case kYCgCo_12bit_Limited_SkYUVColorSpace:
            return {YCgCo_12bit_limited_rgb_to_yuv, YCgCo_12bit_limited_yuv_to_rgb};
        case kYCgCo_16bit_Full_SkYUVColorSpace:
            return {YCgCo_16bit_full_rgb_to_yuv, YCgCo_16bit_full_yuv_to_rgb};
        case kYCgCo_16bit_Limited_SkYUVColorSpace:
            return {YCgCo_16bit_limited_rgb_to_yuv, YCgCo_16bit_limited_yuv_to_rgb};
        default:
            return {};
    }
}

static void set_identity(float m[20]) {
    std::fill_n(m, 20, 0.0f);
    m[0] = m[6] = m[12] = m[18] = 1;
}

void SkColorMatrix_RGB2YUV(SkYUVColorSpace cs, float m[20]) {
    const ColorSpaceInfo* info = get_color_space_info(cs);
    if (!info) {
        set_identity(m);
        return;
    }
    if (const float* table = get_precomputed_matrices(cs).rgb_to_yuv) {
        std::copy_n(table, 20, m);
        return;
    }
    make_rgb_to_yuv_matrix(m, *info);
}

void SkColorMatrix_YUV2RGB(SkYUVColorSpace cs, float m[20]) {
    const ColorSpaceInfo* info = get_color_space_info(cs);
    if (!info) {
        set_identity(m);
        return;
    }
    if (const float* table = get_precomputed_matrices(cs).yuv_to_rgb) {
        std::copy_n(table, 20, m);
        return;
    }
    float rgb2yuv[20];
    make_rgb_to_yuv_matrix(rgb2yuv, *info);
    SkM44 m44, im44;
    colormatrix_to_matrix44(rgb2yuv, &m44);
    SkAssertResult(m44.invert(&im44));
    matrix44_to_colormatrix(im44, m);
}

bool SkYUVColorSpaceIsLimitedRange(SkYUVColorSpace cs) {
    const ColorSpaceInfo* info = get_color_space_info(cs);
    return info && info->range == Range::kLimited;
}

static void dump(
        const float m[20], const char* name, int bits, const char* range_name, bool rgb2yuv) {
    const char* dirname = rgb2yuv ? "rgb_to_yuv" : "yuv_to_rgb";
    SkDebugf("constexpr float %s_%dbit_%s_%s[] = {\n", name, bits, range_name, dirname);
    SkDebugf("#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)\n");
    for (int i = 0; i < 4; ++i) {
        SkDebugf(" ");
        for (int j = 0; j < 5; ++j) {
            const float v = m[i * 5 + j];
            SkDebugf(" %16af,", v);
        }
        SkDebugf("\n");
    }
    SkDebugf("#else\n");
    for (int i = 0; i < 4; ++i) {
        SkDebugf(" ");
        for (int j = 0; j < 5; ++j) {
            SkDebugf(" %16.6ff,", m[i * 5 + j]);
        }
        SkDebugf("\n");
    }
    SkDebugf("#endif\n");
    SkDebugf("};\n");
}

// Used to create the prebuilt tables for each colorspace.
// Don't remove this function, in case we want to recompute those tables in the future.
void SkColorMatrix_DumpYUVMatrixTables() {
    struct {
        const char* name;
        MatrixCoefficients matrix_coefficients;
        int bits;
    } combinations[] = {
        {"Rec601",   MatrixCoefficients::kRec601,    8},
        {"Rec709",   MatrixCoefficients::kRec709,    8},
        {"BT2020",   MatrixCoefficients::kBT2020,    8},
        {"BT2020",   MatrixCoefficients::kBT2020,   10},
        {"BT2020",   MatrixCoefficients::kBT2020,   12},
        {"BT2020",   MatrixCoefficients::kBT2020,   16},
        {"FCC",      MatrixCoefficients::kFCC,       8},
        {"SMPTE240", MatrixCoefficients::kSMPTE240,  8},
        {"YDZDX",    MatrixCoefficients::kYDZDX,     8},
        {"GBR",      MatrixCoefficients::kGBR,       8},
        {"YCgCo",    MatrixCoefficients::kYCgCo,     8},
        {"YCgCo",    MatrixCoefficients::kYCgCo,    10},
        {"YCgCo",    MatrixCoefficients::kYCgCo,    12},
        {"YCgCo",    MatrixCoefficients::kYCgCo,    16},
    };
    struct {
        const char* name;
        Range range;
    } ranges[] = {
        {"full",    Range::kFull},
        {"limited", Range::kLimited},
    };
    for (const auto& combo : combinations) {
        for (const auto& r : ranges) {
            float m[20];
            make_rgb_to_yuv_matrix(m, combo.matrix_coefficients, combo.bits, r.range);
            dump(m, combo.name, combo.bits, r.name, /*rgb2yuv=*/true);
            SkM44 m44, im44;
            colormatrix_to_matrix44(m, &m44);
            float im[20];
#ifdef SK_DEBUG
            // be sure our conversion between matrix44 and colormatrix is perfect
            matrix44_to_colormatrix(m44, im);
            SkASSERT(memcmp(m, im, sizeof(im)) == 0);
#endif
            SkAssertResult(m44.invert(&im44));
            matrix44_to_colormatrix(im44, im);
            dump(im, combo.name, combo.bits, r.name, /*rgb2yuv=*/false);
        }
    }
}
