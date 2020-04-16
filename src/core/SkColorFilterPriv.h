/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorFilterPriv_DEFINED
#define SkColorFilterPriv_DEFINED

#include "include/core/SkColorFilter.h"

class SkColorFilterPriv {
public:
    static sk_sp<SkColorFilter> MakeGaussian();
};

#endif
