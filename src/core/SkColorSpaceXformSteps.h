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

    // Source pipeline steps, pre-blend.
    bool early_unpremul;
    bool linearize_src;
    bool late_unpremul;
    bool gamut_transform;
    bool early_encode;
    bool premul;

    // Destination pipeline steps, pre-blend.
    bool linearize_dst;

    // Post-blend steps.
    bool late_encode;

/* TODO
    SkColorSpaceTransferFn srcTFInv,  // Apply for linearize_src.
                           dstTFInv,  // Apply for linearize_dst.
                           dstTF;     // Apply for early_encode or late_encode.
*/
    float src_to_dst_matrix[9];       // Apply this 3x3 row-major matrix for gamut_transform.
};



#endif//SkColorSpaceXformSteps_DEFINED
