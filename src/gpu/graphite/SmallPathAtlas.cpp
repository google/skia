/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/SmallPathAtlas.h"

#include "include/core/SkColorSpace.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkBlitter_A8.h"
#include "src/core/SkDrawBase.h"
#include "src/core/SkIPoint16.h"
#include "src/core/SkRasterClip.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

SmallPathAtlas::SmallPathAtlas(Recorder* recorder)
        : PathAtlas(recorder, kDefaultAtlasDim, kDefaultAtlasDim) {
}

const TextureProxy* SmallPathAtlas::onAddShape(const Shape& shape,
                                               const Transform& transform,
                                               const SkStrokeRec& strokeRec,
                                               skvx::half2 maskSize,
                                               skvx::half2* outPos) {
    return nullptr;
}

}  // namespace skgpu::graphite
