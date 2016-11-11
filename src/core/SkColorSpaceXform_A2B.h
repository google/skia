/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpaceXform_A2B_DEFINED
#define SkColorSpaceXform_A2B_DEFINED

#include "SkColorSpace_Base.h"
#include "SkColorSpaceXform_Base.h"
#include "SkRasterPipeline.h"

#include <forward_list>
#include <functional>
#include <vector>

class SkColorSpace_A2B;
class SkColorSpace_XYZ;


class SkColorSpaceXform_A2B : public SkColorSpaceXform_Base {
public:
    bool onApply(ColorFormat dstFormat, void* dst, ColorFormat srcFormat, const void* src,
                 int count, SkAlphaType alphaType) const override;

private:
    SkColorSpaceXform_A2B(SkColorSpace_A2B* srcSpace, SkColorSpace_XYZ* dstSpace);

    enum Channels {
        kRGB_Channels = -1,
        kR_Channels   =  0,
        kG_Channels   =  1,
        kB_Channels   =  2
    };
    void addGamma(std::function<float(float)> fn, Channels channels);

    void addMatrix(const SkMatrix44& matrix);

    SkRasterPipeline                               fElementsPipeline;
    bool                                           fLinearDstGamma;
    // storage used by the pipeline
    std::forward_list<std::function<float(float)>> fGammaFunctions;
    std::forward_list<std::vector<float>>          fMatrices;
    std::forward_list<std::vector<float>>          fGammaTables;
    std::vector<sk_sp<const SkColorLookUpTable>>   fCLUTs;
    // these are here to maintain ownership of tables used in the pipeline
    std::vector<sk_sp<const SkGammas>>             fGammaRefs;

    friend class SkColorSpaceXform;
};

#endif
