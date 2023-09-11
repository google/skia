/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkGaussianColorFilter_DEFINED
#define SkGaussianColorFilter_DEFINED

#include "include/core/SkFlattenable.h"
#include "include/core/SkTypes.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"

class SkReadBuffer;
class SkWriteBuffer;
struct SkStageRec;

/**
 * Remaps the input color's alpha to a Gaussian ramp and then outputs premul white using the
 * remapped alpha.
 */
class SkGaussianColorFilter final : public SkColorFilterBase {
public:
    SkGaussianColorFilter();

    bool appendStages(const SkStageRec& rec, bool shaderIsOpaque) const override;

    SkColorFilterBase::Type type() const override { return SkColorFilterBase::Type::kGaussian; }

protected:
    void flatten(SkWriteBuffer&) const override {}

private:
    SK_FLATTENABLE_HOOKS(SkGaussianColorFilter)
};

#endif
