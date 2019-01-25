// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "SkLoadICU.h"

#ifdef _WIN32

#include "unicode/uvernum.h"
#include "unicode/udata.h"

#include <cstdio>
#include <cstdlib>
#include <mutex>

#define CONCAT(X, Y) CONCAT_IMPL_PRIV(X, Y)
#define CONCAT_IMPL_PRIV(X, Y) X ## Y
#define ICU_DATA_SYMBOL CONCAT(CONCAT(icudt, U_ICU_VERSION_MAJOR_NUM), _dat)
#define ERROR(FMT, ...) fprintf(stderr, __FILE__ ":%d: error: " FMT "\n", __LINE__, i##__VA__ARGS)

extern "C" void* ICU_DATA_SYMBOL;

void SkLoadICU() {
    static std::once_flag flag;
    std::call_once(flag, []() {
        UErrorCode err = U_ZERO_ERROR;
        udata_setCommonData(&ICU_DATA_SYMBOL, &err);
        if (err != U_ZERO_ERROR) {
            ERROR("udata_setCommonData() returned %d.", (int)err);
            abort();
        }
        udata_setFileAccess(UDATA_NO_FILES, &err);
        if (err != U_ZERO_ERROR) {
            ERROR("udata_setFileAccess() returned %d.", (int)err);
            abort();
        }
    });
}

#undef ERROR
#undef ICU_UTIL_DATA_SYMBOL
#undef CONCAT
#undef CONCAT_IMPL_PRIV

#endif  // _WIN32

