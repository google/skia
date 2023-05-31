/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkRefCnt.h"
#include "include/effects/SkLumaColorFilter.h"
#include "include/effects/SkOverdrawColorFilter.h"

#if defined(SK_ENABLE_SKSL)
#error This should not be compiled if SKSL is enabled
#endif

sk_sp<SkColorFilter> SkColorFilters::Lerp(float weight, sk_sp<SkColorFilter> cf0,
                                                        sk_sp<SkColorFilter> cf1) {
    // TODO(skia:12197)
    return nullptr;
}

sk_sp<SkColorFilter> SkLumaColorFilter::Make() {
    // TODO(skia:12197)
    return nullptr;
}

sk_sp<SkColorFilter> SkOverdrawColorFilter::MakeWithSkColors(const SkColor colors[kNumColors]) {
    // TODO(skia:12197)
    return nullptr;
}
