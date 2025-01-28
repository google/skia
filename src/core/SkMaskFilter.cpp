/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkMaskFilter.h"

#include "include/core/SkFlattenable.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "src/core/SkMaskFilterBase.h"
struct SkDeserialProcs;

void SkMaskFilter::RegisterFlattenables() {
    sk_register_blur_maskfilter_createproc();
}

sk_sp<SkMaskFilter> SkMaskFilter::Deserialize(const void* data, size_t size,
                                              const SkDeserialProcs* procs) {
    return sk_sp<SkMaskFilter>(static_cast<SkMaskFilter*>(
                               SkFlattenable::Deserialize(
                               kSkMaskFilter_Type, data, size, procs).release()));
}
