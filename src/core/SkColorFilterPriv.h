/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkColorFilterPriv_DEFINED
#define SkColorFilterPriv_DEFINED

#include "SkColorFilter.h"
#include "SkString.h"

using SkRuntimeColorFilterFn = void(*)(float[4], const void*);

class SkRuntimeColorFilterFactory {
public:
    SkRuntimeColorFilterFactory(SkString sksl, SkRuntimeColorFilterFn cpuFunc = nullptr);

    sk_sp<SkColorFilter> make(sk_sp<SkData> inputs);

private:
    int fIndex;
    SkString fSkSL;
    SkRuntimeColorFilterFn fCpuFunc;
};

#endif  // SkColorFilterPriv_DEFINED
