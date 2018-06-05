/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpaceXformSteps_DEFINED
#define SkColorSpaceXformSteps_DEFINED

#include "SkColorSpace.h"
#include "SkImageInfo.h"

struct SkColorSpaceXformSteps {
    struct Flags {
        bool unpremul;
        bool linearize;
        bool gamut_transform;
        bool encode;
        bool premul;

        uint32_t mask() const {
            return unpremul        ?  1 : 0 |
                   linearize       ?  2 : 0 |
                   gamut_transform ?  4 : 0 |
                   encode          ?  8 : 0 |
                   premul          ? 16 : 0;
        }
    };

    SkColorSpaceXformSteps(const SkColorSpace* src, SkAlphaType srcAT,
                           const SkColorSpace* dst);

    Flags flags;

    SkColorSpaceTransferFn srcTF,     // Apply for linearize.
                           dstTFInv;  // Apply for encode.
    float src_to_dst_matrix[9];       // Apply this 3x3 row-major matrix for gamut_transform.
};

#endif//SkColorSpaceXformSteps_DEFINED
