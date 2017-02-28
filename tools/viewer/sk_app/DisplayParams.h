/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef DisplayParams_DEFINED
#define DisplayParams_DEFINED

#include "GrContextOptions.h"
#include "SkImageInfo.h"

namespace sk_app {

struct DisplayParams {
    DisplayParams()
        : fColorType(kN32_SkColorType)
        , fColorSpace(nullptr)
        , fMSAASampleCount(0) {}

    SkColorType         fColorType;
    sk_sp<SkColorSpace> fColorSpace;
    int                 fMSAASampleCount;
    GrContextOptions    fGrContextOptions;
};

}   // namespace sk_app

#endif
