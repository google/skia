/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ContextUtils.h"

#include <string>
#include "experimental/graphite/src/PaintParams.h"
#include "include/private/SkUniquePaintParamsID.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkKeyContext.h"
#include "src/core/SkPipelineData.h"
#include "src/core/SkShaderCodeDictionary.h"

namespace skgpu {

std::tuple<SkUniquePaintParamsID, std::unique_ptr<SkPipelineData>> ExtractPaintData(
        Recorder* recorder,
        SkPaintParamsKeyBuilder* builder,
        const PaintParams& p) {

    SkDEBUGCODE(builder->checkReset());

    SkKeyContext keyContext(recorder);

    std::unique_ptr<SkPipelineData> pipelineData = std::make_unique<SkPipelineData>();

    p.toKey(keyContext, builder, pipelineData.get());

    SkPaintParamsKey key = builder->lockAsKey();

    auto dict = keyContext.dict();

    auto entry = dict->findOrCreate(key, pipelineData->blendInfo());

    return { entry->uniqueID(), std::move(pipelineData) };
}

} // namespace skgpu
