/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpace_A2B.h"

SkColorSpace_A2B::SkColorSpace_A2B(ICCTypeFlag iccType, std::vector<Element> elements,
                                   PCS pcs, sk_sp<SkData> profileData)
    : INHERITED(std::move(profileData))
    , fICCType(iccType)
    , fElements(std::move(elements))
    , fPCS(pcs)
{
    SkASSERT(kRGB_ICCTypeFlag == iccType || kCMYK_ICCTypeFlag == iccType);
}
