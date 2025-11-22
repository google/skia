/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ContextOptionsPriv_DEFINED
#define skgpu_graphite_ContextOptionsPriv_DEFINED

#include "include/private/base/SkMath.h"

#include <optional>

namespace skgpu::graphite {

enum class PathRendererStrategy;

/**
 * Private options that are only meant for testing within Skia's tools.
 */
struct ContextOptionsPriv {

    int  fMaxTextureSizeOverride = SK_MaxS32;

    /**
     * If true, will store a pointer in Recorder that points back to the Context
     * that created it. Used by readPixels() and other methods that normally require a Context.
     */
    bool fStoreContextRefInRecorder = false;

    /**
     * Override Caps' default strategy heuristics to prioritize this one if set *and* is supported.
     */
    std::optional<PathRendererStrategy> fPathRendererStrategy;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_ContextOptionsPriv_DEFINED
