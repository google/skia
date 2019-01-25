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
#include "../private/SkOnce.h"

#include "unicode/udata.h"

extern "C" const char U_ICUDATA_ENTRY_POINT[];

static inline void SkLoadICU() {
    UErrorCode err = U_ZERO_ERROR;
    udata_setCommonData(&U_ICUDATA_ENTRY_POINT, &err);
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

#else
inline void SkLoadICU() {}
#endif  // SK_BUILD_FOR_WIN
#endif  // load_icu_DEFINED

