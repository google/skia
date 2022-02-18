/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ContextUtils.h"

#include <string>
#include "experimental/graphite/src/ContextPriv.h"
#include "experimental/graphite/src/DrawTypes.h"
#include "experimental/graphite/src/PaintParams.h"
#include "include/core/SkPaint.h"
#include "include/private/SkUniquePaintParamsID.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkKeyHelpers.h"
#include "src/core/SkShaderCodeDictionary.h"
#include "src/core/SkUniform.h"
#include "src/core/SkUniformData.h"

namespace skgpu {

std::tuple<SkUniquePaintParamsID, std::unique_ptr<SkUniformBlock>> ExtractPaintData(
        SkShaderCodeDictionary* dict,
        SkPaintParamsKeyBuilder* builder,
        const PaintParams& p) {

    SkDEBUGCODE(builder->checkReset());

    std::unique_ptr<SkUniformBlock> block = std::make_unique<SkUniformBlock>();

    p.toKey(dict, SkBackend::kGraphite, builder, block.get());

    SkPaintParamsKey key = builder->lockAsKey();

    auto entry = dict->findOrCreate(key);

    return { entry->uniqueID(), std::move(block) };
}

} // namespace skgpu
