// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "SkLoadICU.h"

#ifdef _WIN32

#include "unicode/uvernum.h"
#include "unicode/udata.h"

#include <cstdio>
#include <cstdlib>

#define CONCAT(X, Y) CONCAT_IMPL_PRIV(X, Y)
#define CONCAT_IMPL_PRIV(X, Y) X ## Y
#define ICU_DATA_SYMBOL CONCAT(CONCAT(icudt, U_ICU_VERSION_MAJOR_NUM), _dat)

extern "C" void* ICU_DATA_SYMBOL;

void SkLoadICU() {
    UErrorCode err = U_ZERO_ERROR;
    udata_setCommonData(&ICU_DATA_SYMBOL, &err);
    if (err != U_ZERO_ERROR) {
        fprintf(stderr, __FILE__ ": udata_setCommonData() returned %d.\n", (int)err);
        abort();
    }
    udata_setFileAccess(UDATA_NO_FILES, &err);
    if (err != U_ZERO_ERROR) {
        fprintf(stderr, __FILE__ ": udata_setFileAccess() returned %d.\n", (int)err);
        abort();
    }
}

#undef ICU_UTIL_DATA_SYMBOL
#undef CONCAT
#undef CONCAT_IMPL_PRIV

#else

void SkLoadICU() {}

#endif  // _WIN32

