/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"
#include "src/core/SkRasterPipelineUtils.h"

void SkRasterPipelineUtils::append(SkRasterPipeline::Stage stage, void* ctx) {
#if !defined(SKSL_STANDALONE)
    fPipeline->append(stage, ctx);
#else
    (void)fPipeline;
#endif
}
