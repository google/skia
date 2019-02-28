/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkColorFilterPriv_DEFINED
#define SkColorFilterPriv_DEFINED

#include "SkColorFilter.h"

sk_sp<SkColorFilter> sk_make_runtime_color_filter(int index, SkString sksl, sk_sp<SkData> inputs);

using SkRuntimeColorFilterFn = void(*)(float[4], const void*);
sk_sp<SkColorFilter> sk_make_runtime_color_filter(int index, SkString sksl, sk_sp<SkData> inputs,
                                                  SkRuntimeColorFilterFn cpuFunc);


#endif  // SkColorFilterPriv_DEFINED
