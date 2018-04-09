/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpaceXform_Base_DEFINED
#define SkColorSpaceXform_Base_DEFINED

#include "SkColorSpace.h"
#include "SkColorSpaceXform.h"
#include "SkTemplates.h"

class SkColorSpaceXform_Base {
public:
    // A somewhat more powerful SkColorSpaceXform::New() that allows tweaking premulBehavior.
    static std::unique_ptr<SkColorSpaceXform> New(SkColorSpace* srcSpace,
                                                  SkColorSpace* dstSpace,
                                                  SkTransferFunctionBehavior premulBehavior);
};

#if defined(SK_USE_SKCMS)
std::unique_ptr<SkColorSpaceXform> MakeSkcmsXform(SkColorSpace* src, SkColorSpace* dst,
                                                  SkTransferFunctionBehavior premulBehavior);
#endif

#endif
