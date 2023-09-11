/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/effects/colorfilters/SkGaussianColorFilter.h"

#include "include/core/SkColorFilter.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "src/core/SkColorFilterPriv.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"

SkGaussianColorFilter::SkGaussianColorFilter() : SkColorFilterBase() {}

bool SkGaussianColorFilter::appendStages(const SkStageRec& rec, bool shaderIsOpaque) const {
    rec.fPipeline->append(SkRasterPipelineOp::gauss_a_to_rgba);
    return true;
}

sk_sp<SkFlattenable> SkGaussianColorFilter::CreateProc(SkReadBuffer&) {
    return SkColorFilterPriv::MakeGaussian();
}

sk_sp<SkColorFilter> SkColorFilterPriv::MakeGaussian() {
    return sk_sp<SkColorFilter>(new SkGaussianColorFilter);
}
