/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpace_A2B.h"

SkColorSpace_A2B::SkColorSpace_A2B(PCS pcs, sk_sp<SkData> profileData,
                                   std::vector<Element> elements)
    : INHERITED(std::move(profileData))
    , fPCS(pcs)
    , fElements(std::move(elements))
{}
