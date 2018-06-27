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

#include "unicode/uvernum.h"
#include "unicode/udata.h"

#define ICU_UTIL_DATA_SYMBOL "icudt" U_ICU_VERSION_SHORT "_dat"
#define ICU_UTIL_DATA_SHARED_MODULE_NAME "icudt.dll"

inline void SkLoadICU() {
    HMODULE module = LoadLibrary(ICU_UTIL_DATA_SHARED_MODULE_NAME);
    if (!module) {
        SK_ABORT("Failed to load " ICU_UTIL_DATA_SHARED_MODULE_NAME "\n");
    }
    FARPROC addr = GetProcAddress(module, ICU_UTIL_DATA_SYMBOL);
    if (!addr) {
        SK_ABORT("Symbol " ICU_UTIL_DATA_SYMBOL " missing in "
                 ICU_UTIL_DATA_SHARED_MODULE_NAME ".\n");
    }
    UErrorCode err = U_ZERO_ERROR;
    udata_setCommonData(reinterpret_cast<void*>(addr), &err);
    if (err != U_ZERO_ERROR) {
        SkDebugf("udata_setCommonData() returned %d.\n", (int)err);
        SK_ABORT("");
    }
    udata_setFileAccess(UDATA_ONLY_PACKAGES, &err);
    if (err != U_ZERO_ERROR) {
        SkDebugf("udata_setFileAccess() returned %d.\n", (int)err);
        SK_ABORT("");
    }
}

#undef ICU_UTIL_DATA_SHARED_MODULE_NAME
#undef ICU_UTIL_DATA_SYMBOL

#else
inline void SkLoadICU() {}
#endif  // SK_BUILD_FOR_WIN
#endif  // load_icu_DEFINED

