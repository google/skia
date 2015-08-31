/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * GenerateChecksums() in ../../src/utils/SkWhitelistTypefaces.cpp generated SkWhitelistChecksums.cpp.
 * Run 'whitelist_typefaces --generate' to create anew.
 */

#include "SkTDArray.h"

struct Whitelist {
    const char* fFontName;
    uint32_t fChecksum;
    bool fSerializedNameOnly;
    bool fSerializedSub;
};

static Whitelist whitelist[] = {
    { "Aegean", 0x82ad09a5, false, false },
    { "Analecta", 0xc378d990, false, false },
    { "Arial", 0xbc28cb14, false, false },
    { "DejaVu Sans", 0x91e2fe32, false, false },
    { "DejaVu Sans Mono", 0xddc8af48, false, false },
    { "DejaVu Serif", 0x0077166d, false, false },
    { "FreeMono", 0xb9e602cd, false, false },
    { "FreeSans", 0x6f484994, false, false },
    { "FreeSerif", 0x933ce1c4, false, false },
    { "Khmer OS", 0x917c40aa, false, false },
    { "Kochi Gothic", 0x962132dd, false, false },
    { "Lohit Kannada", 0x7c829df4, false, false },
    { "Lohit Marathi", 0x0eb0a941, false, false },
    { "Lohit Oriya", 0xf3e9d313, false, false },
    { "Lohit Punjabi", 0xfd8b26e0, false, false },
    { "Lohit Tamil", 0x83a29565, false, false },
    { "Lohit Telugu", 0x11424bd0, false, false },
    { "Meera", 0xe3e16220, false, false },
    { "Mukti Narrow", 0x53f7d053, false, false },
    { "NanumBarunGothic", 0x859e77ea, false, false },
    { "NanumGothic", 0x9edbcdb8, false, false },
    { "OpenSymbol", 0xd3d743df, false, false },
    { "Symbola", 0xfa80f2ab, false, false },
    { "TakaoPGothic", 0x068c405a, false, false },
    { "Waree", 0x036e4fa4, false, false },
    { "WenQuanYi Micro Hei", 0xfea8587c, false, false },
    { "padmaa", 0x09eb1865, false, false },
};

static const int whitelistCount = (int) SK_ARRAY_COUNT(whitelist);
