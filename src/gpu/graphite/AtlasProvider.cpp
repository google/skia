/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/AtlasProvider.h"

#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/PathAtlas.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/text/TextAtlasManager.h"

namespace skgpu::graphite {

AtlasProvider::AtlasProvider(Recorder* recorder)
        : fTextAtlasManager(std::make_unique<TextAtlasManager>(recorder)) {
#ifdef SK_ENABLE_VELLO_SHADERS
    if (recorder->priv().caps()->computeSupport()) {
        fComputePathAtlas = std::make_unique<ComputePathAtlas>();
    }
#endif  // SK_ENABLE_VELLO_SHADERS
}

}  // namespace skgpu::graphite
