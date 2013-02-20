/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageTypes_DEFINED
#define SkImageTypes_DEFINED

#include "SkTypes.h"

enum SkColorType {
    kAlpha_8_SkColorType,
    kRGB_565_SkColorType,
//    kRGBA_8888_SkColorType,
//    kBGRA_8888_SkColorType,
    kPMColor_SkColorType,

    kLastEnum_SkColorType = kPMColor_SkColorType
};

enum SkAlphaType {
//    kIgnore_SkAlphaType,
    kOpaque_SkAlphaType,
//    kUnpremul_SkAlphaType,
    kPremul_SkAlphaType,

    kLastEnum_SkAlphaType = kPremul_SkAlphaType
};

struct SkImageInfo {
    int         fWidth;
    int         fHeight;
    SkColorType fColorType;
    SkAlphaType fAlphaType;
};

#endif
