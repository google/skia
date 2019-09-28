/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkEncodedInfo.h"

std::unique_ptr<SkEncodedInfo::ICCProfile> SkEncodedInfo::ICCProfile::Make(sk_sp<SkData> data) {
    if (data) {
        skcms_ICCProfile profile;
        if (skcms_Parse(data->data(), data->size(), &profile)) {
            return std::unique_ptr<ICCProfile>(new ICCProfile(profile, std::move(data)));
        }
    }
    return nullptr;
}

std::unique_ptr<SkEncodedInfo::ICCProfile> SkEncodedInfo::ICCProfile::Make(
        const skcms_ICCProfile& profile) {
    return std::unique_ptr<ICCProfile>(new ICCProfile(profile));
}

SkEncodedInfo::ICCProfile::ICCProfile(const skcms_ICCProfile& profile, sk_sp<SkData> data)
    : fProfile(profile)
    , fData(std::move(data))
{}
