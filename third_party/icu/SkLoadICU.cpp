// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "SkLoadICU.h"

#ifdef _WIN32

#include "unicode/udata.h"

#include <cstdio>
#include <cstdlib>
#include <mutex>

extern "C" const char U_IMPORT U_ICUDATA_ENTRY_POINT[];

void SkLoadICU() {
    static std::once_flag flag;
    std::call_once(flag, []() {
        UErrorCode err = U_ZERO_ERROR;
        udata_setCommonData(&U_ICUDATA_ENTRY_POINT, &err);
        if (err != U_ZERO_ERROR) {
            fprintf(stderr , __FILE__ ":%d: error: udata_setCommonData() returned %d.\n",
                    __LINE__, (int)err);
            abort();
        }
        udata_setFileAccess(UDATA_NO_FILES, &err);
        if (err != U_ZERO_ERROR) {
            fprintf(stderr , __FILE__ ":%d: error: udata_setFileAccess() returned %d.\n",
                    __LINE__, (int)err);
            abort();
        }
    });
}

#endif  // _WIN32

