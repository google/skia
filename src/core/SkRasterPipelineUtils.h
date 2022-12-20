/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkRasterPipeline.h"

class SkArenaAlloc;

class SkRasterPipelineUtils_Base {
public:
    virtual ~SkRasterPipelineUtils_Base() = default;

    // Forwards `append` calls to a Raster Pipeline.
    virtual void append(SkRasterPipeline::Stage stage, void* ctx) = 0;
};

class SkRasterPipelineUtils final : public SkRasterPipelineUtils_Base {
public:
    SkRasterPipelineUtils(SkRasterPipeline& p) : fPipeline(&p) {}

    void append(SkRasterPipeline::Stage stage, void* ctx) override;

private:
    SkRasterPipeline* fPipeline;
};
