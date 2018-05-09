/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkICC_DEFINED
#define SkICC_DEFINED

#include "SkData.h"
#include "SkRefCnt.h"

struct SkColorSpaceTransferFn;

sk_sp<SkData> SkWriteICCProfile(const SkColorSpaceTransferFn&, const float toXYZD50[9]);

#endif
