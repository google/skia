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

#include "../private/SkLeanWindows.h"

#include "unicode/udata.h"

extern "C" const char U_ICUDATA_ENTRY_POINT[];

inline void SkLoadICU() {
    UErrorCode udata_setCommonData_error = U_ZERO_ERROR;
    udata_setCommonData(&U_ICUDATA_ENTRY_POINT, &udata_setCommonData_error);
    SkASSERT_RELEASE(udata_setCommonData_error == U_ZERO_ERROR);

    UErrorCode udata_setFileAccess_error = U_ZERO_ERROR;
    udata_setFileAccess(UDATA_NO_FILES, &udata_setFileAccess_error);
    SkASSERT_RELEASE(udata_setFileAccess_error == U_ZERO_ERROR);
}

#undef ICU_UTIL_DATA_SHARED_MODULE_NAME
#undef ICU_UTIL_DATA_SYMBOL

#else
inline void SkLoadICU() {}
#endif  // SK_BUILD_FOR_WIN
#endif  // load_icu_DEFINED

