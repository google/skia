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

struct SkTableTransferFn {
    const float* fData;
    int          fSize;
};

class SkColorSpaceXform_A2B : public SkColorSpaceXform_Base {
public:
    bool onApply(ColorFormat dstFormat, void* dst, ColorFormat srcFormat, const void* src,
                 int count, SkAlphaType alphaType) const override;

private:
    SkColorSpaceXform_A2B(SkColorSpace_A2B* srcSpace, SkColorSpace_XYZ* dstSpace);

    void addTransferFns(const SkColorSpaceTransferFn& fn, int channelCount);

    void addTransferFn(const SkColorSpaceTransferFn& fn, int channelIndex);

    void addTableFn(const SkTableTransferFn& table, int channelIndex);

    void addMatrix(const SkMatrix44& matrix);

    SkRasterPipeline                             fElementsPipeline;
    bool                                         fLinearDstGamma;

    // storage used by the pipeline
    std::forward_list<SkColorSpaceTransferFn>    fTransferFns;
    std::forward_list<SkTableTransferFn>         fTableTransferFns;
    std::forward_list<std::vector<float>>        fMatrices;
    std::vector<sk_sp<const SkColorLookUpTable>> fCLUTs;

    // these are here to maintain ownership of tables used in the pipeline
    std::forward_list<std::vector<float>>        fTableStorage;
    std::vector<sk_sp<const SkGammas>>           fGammaRefs;

    friend class SkColorSpaceXform_Base;
};

#endif
