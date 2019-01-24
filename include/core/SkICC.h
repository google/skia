/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkICC_DEFINED
#define SkICC_DEFINED

#include "SkData.h"
#include "SkMatrix44.h"
#include "SkRefCnt.h"

struct skcms_Matrix3x3;
struct skcms_TransferFunction;
struct SkColorSpaceTransferFn;

SK_API sk_sp<SkData> SkWriteICCProfile(const skcms_TransferFunction&,
                                       const skcms_Matrix3x3& toXYZD50);

namespace SkICC {
    SK_API sk_sp<SkData> WriteToICC(const SkColorSpaceTransferFn&, const SkMatrix44&);
}

#endif//SkICC_DEFINED
