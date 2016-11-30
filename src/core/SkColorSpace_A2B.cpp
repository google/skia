/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpace_A2B.h"

SkColorSpace_A2B::SkColorSpace_A2B(InputColorFormat inputColorFormat, std::vector<Element> elements, 
                                   PCS pcs, sk_sp<SkData> profileData)
    : INHERITED(std::move(profileData))
    , fInputColorFormat(inputColorFormat)
    , fElements(std::move(elements))
    , fPCS(pcs)
{}
