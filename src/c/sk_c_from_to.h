/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

static bool find_sk(CType from, SKType* to) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(CTypeSkTypeMap); ++i) {
        if (CTypeSkTypeMap[i].fC == from) {
            if (to) {
                *to = CTypeSkTypeMap[i].fSK;
            }
            return true;
        }
    }
    return false;
}

static bool find_c(SKType from, CType* to) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(CTypeSkTypeMap); ++i) {
        if (CTypeSkTypeMap[i].fSK == from) {
            if (to) {
                *to = CTypeSkTypeMap[i].fC;
            }
            return true;
        }
    }
    return false;
}

#undef CType
#undef SKType
#undef CTypeSkTypeMap
