/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpaceXformSteps_DEFINED
#define SkColorSpaceXformSteps_DEFINED

#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "include/private/SkImageInfoPriv.h"
#include "src/core/SkVM_fwd.h"

class SkRasterPipeline;

struct SkColorSpaceXformSteps {

    struct Flags {
        bool unpremul         = false;
        bool linearize        = false;
        bool gamut_transform  = false;
        bool encode           = false;
        bool premul           = false;

        uint32_t mask() const {
            return (unpremul        ?  1 : 0)
                 | (linearize       ?  2 : 0)
                 | (gamut_transform ?  4 : 0)
                 | (encode          ?  8 : 0)
                 | (premul          ? 16 : 0);
        }
    };

    SkColorSpaceXformSteps(SkColorSpace* src, SkAlphaType srcAT,
                           SkColorSpace* dst, SkAlphaType dstAT);

    template <typename S, typename D>
    SkColorSpaceXformSteps(const S& src, const D& dst)
        : SkColorSpaceXformSteps(src.colorSpace(), src.alphaType(),
                                 dst.colorSpace(), dst.alphaType()) {}

    void apply(float rgba[4]) const;
    void apply(SkRasterPipeline*, bool src_is_normalized) const;
    skvm::Color program(skvm::Builder*, skvm::Uniforms*, skvm::Color) const;

    void apply(SkRasterPipeline* p, SkColorType srcCT) const {
        return this->apply(p, SkColorTypeIsNormalized(srcCT));
    }

    Flags flags;

    bool srcTF_is_sRGB,
         dstTF_is_sRGB;
    skcms_TransferFunction srcTF,     // Apply for linearize.
                           dstTFInv;  // Apply for encode.
    float src_to_dst_matrix[9];       // Apply this 3x3 column-major matrix for gamut_transform.
};

#endif//SkColorSpaceXformSteps_DEFINED
