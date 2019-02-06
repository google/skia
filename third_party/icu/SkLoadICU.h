/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef load_icu_DEFINED
#define load_icu_DEFINED

#include "SkTypes.h"

#if defined(SK_BUILD_FOR_WIN) && defined(SK_USING_THIRD_PARTY_ICU)

#include "../private/SkLeanWindows.h"

#include "unicode/uvernum.h"
#include "unicode/udata.h"

#define ICU_UTIL_DATA_SYMBOL "icudt" U_ICU_VERSION_SHORT "_dat"
#define ICU_UTIL_DATA_SHARED_MODULE_NAME "icudt.dll"

static inline bool SkLoadICU() {
    SkOnce once;
    once([] {
        HMODULE module = LoadLibraryA(ICU_UTIL_DATA_SHARED_MODULE_NAME);
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
    });
    return true;
}

#undef ICU_UTIL_DATA_SHARED_MODULE_NAME
#undef ICU_UTIL_DATA_SYMBOL

#else
inline bool SkLoadICU() { return true; }
#endif  // SK_BUILD_FOR_WIN
#endif  // load_icu_DEFINED

