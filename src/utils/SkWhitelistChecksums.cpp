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
    { "Aegean", 0x639a35c7, false, false },
    { "Analecta", 0x639a35c7, false, false },
    { "Arial", 0xbc28cb14, false, false },
    { "DejaVu Sans", 0x639a35c7, false, false },
    { "DejaVu Sans Mono", 0xbc29a5d9, false, false },
    { "DejaVu Serif", 0x9db67efe, false, false },
    { "FreeMono", 0x724884f4, false, false },
    { "FreeSans", 0x7dfc48a3, false, false },
    { "FreeSerif", 0xa1ae8c77, false, false },
    { "Khmer OS", 0x917c40aa, false, false },
    { "Kochi Gothic", 0x962132dd, false, false },
    { "Lohit Kannada", 0x0b6ce863, false, false },
    { "Lohit Marathi", 0x0eb0a941, false, false },
    { "Lohit Oriya", 0xf3e9d313, false, false },
    { "Lohit Punjabi", 0xfd8b26e0, false, false },
    { "Lohit Tamil", 0xa8111d99, false, false },
    { "Lohit Telugu", 0xd34299e0, false, false },
    { "Meera", 0xe3e16220, false, false },
    { "Mukti Narrow", 0x53f7d053, false, false },
    { "NanumBarunGothic", 0x639a35c7, false, false },
    { "NanumGothic", 0xff8d773d, false, false },
    { "OpenSymbol", 0x4fcaf331, false, false },
    { "Symbola", 0x639a35c7, false, false },
    { "TakaoPGothic", 0x068c405a, false, false },
    { "Waree", 0x6a2bfca8, false, false },
    { "WenQuanYi Micro Hei", 0xcdec08a3, false, false },
    { "padmaa", 0x09eb1865, false, false },
};

static const int whitelistCount = (int) SK_ARRAY_COUNT(whitelist);
