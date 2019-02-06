// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "SkLoadICU.h"

#if defined(_WIN32) && defined(SK_USING_THIRD_PARTY_ICU)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <cstdio>
#include <mutex>

#include "unicode/udata.h"

#define ICU_UTIL_DATA_SYMBOL "icudt" U_ICU_VERSION_SHORT "_dat"
#define ICU_UTIL_DATA_SHARED_MODULE_NAME "icudt.dll"

bool SkLoadICU() {
    static bool good = false;
    static std::once_flag flag;
    std::call_once(flag, []() {
        HMODULE module = LoadLibraryA(ICU_UTIL_DATA_SHARED_MODULE_NAME);
        if (!module) {
            fprintf(stderr, "Failed to load " ICU_UTIL_DATA_SHARED_MODULE_NAME "\n");
            return;
        }
        FARPROC addr = GetProcAddress(module, ICU_UTIL_DATA_SYMBOL);
        if (!addr) {
            fprintf(stderr, "Symbol " ICU_UTIL_DATA_SYMBOL " missing in "
                     ICU_UTIL_DATA_SHARED_MODULE_NAME ".\n");
            return;
        }
        UErrorCode err = U_ZERO_ERROR;
        udata_setCommonData(reinterpret_cast<void*>(addr), &err);
        if (err != U_ZERO_ERROR) {
            fprintf(stderr, "udata_setCommonData() returned %d.\n", (int)err);
            return;
        }
        udata_setFileAccess(UDATA_ONLY_PACKAGES, &err);
        if (err != U_ZERO_ERROR) {
            fprintf(stderr, "udata_setFileAccess() returned %d.\n", (int)err);
            return;
        }
        good = true;
    });
    return good;
}

#undef ICU_UTIL_DATA_SHARED_MODULE_NAME
#undef ICU_UTIL_DATA_SYMBOL

#endif  // defined(_WIN32) && defined(SK_USING_THIRD_PARTY_ICU)
