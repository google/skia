/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/effects/colorfilters/SkColorFilterBase.h"

#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h" // IWYU pragma: keep
#include "include/core/SkColorType.h"
#include "include/core/SkSurfaceProps.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkAssert.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"

#include <cstddef>

enum class SkBlendMode;

bool SkColorFilterBase::onAsAColorMode(SkColor*, SkBlendMode*) const {
    return false;
}

bool SkColorFilterBase::onAsAColorMatrix(float matrix[20]) const {
    return false;
}

SkPMColor4f SkColorFilterBase::onFilterColor4f(const SkPMColor4f& color,
                                               SkColorSpace* dstCS) const {
    constexpr size_t kEnoughForCommonFilters = 2048;  // big enough for a tiny SkSL program
    SkSTArenaAlloc<kEnoughForCommonFilters> alloc;
    SkRasterPipeline    pipeline(&alloc);
    pipeline.appendConstantColor(&alloc, color.vec());
    SkSurfaceProps props{}; // default OK; colorFilters don't render text
    SkStageRec rec = {&pipeline, &alloc, kRGBA_F32_SkColorType, dstCS, color.unpremul(), props};

    if (as_CFB(this)->appendStages(rec, color.fA == 1)) {
        SkPMColor4f dst;
        SkRasterPipeline_MemoryCtx dstPtr = { &dst, 0 };
        pipeline.append(SkRasterPipelineOp::store_f32, &dstPtr);
        pipeline.run(0,0, 1,1);
        return dst;
    }

    SkDEBUGFAIL("onFilterColor4f unimplemented for this filter");
    return SkPMColor4f{0,0,0,0};
}
