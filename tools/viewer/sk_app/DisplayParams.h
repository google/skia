/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef DisplayParams_DEFINED
#define DisplayParams_DEFINED

#include "SkImageInfo.h"

namespace sk_app {

struct DisplayParams {
    DisplayParams()
        : fColorType(kN32_SkColorType)
        , fProfileType(kLinear_SkColorProfileType)
        , fMSAASampleCount(0)
        , fDeepColor(false) {}

    SkColorType        fColorType;
    SkColorProfileType fProfileType;
    int                fMSAASampleCount;
    bool               fDeepColor;
};

}   // namespace sk_app

#endif
