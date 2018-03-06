/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpaceXform_Base.h"
#include "skcms.h"

class SkColorSpaceXform_skcms : public SkColorSpaceXform {
public:
    SkColorSpaceXform_skcms(const skcms_ICCProfile& srcProfile,
                            const skcms_ICCProfile& dstProfile)
        : fSrcProfile(srcProfile)
        , fDstProfile(dstProfile)
    {}

    bool onApply();

private:
    skcms_ICCProfile fSrcProfile;
    skcms_ICCProfile fDstProfile;
};

std::unique_ptr<SkColorSpaceXform> MakeSkcmsXform(SkColorSpace* src, SkColorSpace* dst,
                                                  SkTransferFunctionBehavior premulBehavior) {
    return nullptr;
}
