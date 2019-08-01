/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkComposeImageFilter_DEFINED
#define SkComposeImageFilter_DEFINED

#include "include/core/SkImageFilter.h"

// DEPRECATED: Use include/effects/SkImageFilters::Compose
class SK_API SkComposeImageFilter {
public:
    static sk_sp<SkImageFilter> Make(sk_sp<SkImageFilter> outer, sk_sp<SkImageFilter> inner);

    static void RegisterFlattenables();

private:
    SkComposeImageFilter() = delete;
};

#endif
