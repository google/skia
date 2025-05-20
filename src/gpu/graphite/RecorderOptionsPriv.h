/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_RecorderOptionsPriv_DEFINED
#define skgpu_graphite_RecorderOptionsPriv_DEFINED

#include "src/gpu/graphite/BufferManager.h"

#include <optional>

namespace skgpu::graphite {

/**
 * Private options that are only meant for testing within Skia's tools.
 */

struct RecorderOptionsPriv {
    // Override the default buffer sizes of the DrawBufferManager using this option.
    std::optional<DrawBufferManager::DrawBufferManagerOptions> fDbmOptions = std::nullopt;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_RecorderOptionsPriv_DEFINED
