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

SK_API sk_sp<SkData> SkWriteICCProfile(const SkColorSpaceTransferFn&, const float toXYZD50[9]);

namespace SkICC {
    static inline sk_sp<SkData> WriteToICC(const SkColorSpaceTransferFn& fn,
                                           const SkMatrix44& toXYZD50) {
        if (toXYZD50.get(3,0) == 0 && toXYZD50.get(3,1) == 0 && toXYZD50.get(3,2) == 0 &&
            toXYZD50.get(3,3) == 1 &&
            toXYZD50.get(0,3) == 0 && toXYZD50.get(1,3) == 0 && toXYZD50.get(2,3) == 0) {

            float m33[9];
            for (int r = 0; r < 3; r++)
            for (int c = 0; c < 3; c++) {
                m33[3*r+c] = toXYZD50.get(r,c);
            }
            return SkWriteICCProfile(fn, m33);

        }
        return nullptr;
    }
}

#endif//SkICC_DEFINED
