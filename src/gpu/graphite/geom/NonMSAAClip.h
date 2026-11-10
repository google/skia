/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_NonMSAAClip_DEFINED
#define skgpu_graphite_geom_NonMSAAClip_DEFINED

#include "include/core/SkM44.h"
#include "include/core/SkRect.h"

#include "include/private/SkOnlyStackAllocatable.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

/**
 * Represents a rect or rrect clip with any circular or rectangular corners under an affine
 * transformation.
 */
struct AnalyticClip {
    // These defaults will produce no clip
    SkRect fBounds = { 0, 0, 0, 0 }; // Bounds of clip
    SkV4   fRadii = {0.f, 0.f, 0.f, 0.f};
    SkV4   fXform = {1.f, 0.f, 0.f, 1.f}; // device-to-clip's local 2x2
    // Inversion matches the ClipStack's convention for depth-only draws: an inverse fill is used
    // for intersection clips and a regular fill is for difference clips.
    bool   fInverted = false;

    // When this is true, the shader will always evaluate to full coverage
    bool isEmpty() const { return fBounds.isEmpty() && !fInverted; }
};

/**
 * Represents a clip that uses a mask in an atlas
 */
struct AtlasClip {
    SkIRect             fMaskBounds = {0, 0, 0, 0};
    SkIPoint            fOutPos = {0, 0};
    sk_sp<TextureProxy> fAtlasTexture = nullptr;

    bool isEmpty() const { return !SkToBool(fAtlasTexture.get()); }
};

/**
 * Combined non-MSAA clip structure
 */
struct NonMSAAClip {
    SK_ONLY_STACK_ALLOCATABLE;

    AnalyticClip fAnalyticClip;
    AtlasClip    fAtlasClip;

    bool isEmpty() const { return fAnalyticClip.isEmpty() && fAtlasClip.isEmpty(); }
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_NonMSAAClip_DEFINED
