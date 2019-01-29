/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef load_icu_DEFINED
#define load_icu_DEFINED

#include "SkTypes.h"

#ifdef SK_BUILD_FOR_WIN

#include "unicode/uvernum.h"
#include "unicode/udata.h"
#include "../private/SkMacros.h"

#define ICU_DATA_SYMBOL SK_MACRO_CONCAT(SK_MACRO_CONCAT(icudt, U_ICU_VERSION_MAJOR_NUM), _dat)

extern "C" void* ICU_DATA_SYMBOL;

inline void SkLoadICU() {
    UErrorCode err = U_ZERO_ERROR;
    udata_setCommonData(&ICU_DATA_SYMBOL, &err);
    if (err != U_ZERO_ERROR) {
        SkDebugf("udata_setCommonData() returned %d.\n", (int)err);
        SK_ABORT("");
    }
    udata_setFileAccess(UDATA_NO_FILES, &err);
    if (err != U_ZERO_ERROR) {
        SkDebugf("udata_setFileAccess() returned %d.\n", (int)err);
        SK_ABORT("");
    }
}

#undef ICU_UTIL_DATA_SYMBOL

#else
inline void SkLoadICU() {}
#endif  // SK_BUILD_FOR_WIN
#endif  // load_icu_DEFINED

