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
    SkColorSpaceXformSteps(SkColorSpace* src, SkAlphaType srcAT,
                           SkColorSpace* dst);

    bool unpremul;
    bool linearize;
    bool gamut_transform;
    bool encode;
    bool premul;

    SkColorSpaceTransferFn srcTFInv,  // Apply for linearize.
                           dstTF;     // Apply for encode.
    float src_to_dst_matrix[9];       // Apply this 3x3 row-major matrix for gamut_transform.
};



#endif//SkColorSpaceXformSteps_DEFINED
